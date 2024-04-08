/*****************************************************************************
  FALCON2 - The Falcon Programming Language
  FILE: logger.h

  Default log singleton for Falcon applications
  -------------------------------------------------------------------
  Author: Giancarlo Niccolai
  Begin : Thu, 28 Feb 2019 21:04:31 +0000
  Touch : Sat, 02 Mar 2019 11:12:05 +0000

  -------------------------------------------------------------------
  (C) Copyright 2019 The Falcon Programming Language
  Released under Apache 2.0 License.
******************************************************************************/

#ifndef _FALCON_LOGGER_H_
#define _FALCON_LOGGER_H_

#include <falcon/logsystem.h>
#include <falcon/logstream.h>
#include <falcon/logproxy.h>
#include <falcon/singleton.h>
#include <sstream>

/** Compile-time log filter */
#ifndef FALCON_MIN_LOG_LEVEL
#define FALCON_MIN_LOG_LEVEL ::falcon::LLTRACE
#endif

namespace falcon {

using LOGLEVEL = LogSystem::LEVEL;

constexpr auto LLDISABLE = -1;
constexpr auto LLCRIT = LogSystem::LEVEL::CRITICAL;
constexpr auto LLERR = LogSystem::LEVEL::ERROR;
constexpr auto LLWARN = LogSystem::LEVEL::WARN;
constexpr auto LLINFO = LogSystem::LEVEL::INFO;
constexpr auto LLDEBUG = LogSystem::LEVEL::DEBUG;
constexpr auto LLTRACE = LogSystem::LEVEL::TRACE;

/**
 * Application-wide Logger.
 *
 * This is the main implementation of the C++ level, compile-time optimised log system.
 *
 * It wraps the LogSystem class and provides a set of utilities to produce logs, in an
 * elegant and simple way.
 *
 * The singleton Logger::instance() can be accessed through the LOGGER macro.
 *
 * @section Logger_init Initialization
 * The Logger is initialised as a singleton, with a default LogStreamListener that is
 * initially mute. To initialise the log system, provide a concrete stream where to
 * log, using the defaultLogger() accessor, or provide your own listener.
 *
 * @code
 *   // Write on a stream without getting ownership.
 * LOGGER.defaultListener()->writeOn(&std::cout);
 *   // Get shared ownership of a stream.
 * std::shared_ptr<MyStream> ms = std::make_shared<MyStream>(...);
 * LOGGER.defaultListener()->share(ms);
 * @endcode
 *
 * @NOTE: Changing output stream is a LogListenerStream is threadsafe.
 *
 * @section Logging Introduction
 *
 * The Logger class offer a stream-like interface, which receives chains of redirection
 * operations. Any std::ostream redirection is accepted; it is not necessary to provide
 * special overrides for the log system.
 *
 * Logging can be done through macro helpers, or through low-level accessors
 * in the Logger::instance(). Macros are granted not to generate any code in case
 * of compile-time log level filtering; also, the fast path of runtime level filtering
 * is will no stringify the components of the log line if not necessary.
 *
 * The underlying LogSystem class is used to perform runtime log filtering. The following
 * log levels are provided:
 * - LogSystem::LEVEL::CRITICAL (Aliased as LLCRIT)
 * - LogSystem::LEVEL::ERROR = LLERR
 * - LogSystem::LEVEL::WARN = LLWARN
 * - LogSystem::LEVEL::INFO =  LLINFO
 * - LogSystem::LEVEL::DEBUG = LLDEBUG
 * - LogSystem::LEVEL::TRACE = LLTRACE
 *
 * Runtime level filtering is set using LogSystem::level():
 * @code
 *   // The default level is TRACE
 *   LOG_INFO << "This will be logged";
 *   LOGGER.level(falcon::LLWARN);
 *   LOG_INFO << "This will not be logged";
 * @endcode
 *
 *
 * @NOTE: The current log level can be determined through, LogSystem::level() but is
 * usually better to rely on @a Logger_block_log blocks.
 *
 * @section Logger_single_line_logging Single Line Logging
 *
 * The LOG(level) macro starts a stream-like log-line, which get flushed at the end
 * of the statement.
 *
 * @code
 * int number = 42;
 * LOG(falcon::LLINFO) << "The magic number is " << number << ".";
 * @endcode
 *
 * The stream operators are using std::ostream for serialization of the log line; it is
 * not necessary to provide a log-specific rendering. If a serialization for std::ostream
 * is available, it will be used in place.
 *
 * The serialization only happens if the current log level filtering results in the log
 * line being generated.
 *
 * The following shortcut macros are available for logging at a certain level:
 * - LOG_CRIT = LOG(falcon::LLCRIT)
 * - LOG_ERR  = LOG(falcon::LLERR)
 * - LOG_WARN = LOG(falcon::LLWARN)
 * - LOG_INFO = LOG(falcon::LLINFO)
 * - LOG_DBG  = LOG(falcon::LLDEBUG)
 * - LOG_TRC  = LOG(falcon::LLTRACE)
 *
 * @section Logger_log_block Log Blocks
 *
 * Sometimes, it's not easy or desirable to generate a log line out of a single statement.
 * the macro LOG_BLOCK(level) opens a block that is executed only if necessary. The log message
 * is generated by using the stream operators on the Logger::instance() (LOGGER macro).
 *
 * @code
 * LOG_BLOCK(falcon::LLINFO) {
 * 	  LOGGER << "Hello 3 times: ";
 * 	  for(int i = 0; i < 3; ++i) {
 * 	     LOGGER << "Hello " << (i+1) << ";"
 * 	  }
 * 	  LOGGER << " done"
 * }
 * @endcode
 *
 * The result message is: "Hello 3 times: Hello 1; Hello 2; Hello 3; done"
 *
 * The block will be skipped entirely if the log level is not high enough.
 *
 * The following shortcut macros are provided:
 *
 * - LOG_BLOCK_CRIT = LOG_BLOCK(falcon::LLCRIT)
 * - LOG_BLOCK_ERR  = LOG_BLOCK(falcon::LLERR)
 * - LOG_BLOCK_WARN = LOG_BLOCK(falcon::LLWARN)
 * - LOG_BLOCK_INFO = LOG_BLOCK(falcon::LLINFO)
 * - LOG_BLOCK_DBG  = LOG_BLOCK(falcon::LLDEBUG)
 * - LOG_BLOCK_TRC  = LOG_BLOCK(falcon::LLTRACE)
 *
 * @section Logger_ct_optimization Compile time optimisation
 *
 * The macro FALCON_MIN_LOG_LEVEL controls compile time optimisation of the MACRO-based log
 * facility: LOG(), LOG_BLOCK() and their aliases. Some part of the Logger class are also
 * optimised depending on the value of this macro. By default, the macro is set as
 * falcon::LLTRACE, meaning that all log statements are compiled. Use falcon::LLDISABLE to
 * excise all log statements at compile time.
 *
 * @section Logger_category Category and category-level filtering
 *
 * The base class LogSystem and its LogListener components allow for fine-tuning of runtime logging
 * through the concept of a log "category", which is a string description of the reason why a log is
 * performed.
 *
 * A category is set for the invoking thread by the Logger::setCategory method, or its helper macro
 * LOG_CATEGORY(name), and all the entries are logged under that category until a different one is set.
 * For example:
 *
 * @code
 * LOG_CATEGORY("First part");
 *
 * LOG_INFO << "First entry";
 * LOG_INFO << "Second entry";
 *
 * LOG_CATEGORY("Second part");
 *
 * LOG_INFO << "Third entry";
 * LOG_INFO << "Fourth entry";
 * @endcode
 *
 * Categories can be filtered via a regex match, by a LogListener only. If you plan to have a
 * category-based filter, it is advisable to let the general level of the logger as high as
 * necessary (possibly, as high as the highest level that a listeners wants to log), and set the
 * category filter in the desired listener.
 *
 * For example, the following code logs normally at level INFO, but logs separately the
 * "INTENAL" sub-category at trace level:
 *
 * @code
 *   // Prevents normal logging of uninteresting stuff
 * LOGGER.defaultListener()->level(falcon::LLINFO);
 * auto catListener = std::make_shared<falcon::LogStreamListener>();
 *
 *   // Listens for trace-level logs in the INTERNAL sub-category
 * catListener->writenOn(&std::cout);
 * catListener->category(".*::INTERNAL");
 * LOGGER.addListener(catListener);
 *
 *   // Will be only logged by the default listener
 * LOG_INFO << "Generic log entry";
 *
 * LOG_CATEGORY("TestClass::INTERNAL");
 * LOG_TRACE << "Category-specific log entry";
 * @endcode
 *
 * In this case, runtime optimisation becomes impossible, as all the logs are sent to the pool of
 * listeners, which perform category checks in the separate log threads. It is advisable to
 * raise the Logger::level() general level as soon as the category-specific logs are not
 * needed anymore.
 *
 * @NOTE: Changing the log category filter is threadsafe operation.
 *
 * @subsection Logger_category_temp Temporary Category
 *
 * At times it's useful to override the default category for the file, code area, section etc.
 * for a single message only.
 *
 * The manipulator msg_cat(), proxied with the macro LOG_CAT() serves the purpose.
 * @code
 * // Sets the default category
 * LOG_CATEGORY("Default");
 * // This message has a special category
 * LOG_TRACE << LOG_CAT("EXEC") << "A trace message in the EXEC category";
 * // the message-specific category is reset.
 * LOG_INFO << "A message in the Default category";
 * @endcode
 *
 *
 * @subsection Logger_category_help Logger category filter helper
 *
 * The method Logger::filterCategory() performs an operation similar to the one shown above:
 *
 * - it lowers the default listener log level to the current general log level.
 * - it raises the general log level as necessary
 * - it creates a new listener with a configured category filter at the desired log level,
 *   streaming on the same stream used by the default listener.
 *
 * This helps implementing application-level emergency log features.
 *
 * Notice that only one filter created Logger::categoryFilter() will be active at a time.
 * To reset the logger to the previous state, use Logger::clearFilter().
 *
 * @Note: setting the category filter at Logger level is threadsafe with respect to the log
 * thread, but concurrent usage of the filter set and clear functions from differen threads have
 * undefined behavior.
 *
 */
class Logger: public LogSystem
{
public:

