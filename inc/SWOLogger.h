#ifndef LOGGER_H_
#define LOGGER_H_

#include "SWO.h"

extern SWO_Channel SWO;

enum
{
    TRACE_DEBUG,
    TRACE_INFO,
    TRACE_WARNING
};

const uint8_t trace_level = TRACE_INFO;

#define LOG_DEBUG(msg, ...) \
{ \
    if (trace_level <= TRACE_DEBUG) \
    { \
        SWO.printf("\r\n[D] // %s, %i --> " msg,  __FILE__, __LINE__, __VA_ARGS__); \
    } \
}

#define LOG_INFO(msg, ...) \
{ \
    if (trace_level <= TRACE_INFO) \
    { \
        SWO.printf("\r\n[I] // %s, %i --> " msg,  __FILE__, __LINE__, __VA_ARGS__); \
    } \
}

#define LOG_WARNING(msg, ...) \
{ \
    if (trace_level <= TRACE_WARNING) \
    { \
        SWO.printf("\r\n[W] // %s, %i --> " msg,  __FILE__, __LINE__, __VA_ARGS__); \
    } \
}
#endif // LOGGER_H_