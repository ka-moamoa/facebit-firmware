#ifndef LOGGER_H_
#define LOGGER_H_

#include "SWO.h"

extern SWO_Channel SWO;

enum
{
    TRACE_TRACE,
    TRACE_DEBUG,
    TRACE_INFO,
    TRACE_WARNING
};

const uint8_t trace_level = TRACE_TRACE;

#define LOG_TRACE(msg, ...) \
{ \
    if (trace_level <= TRACE_TRACE) \
    { \
        printf("[T] // %s, %i --> " msg "\r\n",  __FILE__, __LINE__, __VA_ARGS__); \
    } \
}

#define LOG_DEBUG(msg, ...) \
{ \
    if (trace_level <= TRACE_DEBUG) \
    { \
        printf("[D] // %s, %i --> " msg "\r\n",  __FILE__, __LINE__, __VA_ARGS__); \
    } \
}

#define LOG_INFO(msg, ...) \
{ \
    if (trace_level <= TRACE_INFO) \
    { \
        printf("[I] // %s, %i --> " msg "\r\n",  __FILE__, __LINE__, __VA_ARGS__); \
    } \
}

#define LOG_WARNING(msg, ...) \
{ \
    if (trace_level <= TRACE_WARNING) \
    { \
        printf("[W] // %s, %i --> " msg "\r\n",  __FILE__, __LINE__, __VA_ARGS__); \
    } \
}
#endif // LOGGER_H_