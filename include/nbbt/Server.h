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

#ifndef LIBNBBT_SERVER_H
#define LIBNBBT_SERVER_H

//------------------------------------------------------------------------------

#include <cstddef> /* size_t */
#include <string>

//------------------------------------------------------------------------------

namespace nbbt {

//------------------------------------------------------------------------------

typedef int client_t;

//------------------------------------------------------------------------------

class IServer
{
public:
    virtual bool send(client_t client, unsigned char const* src, size_t bytes) = 0;
    virtual bool memcpy(client_t client, unsigned char* dest, size_t bytes) const = 0;
    virtual void remove(client_t client, size_t bytes) = 0;
    virtual size_t available(client_t client) const = 0;
    virtual bool get_string(client_t client, std::string& string, bool take = false) = 0;

    virtual void onConnected(client_t client) = 0;
    virtual void onDisconnected(client_t client) = 0;
    virtual void onReadyRead(client_t client) = 0;
};

//------------------------------------------------------------------------------

class Server : public IServer
{
public:
    Server();
    virtual ~Server();

    bool init(int port, int domain = AF_INET);
    bool run(int timeout = -1);

    bool send(client_t client, unsigned char const* src, size_t bytes) override;
    bool memcpy(client_t client, unsigned char* dest, size_t bytes) const override;
    void remove(client_t client, size_t bytes) override;
    size_t available(client_t client) const override;
    bool get_string(client_t client, std::string& string, bool take = false) override;

private:
    struct ServerImpl;
    ServerImpl* p;
}; // class Server

//------------------------------------------------------------------------------

class ThreadedServer: public IServer
{
public:
    ThreadedServer();
    virtual ~ThreadedServer();

    bool init(int port, int domain = AF_INET);
    void start();

    bool send(client_t client, unsigned char const* src, size_t bytes) override;
    bool memcpy(client_t client, unsigned char* dest, size_t bytes) const override;
    void remove(client_t client, size_t bytes) override;
    size_t available(client_t client) const override;
    bool get_string(client_t client, std::string& string, bool take = false) override;

private:
    struct ThreadedServerImpl;
    ThreadedServerImpl* p;
}; // class ThreadedServer

//------------------------------------------------------------------------------

} // namespace nbbt

//------------------------------------------------------------------------------

#endif // LIBNBBT_SERVER_H
