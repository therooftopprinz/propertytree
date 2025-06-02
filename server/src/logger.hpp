#ifndef __PTLOGGER_HPP__
#define __PTLOGGER_HPP__

#include <logless/logger.hpp>

using logless::LOGALL;
using logless::ERROR;
using logless::WARNING;
using logless::INFO;
using logless::DEBUG;
using logless::TRACE;
using logless::log;
using logless::buffer_log_t;

extern logless::logger logger;

template <typename... Ts>
void log(const char* id, Ts... ts)
{
    uint64_t timeNow = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    uint64_t threadId = std::hash<std::thread::id>()(std::this_thread::get_id());
    logger.log(id, timeNow, threadId, ts...);
}

#define IF_LB(bit) if (bit & logger.get_logbit())
#define LOG_ERR(id, args...) log(id, args)
#define LOG_WRN(id, args...) if (logger.get_level() >= WARNING) log("WRN | " id, args)
#define LOG_INF(id, args...) if (logger.get_level() >= INFO)    log("INF | " id, args)
#define LOG_DBG(id, args...) if (logger.get_level() >= DEBUG)   log("DBG | " id, args)
#define LOG_TRC(id, args...) if (logger.get_level() >= TRACE)   log("TRC | " id, args)

constexpr uint64_t LB_DUMP_CLIENT_RAW = uint64_t(1) << 0;
constexpr uint64_t LB_DUMP_CLIENT_PM  = uint64_t(1) << 1;

#endif // __PTLOGGER_HPP__