	Logger();

	static Logger& instance() {
		static Logger theLogger;
		return theLogger;
	}

	std::shared_ptr<LogStreamListener> defaultListener() const {
		return m_dflt;
	}

	void setCategory(const std::string& category) noexcept {
		m_category = category;
	}

	void setTempCategory(const std::string& category) noexcept {
		m_tempCategory = category;
	}

	const std::string& getCategory() const noexcept {
		return m_category;
	}

	void commit()
	{
		if(m_tempCategory.empty()) {
			log(m_msgFile, m_msgLine, m_msgLevel, m_category, m_composer.str());
		}
		else {
			log(m_msgFile, m_msgLine, m_msgLevel, m_tempCategory, m_composer.str());
			m_tempCategory.clear();
		}
		readyStream();
	}

	void setLevel(LOGLEVEL lvl) {
		m_msgLevel = lvl;
	}

	void setFile(const std::string& file) {
		m_msgFile = file;
	}

	void setLine(int line) {
		m_msgLine = line;
	}

	void categoryFilter(const std::string& line, LOGLEVEL l=LLTRACE);
	void clearFilter();

	class AutoEnd
	{
	private:
	    Logger* m_obj;
	    std::string m_category;

	public:
	    explicit AutoEnd (Logger& obj, const std::string& file, int line, LOGLEVEL lvl):
			m_obj(nullptr)
	    {
	    	if(FALCON_MIN_LOG_LEVEL >= lvl && obj.level() >= lvl) {
	    		obj.setFile(file);
	    		obj.setLine(line);
	    		obj.setLevel(lvl);
	    		m_obj = &obj;
	    	}
	    }

