#include <string>
#include <algorithm>
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#include "Context.h"

namespace se::audio {

	static constexpr ma_format kDecodeFormat = ma_format_f32;
	static constexpr uint32_t kDecodeChannels = 0;				// native channel count
	static constexpr uint32_t kDecodeSampleRate = 48000;


	struct Context::Impl
	{
		/** A pointer to the LogHandler that used for printing logs */
		LogHandler* logHandler = nullptr;

		/** The Listeners to notify when data is ready to be delivered to or
		 * from the device */
		std::vector<IDeviceDataListener*> deviceDataListeners;

		/** The miniaudio log */
		std::unique_ptr<ma_log> maLog;

		/** The miniaudio context */
		std::unique_ptr<ma_context> maContext;

		/** A pointer to the Device used for playing the sound */
		std::unique_ptr<ma_device> maDevice;

		/** Creates a new Context Impl
		 *
		 * @param	config the parameters of the Context Impl */
		Impl(const CreateConfig& config);

		/** Class destructor */
		~Impl();
	};


	Context::Impl* Context::sImpl = nullptr;
	LogHandler Context::CreateConfig::sDefaultLogHandler = {};


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


	Context::Impl::Impl(const CreateConfig& config) : logHandler(config.logHandler)
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
		if (maDevice) {
			ma_device_uninit(maDevice.get());
			maDevice = nullptr;
		}

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
		return getMAContext() && getMADevice();
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


	ma_device* Context::getMADevice()
	{
		return sImpl? sImpl->maDevice.get() : nullptr;
	}


	bool Context::start(const CreateConfig& config)
	{
		sImpl = new Impl(config);
		if (!getMAContext()) {
			stop();
			return false;
		}

		getLogHandler()->debug("started");
		return true;
	}


	std::vector<Context::DeviceInfo> Context::getDevices()
	{
		std::vector<DeviceInfo> ret;

		if (getMAContext()) {
			ma_device_info* deviceInfos;
			ma_uint32 deviceCount;
			ma_result result = ma_context_get_devices(getMAContext(), &deviceInfos, &deviceCount, nullptr, nullptr);
			if (result == MA_SUCCESS) {
				for (ma_uint32 i = 0; i < deviceCount; ++i) {
					ret.push_back({ deviceInfos[i].name, i });
				}
			}
			else {
				getLogHandler()->error("Failed to retrieve the devices");
			}
		}

		return ret;
	}


	bool Context::setDevice(std::size_t deviceId)
	{
		getLogHandler()->debug(("setDevice(" + std::to_string(deviceId) + ")").c_str());

		ma_device_info* deviceInfos;
		ma_uint32 deviceCount;
		ma_result result = ma_context_get_devices(getMAContext(), &deviceInfos, &deviceCount, nullptr, nullptr);
		if (result != MA_SUCCESS) {
			getLogHandler()->error("Failed to retrieve the devices");
			return false;
		}

		ma_device_config deviceConfig;
		deviceConfig = ma_device_config_init(ma_device_type_playback);
		deviceConfig.playback.pDeviceID = &deviceInfos[deviceId].id;
		deviceConfig.playback.format = kDecodeFormat;
		deviceConfig.playback.channels = kDecodeChannels;
		deviceConfig.sampleRate = kDecodeSampleRate;
		deviceConfig.dataCallback = &maDeviceDataCallback;
		deviceConfig.pUserData = sImpl;

		sImpl->maDevice = std::make_unique<ma_device>();
		result = ma_device_init(getMAContext(), &deviceConfig, sImpl->maDevice.get());
		if (result != MA_SUCCESS) {
			getLogHandler()->error(
				(std::string("Failed to initialize the device ") + deviceInfos[deviceId].name).c_str()
			);
			sImpl->maDevice = nullptr;
			return false;
		}

		getLogHandler()->debug(("setDevice(" + std::to_string(deviceId) + ") end").c_str());
		return true;
	}


	bool Context::addDeviceDataListener(IDeviceDataListener* listener)
	{
		if (!listener || !good()) {
			return false;
		}

		auto it = std::find(sImpl->deviceDataListeners.begin(), sImpl->deviceDataListeners.end(), listener);
		if (it == sImpl->deviceDataListeners.end()) {
			sImpl->deviceDataListeners.push_back(listener);
		}

		return true;
	}


	void Context::removeDeviceDataListener(IDeviceDataListener* listener)
	{
		if (!listener || !good()) {
			return;
		}

		auto it = std::find(sImpl->deviceDataListeners.begin(), sImpl->deviceDataListeners.end(), listener);
		if (it != sImpl->deviceDataListeners.end()) {
			sImpl->deviceDataListeners.erase(it);
		}
	}


	void Context::stop()
	{
		getLogHandler()->debug("stop");

		if (sImpl) {
			delete sImpl;
			sImpl = nullptr;
		}
	}

// Private functions
	void Context::maDeviceDataCallback(ma_device* device, void* output, const void* input, unsigned int frameCount)
	{
		Impl* impl = static_cast<Impl*>(device->pUserData);
		for (auto listener : impl->deviceDataListeners) {
			listener->onDeviceData(output, input, frameCount);
		}
	}

}
