#ifndef SERVER_LOGGER_HPP_
#define SERVER_LOGGER_HPP_

#include <sstream>
#include <iostream>
#include <thread>
#include <list>
#include <chrono>
#include <ctime>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>

namespace ptree
{
namespace logger
{

enum class ELogLevel {DEBUG, INFO, WARNING, ERROR};

#define DEBUG ELogLevel::DEBUG
#define INFO ELogLevel::INFO
#define WARNING ELogLevel::WARNING
#define ERROR ELogLevel::ERROR

struct LogEntry
{
    LogEntry():
        level(INFO),
        text(std::string())
    {}

    std::chrono::time_point<
        std::chrono::high_resolution_clock> time;
    std::string name;
    uint64_t threadId;
    ELogLevel level;
    std::string text;
};

class LoggerServer
{
public:
    LoggerServer():
        logProcessorRunning(false),
        killLogProcessor(false)
    {
        timeBase = std::chrono::duration_cast<std::chrono::microseconds>
            (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        std::cout << "LoggerServer will now be created!!" << std::endl;
        std::thread t(std::bind(&LoggerServer::logProcessor, this));
        t.detach();
    }

    ~LoggerServer()
    {
        std::cout << "LoggerServer will now be destroyed!!" << std::endl;
        killLogProcessor = true;
        {
            std::lock_guard<std::mutex> guard(toBeLoggedMutex);
            toBeLoggedCv.notify_one();
        }
        while(logProcessorRunning)
        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(10ms);
        }
    }

    void log(LogEntry logEntry);
    void waitEmpty();

private:
    void logProcessor();
    std::atomic<bool> logProcessorRunning;
    std::atomic<bool> killLogProcessor;
    std::list<LogEntry> toBeLogged;
    std::condition_variable toBeLoggedCv;
    std::mutex toBeLoggedMutex;
    uint64_t timeBase;
};

extern LoggerServer loggerServer;
class Logger;

class Logger
{
public:
    Logger(std::string name):
        name(name)
    {
    }

    std::string getName();
private:
    std::string name;
};

class LoggerStream
{
public:
    LoggerStream(Logger& logger, ELogLevel logLevel):
        logger(logger),
        logLevel(logLevel)
    {

    }

    ~LoggerStream()
    {
        #ifndef __NOLOGSPLEASE__
        entry.name = logger.getName();
        entry.level = logLevel;
        entry.threadId = std::hash<std::thread::id>()(std::this_thread::get_id());
        entry.time =  std::chrono::high_resolution_clock::now();
        loggerServer.log(entry);
        #endif
    }

    template<typename T>
    #ifdef __NOLOGSPLEASE__
    LoggerStream& operator << (T)
    #else
    LoggerStream& operator << (T message)
    #endif
    {
        #ifndef __NOLOGSPLEASE__
        std::ostringstream os;
        os << message;
        entry.text += os.str();
        #endif
        return *this;
    }

private:
    Logger& logger;
    ELogLevel logLevel;
    LogEntry entry;
};

LoggerStream operator << (Logger logger, ELogLevel logLevel);

} // namespace server
} // namespace ptree
#endif // SERVER_LOGGER_HPP_