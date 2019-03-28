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

#include "nbbt/Client.h"
#include "nbbt/log.h"
#include "nbbt/socket.h"

#include <string.h>
#include <map>

//------------------------------------------------------------------------------

namespace nbbt {

//------------------------------------------------------------------------------

struct Client::ClientImpl
{
    socket_t socket = INVALID_SOCKET;
};

//------------------------------------------------------------------------------


Client::Client()
    : p(new ClientImpl)
{

}

//------------------------------------------------------------------------------

Client::~Client()
{
    delete p;
}

//------------------------------------------------------------------------------

bool Client::connect(char const* host, int port)
{
    if (p->socket != INVALID_SOCKET) {
        return true;
    }

    p->socket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == p->socket) {
        log_last_socket_error();
        return false;
    }

    struct sockaddr_in server;
    ::memset(&server, 0, sizeof(server));
    server.sin_addr.s_addr = ::inet_addr(host);

    if (INADDR_NONE == server.sin_addr.s_addr) {
        struct hostent* he;
        struct in_addr** addr_list;

        // resolve the hostname
        if ((he = ::gethostbyname(host)) == NULL) {
            LOG_ERR_F(u8"Failed to resolve \"%s\".", host);
            return false;
        }

        // We currently only support IPv4
        if (he->h_addrtype != AF_INET) {
            LOG_ERR("Only IPv4 supported!");
            return false;
        }

        addr_list = (struct in_addr**)he->h_addr_list;

        for (int i = 0; addr_list[i]; ++i) {
            server.sin_addr = *addr_list[i];
            break;
        }
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    int ret = ::connect(p->socket, (struct sockaddr*)&server, sizeof(server));
    if (ret != 0) {
#ifdef _WIN32
        if (WSAGetLastError() != WSAECONNREFUSED) {
#else
        if (errno != ECONNREFUSED) {
#endif
            log_last_socket_error();
        }

        socket_close(p->socket);
        p->socket = INVALID_SOCKET;
        return false;
    }

    wbuffer.set_socket(p->socket);
    rbuffer.set_socket(p->socket);

    return true;
}

//------------------------------------------------------------------------------

bool Client::run()
{
    if (INVALID_SOCKET == p->socket) {
        return false;
    }

    fd_set rset, wset;

    FD_ZERO(&rset);
    FD_SET(p->socket, &rset);
    fd_set* rset_ptr = &rset;
    fd_set* wset_ptr = nullptr;

    if (wbuffer.available() > 0) {
        FD_ZERO(&wset);
        FD_SET(p->socket, &wset);
        wset_ptr = &wset;
    }

#ifdef _WIN32
    if (::select(0, rset_ptr, wset_ptr, nullptr, nullptr) == -1) {
#else
    if (::select(p->socket + 1, rset_ptr, wset_ptr, nullptr, nullptr) == -1) {
#endif
        log_last_socket_error();
        return false;
    }

    // Can write more data.
    if (wset_ptr && FD_ISSET(p->socket, &wset)) {
        wbuffer.flush();
    }

    if (!FD_ISSET(p->socket, &rset)) {
        return true;
    }

    size_t read;
    switch (rbuffer.read(read)) {
    case 1:
    {
        onReadyRead();
        return true;
    } break;
    case 0:
    {
        socket_close(p->socket);
        p->socket = INVALID_SOCKET;
        onDisconnected();
        rbuffer.clear();
        wbuffer.clear();
        return false;
    } break;
    default: // -1
    {
        log_last_socket_error();
        return false;
    } break;
    }
}

//------------------------------------------------------------------------------

} // namespace nbbt
