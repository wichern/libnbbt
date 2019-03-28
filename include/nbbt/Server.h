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

#include "nbbt/Buffer.h"

//------------------------------------------------------------------------------

namespace nbbt {

//------------------------------------------------------------------------------

class Server
{
public:
    struct Client {
        Buffer rbuffer;
        Buffer wbuffer;
    };

public:
    Server();
    virtual ~Server();

    bool init(int port, int domain = AF_INET, size_t epoll_queue_len = 1024);
    bool run(int timeout = -1);

    virtual void onConnected(Client* client) = 0;
    virtual void onDisconnected(Client* client) = 0;
    virtual void onReadyRead(Client* client) = 0;

private:
    struct ServerImpl;
    ServerImpl* p;
}; // class Server

//------------------------------------------------------------------------------

} // namespace nbbt

//------------------------------------------------------------------------------

#endif // LIBNBBT_SERVER_H
