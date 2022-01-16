/**
 * @file Logger.cpp
 * @author Alexander Curtiss apcurtiss@gmail.com
 * @brief 
 * @version 0.1
 * @date 2022-01-16
 * 
 * @copyright Copyright (c) 2022 Ka Moamoa
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "Logger.h"

Logger* Logger::_instance = nullptr;
Mutex Logger::_mutex;

Logger::Logger()
{

}

Logger::~Logger()
{

}

Logger* Logger::get_instance()
{
    _mutex.lock();

    if (_instance == nullptr)
    {
        _instance = new Logger();
    }

    _mutex.unlock();

    return _instance;
}

void Logger::initialize(UnbufferedSerial* serial, trace_level_t trace_level)
{
    _serial = serial;
    _serial->baud(115200);
    _serial->enable_input(false);
    _serial->enable_output(false);

    _trace_level = trace_level;

    _uart = true;

    _initialized = true;
}

void Logger::initialize(SWO_Channel* swo, trace_level_t trace_level)
{
    _swo = swo;

    _trace_level = trace_level;

    _uart = false;

    _initialized = true;
}

void Logger::log(trace_level_t level, const char *msg, ...)
{
    if (level < _trace_level) return;
    else if (!_initialized) return;

    va_list args;
    va_start (args, msg);

    char buffer[200];
    vsnprintf(buffer, 200, msg, args);
    va_end(args);

    if (_uart)
    {
        _serial->enable_output(true);
        printf("[%c] // %s\r\n", _trace_char[level], buffer);
        _serial->enable_output(false);
    }
    else
    {
        _swo->printf("[%c] // %s\r\n", _trace_char[level], buffer);
    }
}