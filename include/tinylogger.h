#ifndef TINYWORLD_TINYLOGGER_H
#define TINYWORLD_TINYLOGGER_H

#include <cstdarg>
#include <cstdio>

//////////////////////////////////////////////////////////
//
// Logger Wrapper:
//
//      TRACE < DEBUG < INFO < WARN < ERROR << FATAL
//
//  - C printf style:
//       LOG_TRACE(logger-name, fmt, ...)
//       LOG_DEBUG(logger-name, fmt, ...)
//       LOG_INFO (logger-name, fmt, ...)
//
//       LOG_WARN (logger-name, fmt, ...)
//       LOG_ERROR(logger-name, fmt, ...)
//       LOG_FATAL(logger-name, fmt, ...)
//
//  - C++ stream style:
//       LOGGER_TRACE(logger-name, a << b << ...)
//       LOGGER_DEBUG(logger-name, a << b << ...)
//       LOGGER_INFO (logger-name, a << b << ...)
//
//       LOGGER_WARN (logger-name, a << b << ...)
//       LOGGER_ERROR(logger-name, a << b << ...)
//       LOGGER_FATAL(logger-name, a << b << ...)
//
/////////////////////////////////////////////////////////


#define LOG_LENGTH_MAX 1024

#ifndef TINYLOGGER_LOG4CXX
#define     TINYLOGGER_SIMPLE
#endif

//////////////////////////////////////////////////////////
//
// logger using std::cout/std::cerr
//
/////////////////////////////////////////////////////////

#ifdef TINYLOGGER_SIMPLE

#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>

#define SIMPLE_LOGGER_TRACE 1
#define SIMPLE_LOGGER_DEBUG 2
#define SIMPLE_LOGGER_INFO  3
#define SIMPLE_LOGGER_WARN  4
#define SIMPLE_LOGGER_ERROR 5
#define SIMPLE_LOGGER_FATAL 6

//
// C printf style
//
#define LOG_TRACE(loggername, fmt, ...) \
        SimpleLogger::instance().print_log(SIMPLE_LOGGER_TRACE, loggername, __FILE__, __LINE__, __PRETTY_FUNCTION__, fmt, ##__VA_ARGS__)

#define LOG_DEBUG(loggername, fmt, ...) \
        SimpleLogger::instance().print_log(SIMPLE_LOGGER_DEBUG, loggername, __FILE__, __LINE__, __PRETTY_FUNCTION__, fmt, ##__VA_ARGS__)

#define LOG_INFO(loggername, fmt, ...) \
        SimpleLogger::instance().print_log(SIMPLE_LOGGER_INFO, loggername, __FILE__, __LINE__, __PRETTY_FUNCTION__, fmt, ##__VA_ARGS__)

#define LOG_WARN(loggername, fmt, ...) \
        SimpleLogger::instance().print_log(SIMPLE_LOGGER_WARN, loggername, __FILE__, __LINE__, __PRETTY_FUNCTION__, fmt, ##__VA_ARGS__)

#define LOG_ERROR(loggername, fmt, ...) \
        SimpleLogger::instance().print_log(SIMPLE_LOGGER_ERROR, loggername, __FILE__, __LINE__, __PRETTY_FUNCTION__, fmt, ##__VA_ARGS__)

#define LOG_FATAL(loggername, fmt, ...) \
        SimpleLogger::instance().print_log(SIMPLE_LOGGER_FATAL, loggername, __FILE__, __LINE__, __PRETTY_FUNCTION__, fmt, ##__VA_ARGS__)

//
// C++ streambuf style
//
#define LOGGER_TRACE(loggername, message) { \
        std::ostringstream oss_; \
        oss_ << message; \
        SimpleLogger::instance().print_log(SIMPLE_LOGGER_TRACE, loggername, __FILE__, __LINE__, __PRETTY_FUNCTION__, oss_.str().c_str()); \
    }

#define LOGGER_DEBUG(loggername, message) { \
        std::ostringstream oss_; \
        oss_ << message; \
        SimpleLogger::instance().print_log(SIMPLE_LOGGER_DEBUG, loggername, __FILE__, __LINE__, __PRETTY_FUNCTION__, oss_.str().c_str()); \
    }

#define LOGGER_INFO(loggername, message) { \
        std::ostringstream oss_; \
        oss_ << message; \
        SimpleLogger::instance().print_log(SIMPLE_LOGGER_INFO, loggername, __FILE__, __LINE__, __PRETTY_FUNCTION__, oss_.str().c_str()); \
    }

#define LOGGER_WARN(loggername, message) { \
        std::ostringstream oss_; \
        oss_ << message; \
        SimpleLogger::instance().print_log(SIMPLE_LOGGER_WARN, loggername, __FILE__, __LINE__, __PRETTY_FUNCTION__, oss_.str().c_str()); \
    }

#define LOGGER_ERROR(loggername, message) { \
        std::ostringstream oss_; \
        oss_ << message; \
        SimpleLogger::instance().print_log(SIMPLE_LOGGER_ERROR, loggername, __FILE__, __LINE__, __PRETTY_FUNCTION__, oss_.str().c_str()); \
    }

#define LOGGER_FATAL(loggername, message) { \
        std::ostringstream oss_; \
        oss_ << message; \
        SimpleLogger::instance().print_log(SIMPLE_LOGGER_FATAL, loggername, __FILE__, __LINE__, __PRETTY_FUNCTION__, oss_.str().c_str()); \
    }


