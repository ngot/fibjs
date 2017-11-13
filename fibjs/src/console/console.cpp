/*
 * console.cpp
 *
 *  Created on: Apr 7, 2012
 *      Author: lion
 */

#include "object.h"
#include "ifs/console.h"
#include "ifs/assert.h"
#include "ifs/encoding.h"
#include "ifs/process.h"
#include "ifs/util.h"
#include "v8_api.h"
#include <map>
#include "console.h"
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>

static LARGE_INTEGER systemFrequency;

inline int64_t Ticks()
{
    LARGE_INTEGER t;

    if (systemFrequency.QuadPart == 0)
        QueryPerformanceFrequency(&systemFrequency);

    QueryPerformanceCounter(&t);

    return t.QuadPart * 1000000 / systemFrequency.QuadPart;
}

#else
#include <dlfcn.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include "utils.h" // for ARRAYSIZE()

inline int64_t Ticks()
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) < 0)
        return 0;
    return (tv.tv_sec * 1000000ll) + tv.tv_usec;
}

#endif

namespace fibjs {

exlib::string COLOR_RESET = "";
exlib::string COLOR_BLACK = "";
exlib::string COLOR_RED = "";
exlib::string COLOR_GREEN = "";
exlib::string COLOR_YELLOW = "";
exlib::string COLOR_BLUE = "";
exlib::string COLOR_MAGENTA = "";
exlib::string COLOR_CYAN = "";
exlib::string COLOR_WHITE = "";
exlib::string COLOR_NORMAL = "";
exlib::string COLOR_GREY = "";

exlib::string COLOR_LIGHTRED = "";
exlib::string COLOR_LIGHTGREEN = "";
exlib::string COLOR_LIGHTYELLOW = "";
exlib::string COLOR_LIGHTBLUE = "";
exlib::string COLOR_LIGHTMAGENTA = "";
exlib::string COLOR_LIGHTCYAN = "";
exlib::string COLOR_LIGHTWHITE = "";

exlib::string COLOR_TITLE = "";

void init_color()
{
#ifndef _WIN32
    int32_t hr = ::isatty(1);
#else
    int32_t hr = _isatty(1);
#endif
    if (hr) {
        COLOR_RESET = "\x1b[0m";
        COLOR_BLACK = "\x1b[0;30m"; /* Black */
        COLOR_RED = "\x1b[0;31m"; /* Red */
        COLOR_GREEN = "\x1b[0;32m"; /* Green */
        COLOR_YELLOW = "\x1b[0;33m"; /* Yellow */
        COLOR_BLUE = "\x1b[0;34m"; /* Blue */
        COLOR_MAGENTA = "\x1b[0;35m"; /* Magenta */
        COLOR_CYAN = "\x1b[0;36m"; /* Cyan */
        COLOR_WHITE = "\x1b[0;37m"; /* White */
        COLOR_NORMAL = "\x1b[0;39m"; /* Normal */
        COLOR_GREY = "\x1B[90m"; /* Grey */

        COLOR_LIGHTRED = "\x1b[1;31m"; /* Red */
        COLOR_LIGHTGREEN = "\x1b[1;32m"; /* Green */
        COLOR_LIGHTYELLOW = "\x1b[1;33m"; /* Yellow */
        COLOR_LIGHTBLUE = "\x1b[1;34m"; /* Blue */
        COLOR_LIGHTMAGENTA = "\x1b[1;35m"; /* Magenta */
        COLOR_LIGHTCYAN = "\x1b[1;36m"; /* Cyan */
        COLOR_LIGHTWHITE = "\x1b[1;37m"; /* White */

        COLOR_TITLE = "\x1B[1;39m";
    }
}

extern std_logger* s_std;
void asyncLog(int32_t priority, exlib::string msg);

void _log(int32_t type, exlib::string fmt, OptArgs args)
{
    int32_t level;

    console_base::get_loglevel(level);

    if (type <= level) {
        exlib::string str;

        util_base::format(fmt, args, str);
        asyncLog(type, str);
    }
}

void _log(int32_t type, OptArgs args)
{
    int32_t level;

    console_base::get_loglevel(level);

    if (type <= level) {
        exlib::string str;

        util_base::format(args, str);
        asyncLog(type, str);
    }
}

result_t console_base::log(exlib::string fmt, OptArgs args)
{
    _log(_INFO, fmt, args);
    return 0;
}

result_t console_base::log(OptArgs args)
{
    _log(_INFO, args);
    return 0;
}

result_t console_base::debug(exlib::string fmt, OptArgs args)
{
    _log(_DEBUG, fmt, args);
    return 0;
}

result_t console_base::debug(OptArgs args)
{
    _log(_DEBUG, args);
    return 0;
}

result_t console_base::info(exlib::string fmt, OptArgs args)
{
    _log(_INFO, fmt, args);
    return 0;
}

result_t console_base::info(OptArgs args)
{
    _log(_INFO, args);
    return 0;
}

result_t console_base::notice(exlib::string fmt, OptArgs args)
{
    _log(_NOTICE, fmt, args);
    return 0;
}

result_t console_base::notice(OptArgs args)
{
    _log(_NOTICE, args);
    return 0;
}

result_t console_base::warn(exlib::string fmt, OptArgs args)
{
    _log(_WARN, fmt, args);
    return 0;
}

result_t console_base::warn(OptArgs args)
{
    _log(_WARN, args);
    return 0;
}

result_t console_base::error(exlib::string fmt, OptArgs args)
{
    _log(_ERROR, fmt, args);
    return 0;
}

result_t console_base::error(OptArgs args)
{
    _log(_ERROR, args);
    return 0;
}

result_t console_base::crit(exlib::string fmt, OptArgs args)
{
    _log(_CRIT, fmt, args);
    return 0;
}

result_t console_base::crit(OptArgs args)
{
    _log(_CRIT, args);
    return 0;
}

result_t console_base::alert(exlib::string fmt, OptArgs args)
{
    _log(_ALERT, fmt, args);
    return 0;
}

result_t console_base::alert(OptArgs args)
{
    _log(_ALERT, args);
    return 0;
}

exlib::string json_format(v8::Local<v8::Value> obj);

result_t console_base::dir(v8::Local<v8::Value> obj)
{
    exlib::string strBuffer = json_format(obj);
    asyncLog(_INFO, strBuffer);
    return 0;
}

static std::map<exlib::string, int64_t> s_timers;

result_t console_base::time(exlib::string label)
{
    s_timers[exlib::string(label)] = Ticks();

    return 0;
}

result_t console_base::timeEnd(exlib::string label)
{
    int64_t t = Ticks() - s_timers[label];

    s_timers.erase(label);

    exlib::string strBuffer;
    char numStr[64];

    sprintf(numStr, "%.10g", t / 1000.0);

    strBuffer.append(label);
    strBuffer.append(": ", 2);
    strBuffer.append(numStr);
    strBuffer.append("ms", 2);

    asyncLog(_INFO, strBuffer);
    return 0;
}

result_t console_base::trace(exlib::string label)
{
    exlib::string strBuffer;

    strBuffer.append("console.trace: ", 15);
    strBuffer.append(label);
    strBuffer.append(1, '\n');
    strBuffer.append(traceInfo(Isolate::current()->m_isolate, 10));

    asyncLog(_WARN, strBuffer);
    return 0;
}

result_t console_base::_assert(v8::Local<v8::Value> value, exlib::string msg)
{
    return assert_base::ok(value, msg);
}

result_t console_base::print(exlib::string fmt, OptArgs args)
{
    _log(_PRINT, fmt, args);
    return 0;
}

result_t console_base::print(OptArgs args)
{
    _log(_PRINT, args);
    return 0;
}

#ifdef _WIN32

result_t console_base::get_width(int32_t& retVal)
{
    CONSOLE_SCREEN_BUFFER_INFO info;

    if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info))
        return CHECK_ERROR(LastError());

    retVal = info.dwSize.X;

    return 0;
}

