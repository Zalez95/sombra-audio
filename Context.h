#ifndef SEAUDIO_CONTEXT_H
#define SEAUDIO_CONTEXT_H

#include <memory>
#include <vector>
#include "Constants.h"

struct ma_context;
struct ma_device;

namespace se::audio {

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
		/** Struct ContextConfig, holds all the parameters needed for
		 * initializing the Context */
		struct ContextConfig
		{
			/** The default LogHandler */
			static LogHandler sDefaultLogHandler;

			/** A pointer to the LogHandler that will be used for printing logs
			 * by the Context */
			LogHandler* logHandler = &sDefaultLogHandler;
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

		/** Holds the info about an audio device */
		struct DeviceInfo
		{
			std::string name;	///< The name of the device
			std::size_t id;		///< The id of the device
		};
	private:
		struct Impl;

	private:	// Attributes
		/** A pointer to the single instance that holds all the implementation
		 * details of the Context */
		static Impl* sImpl;

	public:		// Functions
		/** @return	true if the Context was started with @see start and a
		 *			Device was selected with @see setDevice, false otherwise */
		static bool good();

		/** @return	a pointer to the LogHandler of the Context, if available */
		static LogHandler* getLogHandler();

		/** @return	a pointer to the miniaudio logger, if available */
		static void* getMALog();

		/** @return	a pointer to the miniaudio Context, if available */
		static ma_context* getMAContext();

		/** @return	a pointer to the miniaudio Device, if available */
		static ma_device* getMADevice();

		/** Initializes the Context
		 *
		 * @param	config the parameters of the Context
		 * @return	true on success, false otherwise */
		static bool start(const ContextConfig& config);

		/** @return	all the sound devices that can be used */
		static std::vector<DeviceInfo> getDevices();

		/** Sets the audio device to use within the Context @see getDevices
		 *
		 * @param	deviceId the id of the Device to use
		 * @param	config the parameters of the Device
		 * @return	true on success, false otherwise */
		static bool setDevice(std::size_t deviceId, const DeviceConfig& config);

		/** Adds the given IDeviceDataListener to the Context
		 *
		 * @param	listener the IDeviceDataListener to notify on new Device
		 *			data
		 * @return	true on success, false otherwise */
		static bool addDeviceDataListener(IDeviceDataListener* listener);

		/** Removes the given IDeviceDataListener from the Context
		 *
		 * @param	listener the IDeviceDataListener that won¡t longer be
		 *			notified on new Device data */
		static void removeDeviceDataListener(IDeviceDataListener* listener);

		/** Destroys the Context instance and releases all the
		 * resources */
		static void stop();
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

#endif		// SEAUDIO_CONTEXT_H
