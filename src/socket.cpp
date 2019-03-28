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

#include "nbbt/socket.h"
#include "nbbt/log.h"

#ifndef _WIN32
#include <sys/ioctl.h>
#include <unistd.h>
#include <cerrno>
#endif

#include <string.h>
#include <fcntl.h>

//------------------------------------------------------------------------------

namespace nbbt {

//------------------------------------------------------------------------------

void log_last_socket_error() {
#ifdef _WIN32
    LOG_ERR_LASTERROR(WSAGetLastError());
#else
    LOG_ERR_LASTERROR(errno);
#endif
}

//------------------------------------------------------------------------------

bool socket_set_nonblocking(socket_t socket)
{
#ifdef _WIN32
    u_long non_blocking = 1;
    if (-1 == ::ioctlsocket(socket, FIONBIO, &non_blocking)) {
        return false;
    }
#else
    const int flags = ::fcntl(socket, F_GETFL, 0);
    if (flags & O_NONBLOCK) {
        return true; // already in blocking mode
    }
    if (-1 == ::fcntl(socket, F_SETFL, flags | O_NONBLOCK)) {
        return false;
    }
#endif
    return true;
}

//------------------------------------------------------------------------------

void socket_close(socket_t socket)
{
#ifdef _WIN32
    int ret = ::closesocket(socket);
#else
    int ret = ::close(socket);
#endif

    if (0 != ret) {
        log_last_socket_error();
    }
}

//------------------------------------------------------------------------------

} // namespace nbbt
