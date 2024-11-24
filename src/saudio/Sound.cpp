#include <miniaudio.h>
#include "saudio/Sound.h"
#include "saudio/IDataSource.h"
#include "saudio/AudioEngine.h"
#include "LogWrapper.h"

namespace saudio {

	Sound::Sound(AudioEngine* audioEngine)
	{
		if (audioEngine) {
			initInternal(audioEngine->getMAEngine());
		}
		else {
			SAUDIO_DEBUG_LOG << "No engine provided, no Sound initialized";
		}
	}


	Sound::Sound(const Sound& other)
	{
		*this = other;
	}


	Sound::Sound(Sound&& other) : mSound(std::move(other.mSound)) {}


	Sound::~Sound()
	{
		if (mSound) {
			stop();
			uninitInternal();
		}
	}


	Sound& Sound::operator=(const Sound& other)
	{
		if (mSound) {
			stop();
			uninitInternal();
		}

		ma_sound* sound = other.mSound.get();
		ma_engine* engine = ma_sound_get_engine(other.mSound.get());
		copyInternal(sound, engine);
		return *this;
	}


	Sound& Sound::operator=(Sound&& other)
	{
		if (mSound) {
			stop();
			uninitInternal();
		}

		mSound = std::move(other.mSound);

		return *this;
	}


	Sound Sound::copy(const Sound& other, AudioEngine* audioEngine)
	{
		ma_sound* sound = other.mSound.get();
		ma_engine* engine = audioEngine->getMAEngine();

		Sound ret;
		ret.copyInternal(sound, engine);
		return ret;
	}


	bool Sound::good() const
	{
		return (mSound != nullptr);
	}


	bool Sound::isPlaying() const
	{
		return ma_sound_is_playing(mSound.get());
	}


	bool Sound::hasSpacialization() const
	{
		return ma_sound_is_spatialization_enabled(mSound.get());
	}


	Sound& Sound::setSpacialization(bool value)
	{
		ma_sound_set_spatialization_enabled(mSound.get(), value);
		return *this;
	}


	glm::vec3 Sound::getPosition() const
	{
		ma_vec3f pos = ma_sound_get_position(mSound.get());
		return { pos.x, pos.y, pos.z };
	}


	Sound& Sound::setPosition(const glm::vec3& position)
	{
		ma_sound_set_position(mSound.get(), position.x, position.y, position.z);
		return *this;
	}


	glm::vec3 Sound::getOrientation() const
	{
		ma_vec3f dir = ma_sound_get_direction(mSound.get());
		return { dir.x, dir.y, dir.z };
	}


	Sound& Sound::setOrientation(const glm::vec3& forwardVector)
	{
		ma_sound_set_direction(mSound.get(), forwardVector.x, forwardVector.y, forwardVector.z);
		return *this;
	}


	void Sound::getSoundCone(float& innerAngle, float& outerAngle, float& outerGain) const
	{
		ma_sound_get_cone(mSound.get(), &innerAngle, &outerAngle, &outerGain);
	}


	Sound& Sound::setSoundCone(float innerAngle, float outerAngle, float outerGain)
	{
		ma_sound_set_cone(mSound.get(), innerAngle, outerAngle, outerGain);
		return *this;
	}


	glm::vec3 Sound::getVelocity() const
	{
		ma_vec3f vel = ma_sound_get_velocity(mSound.get());
		return { vel.x, vel.y, vel.z };
	}


	Sound& Sound::setVelocity(const glm::vec3& velocity)
	{
		ma_sound_set_velocity(mSound.get(), velocity.x, velocity.y, velocity.z);
		return *this;
	}


	float Sound::getVolume() const
	{
		return ma_sound_get_volume(mSound.get());
	}


	Sound& Sound::setVolume(float volume)
	{
		ma_sound_set_volume(mSound.get(), volume);
		return *this;
	}


	float Sound::getPitch() const
	{
		return ma_sound_get_pitch(mSound.get());
	}


	Sound& Sound::setPitch(float pitch)
	{
		ma_sound_set_pitch(mSound.get(), pitch);
		return *this;
	}


	bool Sound::isLooping() const
	{
		return ma_sound_is_looping(mSound.get());
	}


	Sound& Sound::setLooping(bool looping)
	{
		ma_sound_set_looping(mSound.get(), looping);
		return *this;
	}


	Sound& Sound::bind(IDataSource* source)
	{
		ma_engine* engine = ma_sound_get_engine(mSound.get());
		ma_data_source* dataSource = source->getMADataSource();

		Sound other;
		other.mSound = std::make_unique<ma_sound>();
		ma_result res = ma_sound_init_from_data_source(engine, dataSource, 0, nullptr, other.mSound.get());
		if (res != MA_SUCCESS) {
			SAUDIO_ERROR_LOG << "Failed to create the sound";
			return *this;
		}

		SAUDIO_DEBUG_LOG << "Created Sound " << other.mSound.get() << " with DataSource " << dataSource;

		other.setPosition( getPosition() );
		other.setOrientation( getOrientation() );
		float innerAngle, outerAngle, outerGain;
		getSoundCone(innerAngle, outerAngle, outerGain);
		other.setSoundCone(innerAngle, outerAngle, outerGain);
		other.setVelocity( getVelocity() );
		other.setVolume( getVolume() );
		other.setPitch( getPitch() );
		other.setLooping( isLooping() );

		*this = std::move(other);

		return *this;
	}


	Sound& Sound::unbind()
	{
		ma_engine* engine = ma_sound_get_engine(mSound.get());

		stop();
		uninitInternal();
		initInternal(engine);

		return *this;
	}


	void Sound::play() const
	{
		ma_sound_start(mSound.get());
	}


	void Sound::pause() const
	{
		ma_sound_stop(mSound.get());
	}


	void Sound::setToPCMFrame(unsigned int frame) const
	{
		ma_sound_seek_to_pcm_frame(mSound.get(), frame);
	}


	void Sound::stop() const
	{
		pause();
		setToPCMFrame(0);
	}

// Private functions
	bool Sound::initInternal(ma_engine* engine)
	{
		mSound = std::make_unique<ma_sound>();

		ma_sound_config soundConfig = {};
		soundConfig = ma_sound_config_init();

		ma_result res = ma_sound_init_ex(engine, &soundConfig, mSound.get());
		if (res != MA_SUCCESS) {
			SAUDIO_ERROR_LOG << "Failed to create the sound";
			mSound = nullptr;
			return false;
		}
		ma_sound_stop(mSound.get());

		SAUDIO_DEBUG_LOG << "Created Sound " << mSound.get();
		return true;
	}


	bool Sound::copyInternal(ma_sound* other, ma_engine* engine)
	{
		mSound = std::make_unique<ma_sound>();
		ma_result res = ma_sound_init_copy(engine, other, 0, nullptr, mSound.get());
		if (res != MA_SUCCESS) {
			SAUDIO_ERROR_LOG << "Failed to create the sound";
			mSound = nullptr;
			return false;
		}

		SAUDIO_DEBUG_LOG << "Created Sound " << mSound.get();
		return true;
	}


	void Sound::uninitInternal()
	{
		ma_sound_uninit(mSound.get());
		SAUDIO_DEBUG_LOG << "Deleted Sound " << mSound.get();
		mSound = nullptr;
	}

}
