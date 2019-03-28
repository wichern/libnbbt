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

#ifndef LIBNBBT_WIN32_UTF8_H
#define LIBNBBT_WIN32_UTF8_H

//------------------------------------------------------------------------------

#ifdef _WIN32
#include <string>

//------------------------------------------------------------------------------

namespace nbbt {

//------------------------------------------------------------------------------

std::string ToUtf8(const std::wstring& str);
std::wstring ToUtf16(const std::string& str);

//------------------------------------------------------------------------------
/**
 * Get message of given error.
 *
 * @param error     errno
 * @return          error message (utf-8)
 */
std::string win32_last_error(DWORD error);

//------------------------------------------------------------------------------

} // namespace nbbt

//------------------------------------------------------------------------------

#endif // _WIN32

//------------------------------------------------------------------------------

#endif // LIBNBBT_WIN32_UTF8_H