struct SimpleLogger
{
    static SimpleLogger& instance()
    {
        static SimpleLogger logger_instance;
        return logger_instance;
    }

    const char* level_name(int level)
    {
        switch (level)
        {
            case SIMPLE_LOGGER_TRACE : return "TRACE";
            case SIMPLE_LOGGER_DEBUG : return "DEBUG";
            case SIMPLE_LOGGER_INFO  : return "INFO";
            case SIMPLE_LOGGER_WARN  : return "WARN";
            case SIMPLE_LOGGER_ERROR : return "ERROR";
            case SIMPLE_LOGGER_FATAL : return "FATAL";
        }
        return "";
    }

    void print_log(int level,
                   const char *loggername,
                   const char *file,
                   int line,
                   const char *func,
                   const char *fmt, ...)
    {
        char buf[LOG_LENGTH_MAX] = "";
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);

        char nowstr[64] = "";
        std::time_t now = std::time(NULL);
        std::strftime(nowstr, sizeof(nowstr), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

        std::ostream *output = &std::cout;
//        if (level >= SIMPLE_LOGGER_WARN)
//            output = &std::cerr;

        (*output) << nowstr
                  << " " << std::setfill(' ') << std::setw(5) << level_name(level) << ": "
                  << "[" << loggername << "] - " << buf
                  << std::endl;
        output->flush();
    }
};


#endif // TINYLOGGER_SIMPLE


//////////////////////////////////////////////////////////
//
// logger using log4cxx
//
// ./configure --with-charset=utf-8 --with-logchar=utf-8
//
/////////////////////////////////////////////////////////

#ifdef TINYLOGGER_LOG4CXX

#include "log4cxx/logger.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"

using log4cxx::LoggerPtr;
using log4cxx::Logger;
using log4cxx::BasicConfigurator;
using log4cxx::PropertyConfigurator;

// BasicConfigurator::configure();
// PropertyConfigurator::configure(filename);
// PropertyConfigurator::configureAndWatch(filename);

// LOG4CXX_TRACE
// LOG4CXX_DEBUG
// LOG4CXX_INFO
// LOG4CXX_WARN
// LOG4CXX_ERROR
// LOG4CXX_FATAL
// LOG4CXX_ASSERT

//
// C printf style
//
#define LOG_TRACE(loggername, fmt, ...) { \
        log4cxx::LoggerPtr logger_(log4cxx::Logger::getLogger(loggername)); \
        if (LOG4CXX_UNLIKELY(logger_->isTraceEnabled())) {\
            char buf_[LOG_LENGTH_MAX] = ""; \
            snprintf(buf_, sizeof(buf_), fmt, ##__VA_ARGS__ ); \
            logger_->forcedLog(log4cxx::Level::getTrace(), buf_, LOG4CXX_LOCATION); \
        }}

#define LOG_DEBUG(loggername, fmt, ...) { \
        log4cxx::LoggerPtr logger_(log4cxx::Logger::getLogger(loggername)); \
        if (LOG4CXX_UNLIKELY(logger_->isDebugEnabled())) {\
            char buf_[LOG_LENGTH_MAX] = ""; \
            snprintf(buf_, sizeof(buf_), fmt, ##__VA_ARGS__ ); \
            logger_->forcedLog(log4cxx::Level::getDebug(), buf_, LOG4CXX_LOCATION); \
        }}

#define LOG_INFO(loggername, fmt, ...) { \
        log4cxx::LoggerPtr logger_(log4cxx::Logger::getLogger(loggername)); \
        if (LOG4CXX_UNLIKELY(logger_->isInfoEnabled())) {\
            char buf_[LOG_LENGTH_MAX] = ""; \
            snprintf(buf_, sizeof(buf_), fmt, ##__VA_ARGS__ ); \
            logger_->forcedLog(log4cxx::Level::getInfo(), buf_, LOG4CXX_LOCATION); \
        }}

