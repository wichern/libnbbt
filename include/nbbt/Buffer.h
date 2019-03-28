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

#ifndef LIBNBBT_SOCKETBUFFER_H
#define LIBNBBT_SOCKETBUFFER_H

//------------------------------------------------------------------------------

#include "nbbt/socket.h"

#ifdef _WIN32
#include "shared/win32_utf8.h"
#endif

#include <deque>
#include <string>

//------------------------------------------------------------------------------

namespace nbbt {

//------------------------------------------------------------------------------

/**
 * SocketBuffer is a contiguous unlimited buffer, that supports adding at the
 * tail and taking from the head.
 *
 * Its main purpose is to receive all incoming data of a socket and allow
 * picking completely received messages from the beginning.
 *
 * It uses an array of buffers internally to be able to allow infinite append
 * to tail and remove from head.
 *
 * Usage as read buffer
 * --------------------
 *
 * SocketBuffer readbuffer(socket);
 *
 * switch (readbuffer.read()) {
 * case 1:
 * {
 *     if (available >= 512) {
 *         // take received data and work with it
 *         unsigned char buffer[512];
 *         readbuffer.memcpy(buffer, 512);
 *         readbuffer.remove(512); // removes from beginning
 *     }
 * } break;
 * case 0:
 * {
 *     // socket disconnected
 * } break;
 * default: // -1
 * {
 *     log_last_socket_error();
 * } break;
 * }
 *
 * Usage as write buffer
 * ---------------------
 *
 * SocketBuffer writebuffer(socket);
 *
 * unsigned char buffer[512] = { ... };
 * switch (wbuffer.send(buffer, 512)) {
 * case 1:
 * {
 *     // success
 * } break;
 * case 0:
 * {
 *     // socket disconnected
 * } break;
 * default: // -1
 * {
 *     log_last_socket_error();
 * } break;
 * }
 *
 * Data that could not be sent because ::send() returned EAGAIN will be appended
 * to the end of this buffer. Subsequent calls to flush() will send more.
 *
 * switch (wbuffer.flush()) {
 * case 1:
 * {
 *     // success
 * } break;
 * case 0:
 * {
 *     // socket disconnected
 * } break;
 * default: // -1
 * {
 *     log_last_socket_error();
 * } break;
 * }
 */
class Buffer
{
public:
    /**
     * Constructor
     *
     * @param socket        a non blocking socket
     * @param chunksize     size = 2^chunksize
     */
    explicit Buffer(socket_t socket = INVALID_SOCKET, size_t chunksize = 12);
    ~Buffer();

    /**
     * @brief read all data from give socket.
     *
     * Read all the data available on given socket.
     *
     * @param bytes_read    number of bytes now available in this buffer
     * @return              1 on success
     *                      0 on closed socket
     *                      -1 on socket error
     */
    int read(size_t& bytes_read);

    /**
     * Copy data into dest buffer.
     *
     * @param dst           destination buffer
     * @param bytes         number of bytes to copy
     */
    void memcpy(unsigned char* dest, size_t bytes);

    /**
     * Remove given number of bytes from the beginning of this buffer.
     *
     * @param bytes         bytes to remove
     */
    void remove(size_t bytes);

    /**
     * This is a helper function, that returns the first string at the beginning
     * of the buffer.
     *
     * @param take          whether to remove the returned string from this buffer
     * @param string        the string retrieved from this buffer.
     * @return              false if no complete string was in the buffer.
     */
    bool get_string(std::string& string, bool take = false);

    /**
     * @brief send given buffer.
     *
     * If ::send() returns with EAGAIN, the rest of the given buffer will
     * be added to this buffer and sent when you call flush().
     *
     * @param src           buffer to send
     * @param bytes         size of buffer
     * @return              1 on success
     *                      0 on closed socket
     *                      -1 on socket error
     */
    int send(unsigned char const* src, size_t bytes);

    /**
     * Flush buffer by calling ::send().
     *
     * @return              1 on success
     *                      0 on closed socket
     *                      -1 on socket error
     */
    int flush();

    /**
     * Set the current socket.
     *
     * @param socket        non blocking socket
     */
    void set_socket(socket_t socket) { m_socket = socket; }

    inline size_t available() const { return m_writepos - m_readpos; }

    void clear();

private:
    void _append(unsigned char const* src, size_t bytes);
    int _send(unsigned char const* src, size_t bytes, size_t& sent);

    socket_t m_socket;
    size_t m_chunksize;
    size_t m_readpos;
    size_t m_writepos;
    std::deque<unsigned char*> m_chunks;
}; // class Buffer

//------------------------------------------------------------------------------

} // namespace nbbt

//------------------------------------------------------------------------------

#endif // LIBNBBT_SOCKETBUFFER_H
