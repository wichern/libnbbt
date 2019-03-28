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

#include "nbbt/Buffer.h"

#include <cassert>
#include <errno.h>
#include <algorithm>
#include <cstring>
#include <limits>

//------------------------------------------------------------------------------

namespace nbbt {

//------------------------------------------------------------------------------

Buffer::Buffer(socket_t socket, size_t chunksize)
    : m_socket(socket), m_chunksize(chunksize), m_writepos(0), m_readpos(0)
{

}

//------------------------------------------------------------------------------

Buffer::~Buffer()
{
    clear();
}

//------------------------------------------------------------------------------

void Buffer::_append(const unsigned char* src, size_t bytes)
{
    assert(src);
    assert(bytes > 0);

    // add required chunks
    while ((m_writepos + bytes) > (m_chunks.size() << m_chunksize)) {
        m_chunks.push_back(new unsigned char[1 << m_chunksize]);
    }

    // copy data chunk wise
    while (bytes > 0) {
        size_t chunk = m_writepos >> m_chunksize;
        size_t chunk_idx = m_writepos - (chunk << m_chunksize);
        size_t to_copy = std::min(bytes, (1 << m_chunksize) - chunk_idx);
        ::memcpy(reinterpret_cast<void*>(m_chunks[chunk] + chunk_idx), src, to_copy);
        m_writepos += to_copy;
        bytes -= to_copy;
        src += to_copy;
    }

    assert(bytes == 0);
}

//------------------------------------------------------------------------------

int Buffer::_send(const unsigned char* src, size_t bytes, size_t& sent)
{
    sent = 0;

#ifdef _WIN32
    int ret = ::send(m_socket, reinterpret_cast<char const*>(src), static_cast<int>(bytes), 0);
    if (-1 == ret) {
        if (WSAGetLastError() == WSAEWOULDBLOCK) {
#else
    ssize_t ret = ::send(m_socket, reinterpret_cast<void const*>(src), bytes, 0);
    if (-1 == ret) {
        if (errno == EAGAIN) {
#endif
            return 1;
        } else {
            return -1;
        }
    } else if (ret == 0) {
        return 0;
    } else if (ret > 0) {
        sent = static_cast<size_t>(ret);
        return 1;
    }
}

//------------------------------------------------------------------------------

void Buffer::memcpy(unsigned char* dst, size_t bytes)
{
    assert((m_readpos + bytes) <= m_writepos);

    // copy data chunk wise
    size_t pos = m_readpos;
    while (bytes > 0) {
        size_t chunk = pos >> m_chunksize;
        size_t chunk_idx = pos - (chunk << m_chunksize);
        size_t to_copy = std::min(bytes, (1 << m_chunksize) - chunk_idx);
        ::memcpy(dst, reinterpret_cast<void*>(m_chunks[chunk] + chunk_idx), to_copy);
        bytes -= to_copy;
        dst += to_copy;
        pos += to_copy;
    }

    assert(bytes == 0);
}

//------------------------------------------------------------------------------

void Buffer::remove(size_t bytes)
{
    m_readpos += bytes;

    // move every chunk that has been completely removed to the end of the chunks.
    while (m_readpos > (1 << m_chunksize)) {
        unsigned char* chunk = m_chunks.front();
        m_chunks.pop_front();

        // at max only store twice the amount of chunks as currently needed
        if (m_chunks.size() < (m_readpos >> m_chunksize) * 2) {
            m_chunks.push_back(chunk);
        } else {
            delete [] chunk;
        }

        m_readpos -= 1 << m_chunksize;
        m_writepos -= 1 << m_chunksize;
    }
}

//------------------------------------------------------------------------------

int Buffer::read(size_t& bytes_read)
{
    // read all available data until EAGAIN
    bytes_read = 0;
    unsigned char buffer[4096];
    for (;;) {
#ifdef _WIN32
        int read = ::recv(m_socket, reinterpret_cast<char*>(buffer), 4096, 0);
        if (-1 == read) {
            if (WSAGetLastError() == WSAEWOULDBLOCK) {
#else
        ssize_t read = ::recv(m_socket, buffer, 4096, 0);
        if (-1 == read) {
            if (errno == EAGAIN) {
#endif
                return 1;
            } else {
                return -1;
            }
        } else if (0 == read) {
            return 0;
        } else {
            _append(buffer, read);
            bytes_read = available();
        }
    }
}

//------------------------------------------------------------------------------

void Buffer::clear()
{
    for (unsigned char* chunk : m_chunks) {
        delete [] chunk;
    }

    m_chunks.clear();
    m_writepos = 0;
    m_readpos = 0;
}

//------------------------------------------------------------------------------

bool Buffer::get_string(std::string& string, bool take)
{
    string.clear();

    size_t end = std::numeric_limits<size_t>::max();
    for (size_t i = m_readpos; i < m_writepos; ++i) {
        size_t chunk = i >> m_chunksize;
        size_t idx = i - (chunk << m_chunksize);
        if (m_chunks[chunk][idx] == '\0') {
            end = i;
            break;
        }
    }

    if (end == std::numeric_limits<size_t>::max()) {
        return false;
    }

    // special case for empty string
    if (0 == end) {
        if (take) {
            remove(1);
        }
        return true;
    }

    string.resize(end);
    memcpy(reinterpret_cast<unsigned char*>(&string[0]), end);

    if (take) {
        remove(end + 1);
    }

    return true;
}

//------------------------------------------------------------------------------

int Buffer::send(unsigned char const* src, size_t bytes)
{
    // If the buffer already contains data, we try to flush that first.
    if (available() > 0) {
        int ret = flush();
        if (ret != 1) {
            return ret;
        }
    }

    // If no more data is in the buffer we try to sent the data directly.
    if (available() == 0) {
        size_t sent;
        int ret = _send(src, bytes, sent);
        if (1 == ret) {
            // Could all data be sent? append the rest to this buffer.
            if (sent < bytes) {
                _append(src + sent, bytes - sent);
            }
        }

        return ret;
    }

    _append(src, bytes);

    return 1;
}

//------------------------------------------------------------------------------

int Buffer::flush()
{
    if (available() == 0) {
        return 1;
    }

    size_t pos = m_readpos;
    while (pos < m_writepos) {
        size_t chunk = pos >> m_chunksize;
        size_t chunk_idx = pos - (chunk << m_chunksize);
        size_t to_send = std::min(m_writepos - pos, (1 << m_chunksize) - chunk_idx);
#ifdef _WIN32
        int sent = ::send(m_socket, (const char*)(m_chunks[chunk] + chunk_idx), static_cast<int>(to_send), 0);
        if (-1 == sent) {
            if (WSAGetLastError() == WSAEWOULDBLOCK) {
#else
        ssize_t sent = ::send(m_socket, reinterpret_cast<void*>(m_chunks[chunk] + chunk_idx), to_send, 0);
        if (-1 == sent) {
            if (errno == EAGAIN) {
#endif
                return 1;
            } else {
                return -1;
            }
        } else if (0 == sent) {
            return 0;
        }

        pos += static_cast<size_t>(sent);
    }

    return 1;
}

//------------------------------------------------------------------------------

} // namespace nbbt