result_t console_base::get_height(int32_t& retVal)
{
    CONSOLE_SCREEN_BUFFER_INFO info;

    if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info))
        return CHECK_ERROR(LastError());

    retVal = info.srWindow.Bottom - info.srWindow.Top + 1;

    return 0;
}

result_t console_base::moveTo(int32_t row, int32_t column)
{
    if (row < 1 || column < 1)
        return CHECK_ERROR(CALL_E_INVALIDARG);

    CONSOLE_SCREEN_BUFFER_INFO info;
    HANDLE hd = GetStdHandle(STD_OUTPUT_HANDLE);

    flushLog();
    if (!GetConsoleScreenBufferInfo(hd, &info))
        return CHECK_ERROR(LastError());

    COORD pos = {
        (short)column - 1,
        (short)row - 1 + info.srWindow.Top
    };

    SetConsoleCursorPosition(hd, pos);

    return 0;
}

result_t console_base::hideCursor()
{
    HANDLE hd = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO aCursorInfo;

    GetConsoleCursorInfo(hd, &aCursorInfo);
    aCursorInfo.bVisible = false;
    SetConsoleCursorInfo(hd, &aCursorInfo);

    return 0;
}

result_t console_base::showCursor()
{
    HANDLE hd = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO aCursorInfo;

    GetConsoleCursorInfo(hd, &aCursorInfo);
    aCursorInfo.bVisible = true;
    SetConsoleCursorInfo(hd, &aCursorInfo);

    return 0;
}

