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

#include "nbbt/win32_utf8.h"

//------------------------------------------------------------------------------

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

//------------------------------------------------------------------------------

namespace nbbt {

//------------------------------------------------------------------------------

std::string ToUtf8(const std::wstring& str)
{
    std::string ret;
    int len = ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), NULL, 0, NULL, NULL);
    if (len > 0) {
        ret.resize(len);
        ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), &ret[0], len, NULL, NULL);
    }
    return ret;
}

//------------------------------------------------------------------------------

std::wstring ToUtf16(const std::string& str)
{
    std::wstring ret;
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), NULL, 0);
    if (len > 0) {
        ret.resize(len);
        ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), &ret[0], len);
    }
    return ret;
}

//------------------------------------------------------------------------------

std::string win32_last_error(DWORD error) {
    LPVOID buffer;

    ::FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&buffer,
        0, NULL);

    std::wstring ret(static_cast<wchar_t*>(buffer));

    ::LocalFree(buffer);

    return ToUtf8(ret);
}

//------------------------------------------------------------------------------

} // namespace nbbt

//------------------------------------------------------------------------------

#endif // _WIN32
