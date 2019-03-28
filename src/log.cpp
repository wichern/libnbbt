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

#include "nbbt/log.h"

#ifndef _WIN32
#include <syslog.h>
#include <unistd.h>
#include <cstring>
#else
#include "shared/win32_utf8.h"
#endif

#include <cstdarg>
#include <iostream>

#ifndef va_copy
#define va_copy(d, s) ((d) = (s))
#endif

//------------------------------------------------------------------------------

namespace nbbt {

//------------------------------------------------------------------------------

static const char* S_loglevel_names[] = {
    "ERR", "WARN", "INFO", "DBG"
};

#ifndef _WIN32
static int S_loglevel_syslog_codes[] = {
    LOG_ERR, LOG_WARNING, LOG_INFO, LOG_DEBUG
};
#endif

//------------------------------------------------------------------------------

void _log_message(int level, char const* format, va_list& ap) {
    static char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, ap);

#ifndef _WIN32
    syslog(S_loglevel_syslog_codes[level], "%s", buffer);
#endif
    std::cout << S_loglevel_names[level] << ": " << buffer << std::endl;
}

//------------------------------------------------------------------------------

void log_message_f(int level, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    _log_message(level, format, ap);
    va_end(ap);
}

//------------------------------------------------------------------------------

void log_message(int level, const char* message) {
#ifndef _WIN32
    syslog(S_loglevel_syslog_codes[level], "%s", message);
#endif
    std::cout << S_loglevel_names[level] << ": " << message << std::endl;
}

//------------------------------------------------------------------------------

#ifdef _WIN32
void log_win32_error(int level, unsigned long error) {
    std::string str = win32_last_error((DWORD)error);
    log_message(level, str.c_str());
}
#endif

//------------------------------------------------------------------------------

#ifndef _WIN32
void log_errno(int level, int error) {
    log_message(level, strerror(error));
}
#endif

//------------------------------------------------------------------------------

}  // namespace nbbt