result_t console_base::clear()
{
    HANDLE hd = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coordScreen = { 0, 0 };
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;

    if (!GetConsoleScreenBufferInfo(hd, &csbi))
        return CHECK_ERROR(LastError());

    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    FillConsoleOutputCharacter(hd, ' ', dwConSize,
        coordScreen, &cCharsWritten);

    if (!GetConsoleScreenBufferInfo(hd, &csbi))
        return CHECK_ERROR(LastError());

    FillConsoleOutputAttribute(hd, csbi.wAttributes, dwConSize,
        coordScreen, &cCharsWritten);

    SetConsoleCursorPosition(hd, coordScreen);
    return 0;
}

#else

result_t console_base::get_width(int32_t& retVal)
{
    struct winsize ws;
    int err;

    do
        err = ioctl(2, TIOCGWINSZ, &ws);
    while (err == -1 && errno == EINTR);

    if (err == -1)
        return CHECK_ERROR(LastError());

    retVal = ws.ws_col;

    return 0;
}

result_t console_base::get_height(int32_t& retVal)
{
    struct winsize ws;
    int err;

    do
        err = ioctl(2, TIOCGWINSZ, &ws);
    while (err == -1 && errno == EINTR);

    if (err == -1)
        return CHECK_ERROR(LastError());

    retVal = ws.ws_row;

    return 0;
}

result_t console_base::moveTo(int32_t row, int32_t column)
{
    if (row < 1 || column < 1)
        return CHECK_ERROR(CALL_E_INVALIDARG);

    char numStr[64];

    sprintf(numStr, "\x1b[%d;%dH", row, column);
    asyncLog(_PRINT, numStr);

    return 0;
}

result_t console_base::hideCursor()
{
    asyncLog(_PRINT, "\x1b[?25l");
    return 0;
}

result_t console_base::showCursor()
{
    asyncLog(_PRINT, "\x1b[?25h");
    return 0;
}

result_t console_base::clear()
{
    asyncLog(_PRINT, "\x1b"
                     "c");
    return 0;
}

#endif

char* read_line()
{
    char* text = (char*)malloc(1024);

    if (fgets(text, 1024, stdin) != NULL) {
        int32_t textLen = (int32_t)qstrlen(text);
        if (textLen > 0 && text[textLen - 1] == '\n')
            text[textLen - 1] = '\0'; // getting rid of newline character
        return text;
    }

    free(text);
    return NULL;
}

#ifndef _WIN32
extern "C" {
char* readline(const char* prompt);
void add_history(char* line);
}
#endif

result_t console_base::readLine(exlib::string msg, exlib::string& retVal,
    AsyncEvent* ac)
{
    if (ac->isSync())
        return CHECK_ERROR(CALL_E_LONGSYNC);

    flushLog();

#ifndef _WIN32
    static bool _init = false;
    static char* (*_readline)(const char*);
    static void (*_add_history)(char*);

    if (!_init) {
        _init = true;

#ifdef __clang__
        void* handle = dlopen("libreadline.dylib", RTLD_LAZY);
#else
        const char* readline_dylib_names[] = {
            "libreadline.so.6",
            "libreadline.so.5",
            "libreadline.so"
        };
        const size_t readline_dylib_names_size = ARRAYSIZE(readline_dylib_names);
        void* handle = 0;

        for (size_t i = 0; i < readline_dylib_names_size; i++) {
            handle = dlopen(readline_dylib_names[i], RTLD_LAZY);
            if (handle)
                break;
        }
#endif

        if (handle) {
            _readline = (char* (*)(const char*))dlsym(handle, "readline");
            _add_history = (void (*)(char*))dlsym(handle, "add_history");
        } else {
            _readline = readline;
            _add_history = add_history;
        }
    }

    if (isatty(fileno(stdin))) {
        char* line;
        const char* lfptr = qstrrchr(msg.c_str(), '\n');

        if (lfptr != NULL) {
            puts(msg.substr(0, lfptr - msg.c_str()).c_str());
            line = _readline(lfptr + 1);
        } else
            line = _readline(msg.c_str());

        if (!line)
            return CHECK_ERROR(LastError());

        if (*line) {
            _add_history(line);
            retVal = line;
        }
        free(line);
    } else
#endif
    {
        std_logger::out(msg);
        char* line = read_line();

        if (!line)
            return CHECK_ERROR(LastError());

        retVal = line;
        free(line);
    }

    return 0;
}
};