	    ~AutoEnd () {if(m_obj){m_obj->commit();}}
	    bool doLog() const noexcept {return m_obj != nullptr;}
	    Logger& obj () const noexcept {return *m_obj;}
	};

	class BlockEnd
	{
	private:
	    Logger* m_obj;

	public:
	    explicit BlockEnd (Logger& obj, const std::string& file, int line, LOGLEVEL lvl):
			m_obj(nullptr)
	    {
	    	if(FALCON_MIN_LOG_LEVEL >= lvl && obj.level() >= lvl) {
	    		obj.setFile(file);
	    		obj.setLine(line);
	    		obj.setLevel(lvl);
	    		m_obj = &obj;
	    	}
	    }
	    void complete() {m_obj->commit(); m_obj = nullptr;}
	    operator bool() const noexcept {return m_obj != nullptr;}
	};

	// Hearth of the composition.
	template<typename T>
	const Logger& operator<<(const T& v) const {
		m_composer << v;
		return *this;
	}


	struct category_manipulator {
		category_manipulator(const std::string& cat):
			m_cat{cat}
		{}
		std::string m_cat;
	};

	/**
	 * Stream-log style manipulator for temporary (message scoped) category.
	 *
	 * Usage: LOG_INFO << msg_cat("a category") << 1 << 2 << 3;
	 *
	 * The macro LOG_CAT() is provided for abbreviation.
	 */
	static category_manipulator msg_cat(const std::string& category)
	{
		return category_manipulator(category);
	}

private:
	std::shared_ptr<LogStreamListener> m_dflt;
	std::shared_ptr<LogProxyListener> m_proxy;
	LOGLEVEL m_proxyBaseLevel{LLTRACE};

