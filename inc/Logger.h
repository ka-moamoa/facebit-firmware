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