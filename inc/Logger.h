/**
 * @file Logger.h
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

#ifndef LOGGER_H_
#define LOGGER_H_

#include "mbed.h"
#include "rtos.h"
#include "UnbufferedSerial.h"
#include "SWO.h"

typedef enum
{
    TRACE_TRACE,
    TRACE_DEBUG,
    TRACE_INFO,
    TRACE_WARNING,
    TRACE_LAST
} trace_level_t;


class Logger
{
public:
    Logger(Logger &other) = delete;
    void operator=(const Logger &) = delete;

    static Logger* get_instance();
    void initialize(UnbufferedSerial* serial, trace_level_t trace_level);
    void initialize(SWO_Channel* swo, trace_level_t trace_level);
    void log(trace_level_t level, const char *msg, ...);

private:
    Logger();
    ~Logger();

    static Mutex _mutex;
    static Logger* _instance;
    UnbufferedSerial* _serial = nullptr;
    SWO_Channel* _swo = nullptr;
    trace_level_t _trace_level = TRACE_TRACE;
    const char _trace_char[TRACE_LAST] = {'T', 'D', 'I', 'W'};
    bool _uart = false;
    bool _initialized = false;
};

#endif // LOGGER_H_