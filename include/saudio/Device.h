#ifndef SAUDIO_DEVICE_H
#define SAUDIO_DEVICE_H

#include <memory>
#include <vector>
#include <string>
#include "Constants.h"

struct ma_device;

namespace saudio {

	/**
	 * Class Device, it's the class used for initializing the
	 * communications with the Audio device
	 */
	class Device
	{
	public:		// Nested types
		/** Holds the info about an audio device */
		struct DeviceInfo
		{
			std::string name;	///< The name of the device
			bool isDefault;		///< If it's the default device or not
		};

		/** Struct DeviceConfig, holds all the parameters needed for
		 * setting the Device */
		struct DeviceConfig
		{
			Format decodeFormat = Format::f32;
			uint32_t decodeChannels = 0;
			uint32_t decodeSampleRate = 48000;
		};

		/** Class IDeviceDataListener, it's the interface that should be
		 * implemented for receiving notifications of new device data */
		class IDeviceDataListener
		{
		public:		// Functions
			/** Class destructor */
			virtual ~IDeviceDataListener() = default;

			/** The function callback used for notifying the listener of new
			 * Device data
			 *
			 * @param	output a pointer to the output data
			 * @param	input a pointer to the input data
			 * @param	frameCount the limit of frames to process */
			virtual void onDeviceData(
				void* output, const void* input, unsigned int frameCount
			) = 0;
		};

	private:	// Attributes
		/** A pointer to the Device */
		std::unique_ptr<ma_device> mDevice;

		/** The Listeners to notify when data is ready to be delivered to or
		 * from the device */
		std::vector<IDeviceDataListener*> mDeviceDataListeners;

	public:		// Functions
		/** Creates a new Device
		 *
		 * @param	info the DeviceInfo of the device to create
		 * @param	config the parameters of the device to create */
		Device(const DeviceInfo& deviceInfo, const DeviceConfig& config);

		/** Class destructor */
		~Device();

		/** @return	true if the Device was created successfully,
		 *			false otherwise */
		bool good();

		/** @return	a pointer to the miniaudio Device, if available */
		ma_device* getMADevice();

		/** Adds the given IDeviceDataListener to the Device
		 *
		 * @param	listener the IDeviceDataListener to notify on new Device
		 *			data
		 * @return	true on success, false otherwise */
		bool addDeviceDataListener(IDeviceDataListener* listener);

		/** Removes the given IDeviceDataListener from the Device
		 *
		 * @param	listener the IDeviceDataListener that won¡t longer be
		 *			notified on new Device data */
		void removeDeviceDataListener(IDeviceDataListener* listener);

		/** @return	the DeviceInfos of all the Devices that can be used */
		static std::vector<DeviceInfo> getDeviceInfos();
	private:
		/** The callback that will be executed when the device has new data is
		 * available
		 *
		 * @param	device a pointer to the miniaudio device
		 * @param	output a pointer to the new output data
		 * @param	input a pointer to the new input data
		 * @param	frameCount the number of frames available */
		static void maDeviceDataCallback(
			ma_device* device,
			void* output, const void* input, unsigned int frameCount
		);
	};

}

#endif		// SAUDIO_DEVICE_H
