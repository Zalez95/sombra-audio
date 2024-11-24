#include <memory>
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#include "saudio/Context.h"
#include "LogWrapper.h"

namespace saudio {

	struct Context::Impl
	{
		/** A pointer to the LogHandler that used for printing logs */
		LogHandler* logHandler = nullptr;

		/** The miniaudio log */
		std::unique_ptr<ma_log> maLog;

		/** The miniaudio context */
		std::unique_ptr<ma_context> maContext;

		/** Creates a new Context Impl
		 *
		 * @param	config the parameters of the Context Impl */
		Impl(const Context::Config& config);

		/** Class destructor */
		~Impl();
	};


	Context::Impl* Context::sImpl = nullptr;
	LogHandler Context::Context::Config::sDefaultLogHandler = {};


	static void myMALogCallback(void*, ma_uint32 level, const char* pMessage)
	{
		auto pLogHandler = Context::getLogHandler();
		if (pLogHandler) {
			switch (level) {
				case MA_LOG_LEVEL_DEBUG:
					pLogHandler->debug(pMessage);
					break;
				case MA_LOG_LEVEL_INFO:
					pLogHandler->info(pMessage);
					break;
				case MA_LOG_LEVEL_WARNING:
					pLogHandler->warning(pMessage);
					break;
				case MA_LOG_LEVEL_ERROR:
					pLogHandler->error(pMessage);
					break;
			}
		}
	}


	Context::Impl::Impl(const Context::Config& config) : logHandler(config.logHandler)
	{
		// Create the miniaudio log
		maLog = std::make_unique<ma_log>();
		ma_result result = ma_log_init(nullptr, maLog.get());
		if (result != MA_SUCCESS) {
			logHandler->error("Log creation error");
			maLog = nullptr;
			return;
		}

		ma_log_register_callback(maLog.get(), ma_log_callback_init(&myMALogCallback, nullptr));

		// Create the miniaudio context
		ma_context_config contextConfig = ma_context_config_init();
		contextConfig.pLog = maLog.get();

		maContext = std::make_unique<ma_context>();
		result = ma_context_init(nullptr, 0, &contextConfig, maContext.get());
		if (result != MA_SUCCESS) {
			logHandler->error("Context creation error");
			maContext = nullptr;
		}
	}


	Context::Impl::~Impl()
	{
		if (maContext) {
			ma_context_uninit(maContext.get());
			maContext = nullptr;
		}

		if (maLog) {
			ma_log_uninit(maLog.get());
			maLog = nullptr;
		}
	}


	bool Context::good()
	{
		return getMAContext();
	}


	LogHandler* Context::getLogHandler()
	{
		return sImpl? sImpl->logHandler : nullptr;
	}


	void* Context::getMALog()
	{
		return sImpl? sImpl->maLog.get() : nullptr;
	}


	ma_context* Context::getMAContext()
	{
		return sImpl? sImpl->maContext.get() : nullptr;
	}


	bool Context::start(const Context::Config& config)
	{
		sImpl = new Impl(config);
		if (!getMAContext()) {
			stop();
			return false;
		}

		SAUDIO_INFO_LOG << "started";
		return true;
	}

	void Context::stop()
	{
		SAUDIO_INFO_LOG << "stop";

		if (sImpl) {
			delete sImpl;
			sImpl = nullptr;
		}
	}

}
