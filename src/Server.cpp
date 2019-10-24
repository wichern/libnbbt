/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Paul Wichern
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "nbbt/Server.h"
#include "Buffer.h"
#include "log.h"
#include "socket.h"

#include <map>
#include <sys/epoll.h>

//------------------------------------------------------------------------------

namespace nbbt {

//------------------------------------------------------------------------------

static size_t const c_epoll_queue_len = 1024;

//------------------------------------------------------------------------------

struct Client
{
    socket_t socket;
    client_t id;
    struct epoll_event event;
    Buffer rbuffer;
    Buffer wbuffer;
};

//------------------------------------------------------------------------------

struct Server::ServerImpl
{
    Client* accept();
    void disconnected(Client* client);

    int epoll_ = -1;
    struct epoll_event* events_ = nullptr;

    socket_t listener_ = INVALID_SOCKET;
    std::map<socket_t, Client*> clients_;
    std::map<client_t, Client*> idMapping_;
};

//------------------------------------------------------------------------------

Server::Server()
    : p(new ServerImpl)
{

}

//------------------------------------------------------------------------------

Server::~Server()
{
    if (INVALID_SOCKET != p->listener_) {
        socket_close(p->listener_);
    }

    if (-1 != p->epoll_) {
        socket_close(p->epoll_);
    }

    for (auto& client : p->clients_) {
        if (client.first != INVALID_SOCKET) {
            socket_close(client.first);
        }
        delete client.second;
    }

    if (p->events_) {
        delete [] p->events_;
    }

    delete p;
}

//------------------------------------------------------------------------------

bool Server::init(int port, int domain)
{
    // dDon't call again, when already listening.
    if (p->listener_ != INVALID_SOCKET) {
        return false;
    }

    if ((p->listener_ = ::socket(domain, SOCK_STREAM | SOCK_NONBLOCK, 0)) == INVALID_SOCKET) {
        goto init_socket_failed;
    }

    int one = 1;
    if (::setsockopt(p->listener_, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == -1) {
        goto init_socket_failed;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (::bind(p->listener_, (struct sockaddr*)&address, sizeof(address)) == -1) {
        goto init_socket_failed;
    }

    if (::listen(p->listener_, SOMAXCONN) == -1) {
        goto init_socket_failed;
    }

    p->epoll_ = ::epoll_create1(0);
    if (-1 == p->epoll_) {
        goto init_socket_failed;
    }

    struct epoll_event event;
    event.data.fd = p->listener_;
    event.events = EPOLLIN | EPOLLET;
    if (-1 == ::epoll_ctl(p->epoll_, EPOLL_CTL_ADD, p->listener_, &event)) {
        goto init_socket_failed;
    }

    p->events_ = new struct epoll_event[c_epoll_queue_len];

    return true;

init_socket_failed:
    log_last_socket_error();
    if (p->listener_ != INVALID_SOCKET) {
        socket_close(p->listener_);
    }
    return false;
}

//------------------------------------------------------------------------------

bool Server::run(int timeout)
{
    if (nullptr == p->events_) {
        return false;
    }

    int nfds = ::epoll_wait(p->epoll_, p->events_, c_epoll_queue_len, timeout);
    if (-1 == nfds) {
        log_last_socket_error();
        return false;
    }

    for (int i = 0; i < nfds; ++i) {
        struct epoll_event const& event = p->events_[i];

        if (event.data.fd == p->listener_) {
            // new client connects
            ClientData* client;
            while (client = p->accept()) {
                onConnected(&(client->client));
            }
            continue;
        }

        // client socket
        auto it = p->clients_.find(event.data.fd);
        if (it == p->clients_.end()) {
            LOG_ERR(u8"unknown socket");
            break;
        }
        ClientData* client = (*it).second;

        // socket has disconnected
        if (event.events & (EPOLLERR | EPOLLHUP)) {
            LOG_WARN_F(u8"Socket error (events:%u)", event.events);
            p->disconnected(client);
            onDisconnected(&(client->client));
            continue;
        }

        if (event.events & EPOLLRDHUP) {
            // client closed the connection
            p->disconnected(client);
            onDisconnected(&(client->client));
            continue;
        }

        // data available to read from client/slave
        if (event.events & EPOLLIN) {
            size_t read;
            if (client->client.rbuffer.read(read) < 1) {
                log_last_socket_error();
                continue;
            }

            onReadyRead(&(client->client));
        }

        // write buffer has more space
        if (event.events & EPOLLOUT) {
            // write more data
            switch (client->client.wbuffer.flush()) {
            case 0: // socket disconnected
            {
                p->disconnected(client);
                onDisconnected(&(client->client));
                continue;
            }
            case -1:
            {
                log_last_socket_error();
            } break;
            default:
            {
                // noop
            }
            } // switch

            // if no more data needs to be written, we can remove the EPOLLOUT flag
            if (client->client.wbuffer.available() == 0) {
                client->event.events &= ~EPOLLOUT;
                if (-1 == ::epoll_ctl(p->epoll_, EPOLL_CTL_MOD, client->socket, &client->event)) {
                    log_last_socket_error();
                }
            }
        }
    }

    return true;
}

bool Server::send(client_t client, const unsigned char* src, size_t bytes)
{

}

//------------------------------------------------------------------------------

void Server::ServerImpl::disconnected(ClientData* client)
{
    if (-1 == ::epoll_ctl(epoll_, EPOLL_CTL_DEL, client->socket, nullptr)) {
        log_last_socket_error();
    }
    socket_close(client->socket);
    clients_.erase(client->socket);
    delete client;
}

//------------------------------------------------------------------------------

ClientData* Server::ServerImpl::accept()
{
    socket_t socket = ::accept(listener_, nullptr, nullptr);
    if (INVALID_SOCKET == socket) {
        if (errno == EAGAIN) {
            // we have processed all incoming connections
        } else {
            log_last_socket_error();
        }
        return nullptr;
    } else {
        ClientData* client = new ClientData;
        client->socket = socket;
        client->client.wbuffer.set_socket(socket);
        client->client.rbuffer.set_socket(socket);
        client->event.data.fd = socket;
        client->event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;

        if (!socket_set_nonblocking(socket)) {
            socket_close(socket);
            delete client;
            return nullptr;
        }

        if (-1 == ::epoll_ctl(epoll_, EPOLL_CTL_ADD, socket, &client->event)) {
            log_last_socket_error();
            socket_close(socket);
            delete client;
            return nullptr;
        }

        clients_[socket] = client;

        return client;
    }
}

//------------------------------------------------------------------------------

} // namespace nbbt
