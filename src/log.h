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

#ifndef LIBNBBT_LOG_H
#define LIBNBBT_LOG_H

//------------------------------------------------------------------------------

#include "nbbt/socket.h"

#include <string>

//------------------------------------------------------------------------------

namespace nbbt {

//------------------------------------------------------------------------------

#define LOG_INFO_F(format, ...) log_message_f(2, format, __VA_ARGS__)
#define LOG_INFO(message) log_message(2, message)

#define LOG_WARN_F(format, ...) log_message_f(1, format, __VA_ARGS__)
#define LOG_WARN(message) log_message(1, message)

#define LOG_ERR_F(format, ...) log_message_f(0, format, __VA_ARGS__)
#define LOG_ERR(message) log_message(0, message)

void log_message_f(int level, char const* format, ...);
void log_message(int level, char const* message);

//------------------------------------------------------------------------------

#ifdef _WIN32
#define LOG_INFO_LASTERROR(error) log_win32_error(2, error)
#define LOG_WARN_LASTERROR(error) log_win32_error(1, error)
#define LOG_ERR_LASTERROR(error) log_win32_error(0, error)

void log_win32_error(int level, unsigned long error);
#endif // _WIN32

//------------------------------------------------------------------------------

#define LOG_INFO_LASTERROR(error) log_errno(2, error)
#define LOG_WARN_LASTERROR(error) log_errno(1, error)
#define LOG_ERR_LASTERROR(error) log_errno(0, error)

void log_errno(int level, int error);

//------------------------------------------------------------------------------

} // namespace nbbt

//------------------------------------------------------------------------------

#endif // LIBNBBT_LOG_H
