#ifndef SAUDIO_CONTEXT_H
#define SAUDIO_CONTEXT_H

#include "Constants.h"

struct ma_context;

namespace saudio {

	/**
	 * Class LogHandler, it's the class that must be inherited from if someone
	 * wants to print the audio traces. The default behavior is to
	 * not do anything with the traces.
	 */
	class LogHandler
	{
	public:		// Functions
		/** Class destructor */
		virtual ~LogHandler() = default;

		/** Traces error logs
		 *
		 * @param	str the trace string to log */
		virtual void error(const char* /* str */) {};

		/** Traces warning logs
		 *
		 * @param	str the trace string to log */
		virtual void warning(const char* /* str */) {};

		/** Traces info logs
		 *
		 * @param	str the trace string to log */
		virtual void info(const char* /* str */) {};

		/** Traces debug logs
		 *
		 * @param	str the trace string to log */
		virtual void debug(const char* /* str */) {};
	};


	/**
	 * Class Context, it's the class used for initializing the
	 * communications with the Audio device
	 */
	class Context
	{
	public:		// Nested types
		/** Struct Config, holds all the parameters needed for
		 * initializing the Context */
		struct Config
		{
			/** The default LogHandler */
			static LogHandler sDefaultLogHandler;

			/** A pointer to the LogHandler that will be used for printing logs
			 * by the Context */
			LogHandler* logHandler = &sDefaultLogHandler;
		};
	private:
		struct Impl;

	private:	// Attributes
		/** A pointer to the single instance that holds all the implementation
		 * details of the Context */
		static Impl* sImpl;

	public:		// Functions
		/** @return	true if the Context was successfully started with
		 *			@see start, false otherwise */
		static bool good();

		/** @return	a pointer to the LogHandler of the Context, if available */
		static LogHandler* getLogHandler();

		/** @return	a pointer to the miniaudio logger, if available */
		static void* getMALog();

		/** @return	a pointer to the miniaudio Context, if available */
		static ma_context* getMAContext();

		/** Initializes the Context
		 *
		 * @param	config the parameters of the Context
		 * @return	true on success, false otherwise */
		static bool start(const Config& config);

		/** Destroys the Context instance and releases all the
		 * resources */
		static void stop();
	};

}

#endif		// SAUDIO_CONTEXT_H
