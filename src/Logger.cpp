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

    char buffer[50];
    vsnprintf(buffer, 50, msg, args);
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