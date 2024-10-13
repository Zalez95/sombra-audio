#ifndef SAUDIO_LOG_WRAPPER_H
#define SAUDIO_LOG_WRAPPER_H

#include <ostream>
#include <stdext/StringUtils.h>
#include "saudio/Context.h"

namespace saudio {

	/** The different log levels */
	enum class LogLevel : int { Error = 0, Warning, Info, Debug };


	/**
	 * Class LogStream, it's used to write logs with the LogHandler in a stream
	 * like fashion
	 */
	template <typename CharT, std::streamsize Size, LogLevel Level>
	class LogStream : public std::basic_ostream<CharT>
	{
	private:	// Attributes
		/** The buffer where the traces will be stored */
		stdext::ArrayStreambuf<CharT, Size> mASBuf;

	public:		// Functions
		/** Creates a new LogStream */
		LogStream() : std::basic_ostream<CharT>(&mASBuf) {};

		/** Class destructor */
		~LogStream()
		{
			if constexpr (Level == LogLevel::Error) {
				Context::getLogHandler()->error(mASBuf.data());
			}
			else if constexpr (Level == LogLevel::Warning) {
				Context::getLogHandler()->warning(mASBuf.data());
			}
			else if constexpr (Level == LogLevel::Info) {
				Context::getLogHandler()->info(mASBuf.data());
			}
			else {
				Context::getLogHandler()->debug(mASBuf.data());
			}
		};
	};

}

#define FORMAT_LOCATION(function, line) function << "(" << line << "): "
#define LOCATION FORMAT_LOCATION(__func__, __LINE__)

#define SAUDIO_ERROR_LOG	\
	LogStream<char, 512, LogLevel::Error>() << LOCATION
#define SAUDIO_WARN_LOG	\
	LogStream<char, 512, LogLevel::Warning>() << LOCATION
#define SAUDIO_INFO_LOG	\
	LogStream<char, 512, LogLevel::Info>() << LOCATION
#define SAUDIO_DEBUG_LOG	\
	LogStream<char, 512, LogLevel::Debug>() << LOCATION

#endif		// SAUDIO_LOG_WRAPPER_H
