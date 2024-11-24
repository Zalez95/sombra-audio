#include <algorithm>
#include <miniaudio.h>
#include "saudio/Device.h"
#include "saudio/Context.h"
#include "MAWrapper.h"
#include "LogWrapper.h"

namespace saudio {

	Device::Device(const DeviceInfo& info, const Config& config)
	{
		SAUDIO_DEBUG_LOG << "init \"" << info.name << "\"";

		ma_device_info* deviceInfos;
		ma_uint32 deviceCount;
		ma_result result = ma_context_get_devices(Context::getMAContext(), &deviceInfos, &deviceCount, nullptr, nullptr);
		if (result != MA_SUCCESS) {
			SAUDIO_ERROR_LOG << "Failed to retrieve the devices";
			return;
		}

		ma_device_id* deviceId = nullptr;
		for (ma_uint32 i = 0; i < deviceCount; ++i) {
			if (info.name == deviceInfos[i].name) {
				deviceId = &deviceInfos[i].id;
				break;
			}
		}
		if (!deviceId) {
			SAUDIO_ERROR_LOG << "Device \"" << info.name << "\" not found";
			return;
		}

		ma_device_config deviceConfig;
		deviceConfig = ma_device_config_init(ma_device_type_playback);
		deviceConfig.playback.pDeviceID = deviceId;
		deviceConfig.playback.format = toMAFormat(config.decodeFormat);
		deviceConfig.playback.channels = config.decodeChannels;
		deviceConfig.sampleRate = config.decodeSampleRate;
		deviceConfig.dataCallback = &maDeviceDataCallback;
		deviceConfig.pUserData = this;

		mDevice = std::make_unique<ma_device>();
		result = ma_device_init(Context::getMAContext(), &deviceConfig, mDevice.get());
		if (result != MA_SUCCESS) {
			SAUDIO_ERROR_LOG << "Failed to initialize the device \"" << info.name << "\"";
			mDevice = nullptr;
			return;
		}

		SAUDIO_DEBUG_LOG << "Created Device " << mDevice.get();
	}


	Device::~Device()
	{
		if (mDevice) {
			ma_device_uninit(mDevice.get());
			SAUDIO_DEBUG_LOG << "Deleted Device " << mDevice.get();
			mDevice = nullptr;
		}
	}


	bool Device::good()
	{
		return (mDevice != nullptr);
	}


	ma_device* Device::getMADevice()
	{
		return mDevice.get();
	}


	bool Device::addDeviceDataListener(IDeviceDataListener* listener)
	{
		if (!listener || !good()) {
			return false;
		}

		auto it = std::find(mDeviceDataListeners.begin(), mDeviceDataListeners.end(), listener);
		if (it == mDeviceDataListeners.end()) {
			mDeviceDataListeners.push_back(listener);
		}

		return true;
	}


	void Device::removeDeviceDataListener(IDeviceDataListener* listener)
	{
		if (!listener || !good()) {
			return;
		}

		auto it = std::find(mDeviceDataListeners.begin(), mDeviceDataListeners.end(), listener);
		if (it != mDeviceDataListeners.end()) {
			mDeviceDataListeners.erase(it);
		}
	}


	std::vector<Device::DeviceInfo> Device::getDeviceInfos()
	{
		std::vector<DeviceInfo> ret;

		if (Context::good()) {
			ma_device_info* deviceInfos;
			ma_uint32 deviceCount;
			ma_result result = ma_context_get_devices(Context::getMAContext(), &deviceInfos, &deviceCount, nullptr, nullptr);
			if (result == MA_SUCCESS) {
				for (ma_uint32 i = 0; i < deviceCount; ++i) {
					ret.push_back({ deviceInfos[i].name, static_cast<bool>(deviceInfos[i].isDefault) });
				}
			}
			else {
				SAUDIO_ERROR_LOG << "Failed to retrieve the device infos";
			}
		}

		return ret;
	}

// Private functions
	void Device::maDeviceDataCallback(ma_device* device, void* output, const void* input, unsigned int frameCount)
	{
		Device* pDevice = static_cast<Device*>(device->pUserData);
		for (auto listener : pDevice->mDeviceDataListeners) {
			listener->onDeviceData(output, input, frameCount);
		}
	}

}
