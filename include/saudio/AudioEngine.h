#ifndef SAUDIO_AUDIO_ENGINE_H
#define SAUDIO_AUDIO_ENGINE_H

#include <glm/glm.hpp>
#include "Device.h"

struct ma_resource_manager;
struct ma_engine;

namespace saudio {

	/**
	 * Class AudioEngine, It's the class used to prepare the audio devices for
	 * playing sounds, and to set the properties of the Listener of the Sounds.
	 * The properties of this Listener can be used to control from where we are
	 * going to be listening the Sounds in this 3D audio scene.
	 */
	class AudioEngine : public Device::IDeviceDataListener
	{
	public:		// Nested Types
		friend class Sound;
		friend class DataSource;

		/** It's used for openning audio files from a virtual file system */
		class IVFS
		{
		public:		// Functions
			virtual ~IVFS() = default;
			virtual bool getSize(
				const char* path, std::size_t& size
			) = 0;
			virtual bool openR(
				const char* path, std::unique_ptr<std::istream>& stream
			) = 0;
		};

		/** Struct Config, holds all the parameters needed for
		 * initializing the AudioEngine */
		struct Config
		{
			/** A pointer to the VFS to use with the Engine for loading
			 * audio files (the OS one by default) */
			IVFS* vfs = nullptr;

			Format decodeFormat = Format::f32;
			uint32_t decodeChannels = 0;
			uint32_t decodeSampleRate = 48000;
		};
	private:
		struct MaVFS;

	private:	// Attributes
		/** The id of the single listener in @see mEngine */
		static constexpr unsigned int kListenerIndex = 0;

		/** The device used by the engine */
		Device& mDevice;

		/** A pointer to the ResourceManager */
		std::unique_ptr<ma_resource_manager> mResourceManager;

		/** A pointer to the Engine */
		std::unique_ptr<ma_engine> mEngine;

		/** The virtual file system to use (the actual OS FS by default) */
		std::unique_ptr<MaVFS> mVFS;

	public:		// Functions
		/** Creates a new AudioEngine
		 *
		 * @param	device the device that will be used by the AudioEngine
		 * @param	config the parameters of the new AudioEngine */
		AudioEngine(Device& device, const Config& config);

		/** Class destructor */
		~AudioEngine();

		/** @return	true if the AudioEngine was created successfully,
		 *			false otherwise */
		bool good();

		/** @return	the Device used by the Engine */
		Device& getDevice() const;

		/** @return	a pointer to the miniaudio engine of the AudioEngine */
		ma_engine* getMAEngine() const;

		/** @return	the 3D position of the current Listener */
		glm::vec3 getListenerPosition() const;

		/** Sets the 3D position of the current Listener
		 *
		 * @param	position the new position of the Listener
		 * @return	a reference to the current AudioEngine object */
		AudioEngine& setListenerPosition(const glm::vec3& position);

		/** Returns the 3D orientation of the current Listener
		 *
		 * @param	forwardVector a reference to the vector that points to
		 *			the new forward direction of the current Listener
		 * @param	upVector a reference to the vector that points to the new
		 *			up direction of the current Listener */
		void getListenerOrientation(
			glm::vec3& forwardVector, glm::vec3& upVector
		) const;

		/** Sets the 3D orientation of the current Listener
		 *
		 * @param	forwardVector the vector that points to the new forward
		 *			direction of the current Listener
		 * @param	upVector the vector that points to the new up direction of
		 *			the current Listener
		 * @return	a reference to the current AudioEngine object */
		AudioEngine& setListenerOrientation(
			const glm::vec3& forwardVector, const glm::vec3& upVector
		);

		/** Returns the directional attenuation of the Listener
		 *
		 * @param	innerAngle a reference to the float where the angle in
		 *			radians where a sound inside that cone will have no
		 *			attenuation will be stored
		 * @param	outerAngle a reference to the float where the angle in
		 *			radians where a sound outside that cone will have an
		 *			attenuation set to @see outerGain will be stored
		 * @param	outerGain a reference to the float where the gain used
		 *			for the @see outerAngle cone will be stored */
		void getListenerCone(
			float& innerAngle, float& outerAngle, float& outerGain
		) const;

		/** Sets the directional attenuation of the Listener
		 *
		 * @param	innerAngle the angle in radians where a sound inside that
		 *			cone will have no attenuation
		 * @param	outerAngle the angle in radians where a sound outside that
		 *			cone will have an attenuation set to @see outerGain. The
		 *			sounds inside the outerAngle cone and outside
		 *			@see innerAngle cone will be interpolated between 1 and
		 *			@see outerGain
		 * @param	outerGain the gain used for the @see outerAngle cone
		 * @return	a reference to the current AudioEngine object */
		AudioEngine& setListenerCone(
			float innerAngle, float outerAngle, float outerGain
		);

		/** @return	the 3D velocity of the current Listener */
		glm::vec3 getListenerVelocity() const;

		/** Sets the 3D velocity of the current Listener
		 *
		 * @param	velocity the new velocity of the Listener
		 * @return	a reference to the current AudioEngine object */
		AudioEngine& setListenerVelocity(const glm::vec3& velocity);

		/** @copydoc saudio::AudioEngine::onDeviceData() */
		virtual void onDeviceData(
			void* output, const void* input, unsigned int frameCount
		) override;
	};

}

#endif		// SAUDIO_AUDIO_ENGINE_H