	static thread_local std::ostringstream m_composer;
	static thread_local std::string m_category;
	static thread_local std::string m_tempCategory;
	static thread_local std::string m_msgFile;
	static thread_local int m_msgLine;
	static thread_local LOGLEVEL m_msgLevel;

	void readyStream() noexcept {
		m_composer.clear();
		m_composer.str("");
	}
};


template <typename T>
const Logger::AutoEnd& operator << (const Logger::AutoEnd& aes, T&& arg)
{
	if (aes.doLog()) {
		aes.obj() << std::forward<T>(arg);
	}
    return aes;
}

const Logger::AutoEnd& operator<<(const Logger::AutoEnd&& aes, Logger::category_manipulator&& cat)
{
    if(aes.doLog()) {
    	aes.obj().setTempCategory(cat.m_cat);
    }
    return aes;
}

#define LOGGER (::falcon::Logger::instance())
#define LOG_CATEGORY(__CAT) if(FALCON_MIN_LOG_LEVEL >= LOGGER.level()){LOGGER.setCategory(__CAT);}

#define LOG(__LVL) (::falcon::Logger::AutoEnd(LOGGER, __FILE__, __LINE__, __LVL))
#define LOG_CRIT LOG(::falcon::LLCRIT)
#define LOG_ERR  LOG(::falcon::LLERR)
#define LOG_WARN LOG(::falcon::LLWARN)
#define LOG_INFO LOG(::falcon::LLINFO)
#define LOG_DBG  LOG(::falcon::LLDEBUG)
#define LOG_TRC  LOG(::falcon::LLTRACE)
#define LOG_CAT  ::falcon::Logger::msg_cat

#define LOG_BLOCK(lvl) \
	for( ::falcon::Logger::BlockEnd __ender(LOGGER, __FILE__, __LINE__, lvl); \
		FALCON_MIN_LOG_LEVEL >= lvl && LOGGER.level() >= lvl && __ender; \
		__ender.complete() )

#define LOG_BLOCK_CRIT LOG_BLOCK(::falcon::LLCRIT)
#define LOG_BLOCK_ERR  LOG_BLOCK(::falcon::LLERR)
#define LOG_BLOCK_WARN LOG_BLOCK(::falcon::LLWARN)
#define LOG_BLOCK_INFO LOG_BLOCK(::falcon::LLINFO)
#define LOG_BLOCK_DBG  LOG_BLOCK(::falcon::LLDEBUG)
#define LOG_BLOCK_TRC  LOG_BLOCK(::falcon::LLTRACE)

}

#endif /* _FALCON_LOGGER_H_ */

/* end of logger.h */