#define LOG_WARN(loggername, fmt, ...) { \
        log4cxx::LoggerPtr logger_(log4cxx::Logger::getLogger(loggername)); \
        if (LOG4CXX_UNLIKELY(logger_->isWarnEnabled())) {\
            char buf_[LOG_LENGTH_MAX] = ""; \
            snprintf(buf_, sizeof(buf_), fmt, ##__VA_ARGS__ ); \
            logger_->forcedLog(log4cxx::Level::getWarn(), buf_, LOG4CXX_LOCATION); \
        }}

#define LOG_ERROR(loggername, fmt, ...) { \
        log4cxx::LoggerPtr logger_(log4cxx::Logger::getLogger(loggername)); \
        if (LOG4CXX_UNLIKELY(logger_->isErrorEnabled())) {\
            char buf_[LOG_LENGTH_MAX] = ""; \
            snprintf(buf_, sizeof(buf_), fmt, ##__VA_ARGS__ ); \
            logger_->forcedLog(log4cxx::Level::getError(), buf_, LOG4CXX_LOCATION); \
        }}

#define LOG_FATAL(loggername, fmt, ...) { \
        log4cxx::LoggerPtr logger_(log4cxx::Logger::getLogger(loggername)); \
        if (LOG4CXX_UNLIKELY(logger_->isFatalEnabled())) {\
            char buf_[LOG_LENGTH_MAX] = ""; \
            snprintf(buf_, sizeof(buf_), fmt, ##__VA_ARGS__ ); \
            logger_->forcedLog(log4cxx::Level::getFatal(), buf_, LOG4CXX_LOCATION); \
        }}


//
// C++ streambuf style
//
#define LOGGER_TRACE(loggername, message) { \
        if (LOG4CXX_UNLIKELY(logger->isTraceEnabled())) {\
            log4cxx::helpers::MessageBuffer oss_; \
            log4cxx::LoggerPtr logger_(log4cxx::Logger::getLogger(loggername)); \
            logger_->forcedLog(::log4cxx::Level::getTrace(), oss_.str(oss_ << message), LOG4CXX_LOCATION); \
        }}

#define LOGGER_DEBUG(loggername, message) { \
        if (LOG4CXX_UNLIKELY(logger->isDebugEnabled())) {\
            log4cxx::helpers::MessageBuffer oss_; \
            log4cxx::LoggerPtr logger_(log4cxx::Logger::getLogger(loggername)); \
            logger_->forcedLog(::log4cxx::Level::getDebug(), oss_.str(oss_ << message), LOG4CXX_LOCATION); \
        }}

#define LOGGER_INFO(loggername, message) { \
        if (LOG4CXX_UNLIKELY(logger->isInfoEnabled())) {\
            log4cxx::helpers::MessageBuffer oss_; \
            log4cxx::LoggerPtr logger_(log4cxx::Logger::getLogger(loggername)); \
            logger_->forcedLog(::log4cxx::Level::getInfo(), oss_.str(oss_ << message), LOG4CXX_LOCATION); \
        }}

#define LOGGER_WARN(loggername, message) { \
        if (LOG4CXX_UNLIKELY(logger->isWarnEnabled())) {\
            log4cxx::helpers::MessageBuffer oss_; \
            log4cxx::LoggerPtr logger_(log4cxx::Logger::getLogger(loggername)); \
            logger_->forcedLog(::log4cxx::Level::getWarn(), oss_.str(oss_ << message), LOG4CXX_LOCATION); \
        }}

#define LOGGER_ERROR(loggername, message) { \
        if (LOG4CXX_UNLIKELY(logger->isErrorEnabled())) {\
            log4cxx::helpers::MessageBuffer oss_; \
            log4cxx::LoggerPtr logger_(log4cxx::Logger::getLogger(loggername)); \
            logger_->forcedLog(::log4cxx::Level::getError(), oss_.str(oss_ << message), LOG4CXX_LOCATION); \
        }}

#define LOGGER_FATAL(loggername, message) { \
        if (LOG4CXX_UNLIKELY(logger->isFatalEnabled())) {\
            log4cxx::helpers::MessageBuffer oss_; \
            log4cxx::LoggerPtr logger_(log4cxx::Logger::getLogger(loggername)); \
            logger_->forcedLog(::log4cxx::Level::getFatal(), oss_.str(oss_ << message), LOG4CXX_LOCATION); \
        }}

#endif // TINYLOGGER_LOG4CXX

#endif //TINYWORLD_TINYLOGGER_H
