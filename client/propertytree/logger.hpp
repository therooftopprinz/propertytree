#ifndef __PTLOGGER_HPP__
#define __PTLOGGER_HPP__

#include <logless/logger.hpp>

namespace propertytree
{

using logless::LOGALL;
using logless::ERROR;
using logless::WARNING;
using logless::INFO;
using logless::DEBUG;
using logless::TRACE;
using logless::log;
using logless::buffer_log_t;

template <typename... Ts>
void log(logless::logger& logger, const char* id, Ts... ts)
{
    uint64_t timeNow = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    uint64_t threadId = std::hash<std::thread::id>()(std::this_thread::get_id());
    logger.log(id, timeNow, threadId, ts...);
}

#define PTIF_LB(bit) if (bit & m_logger.get_logbit())
#define PTLOG_ERR(id, args...) log(m_logger, "ERR | " id, args)
#define PTLOG_WRN(id, args...) if (m_logger.get_level() >= WARNING) log(m_logger, "WRN | " id, args)
#define PTLOG_INF(id, args...) if (m_logger.get_level() >= INFO)    log(m_logger, "INF | " id, args)
#define PTLOG_DBG(id, args...) if (m_logger.get_level() >= DEBUG)   log(m_logger, "DBG | " id, args)
#define PTLOG_TRC(id, args...) if (m_logger.get_level() >= TRACE)   log(m_logger, "TRC | " id, args)
#define PTLOG_ERR_S(id) log(m_logger, "ERR | " id)

constexpr uint64_t LB_DUMP_MSG_RAW    = uint64_t(1) << 0;
constexpr uint64_t LB_DUMP_MSG_PROTO  = uint64_t(1) << 1;
constexpr uint64_t LB_DUMP_MSG_SOCK   = uint64_t(1) << 2;

} // namespace propertytree

#endif // __PTLOGGER_HPP__
