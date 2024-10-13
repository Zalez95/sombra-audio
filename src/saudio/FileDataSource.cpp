#include <miniaudio.h>
#include "saudio/FileDataSource.h"
#include "saudio/AudioEngine.h"
#include "LogWrapper.h"

namespace saudio {

	FileDataSource::FileDataSource(AudioEngine& engine, const char* path) : IDataSource()
	{
		mDataSourceOwner = std::make_unique<ma_sound>();
		ma_result res = ma_sound_init_from_file(engine.getMAEngine(), path, MA_SOUND_FLAG_DECODE, nullptr, nullptr, mDataSourceOwner.get());
		if (res != MA_SUCCESS) {
			SAUDIO_ERROR_LOG << "Failed to create the DataSourceOwner";
			mDataSourceOwner = nullptr;
		}
		else {
			SAUDIO_DEBUG_LOG << "Created DataSourceOwner " << mDataSourceOwner.get();
		}
	}


	FileDataSource::FileDataSource(FileDataSource&& other) :
		mDataSourceOwner(std::move(other.mDataSourceOwner)) {}


	FileDataSource::~FileDataSource()
	{
		if (mDataSourceOwner) {
			ma_sound_uninit(mDataSourceOwner.get());
			SAUDIO_DEBUG_LOG << "Deleted DataSourceOwner " << mDataSourceOwner.get();
			mDataSourceOwner = nullptr;
		}
	}


	FileDataSource& FileDataSource::operator=(FileDataSource&& other)
	{
		if (mDataSourceOwner) {
			ma_sound_uninit(mDataSourceOwner.get());
			SAUDIO_DEBUG_LOG << "Deleted DataSourceOwner " << mDataSourceOwner.get();
			mDataSourceOwner = nullptr;
		}

		mDataSourceOwner = std::move(other.mDataSourceOwner);

		return *this;
	}


	bool FileDataSource::good() const
	{
		return (mDataSourceOwner != nullptr);
	}


	ma_data_source* FileDataSource::getMADataSource() const
	{
		return ma_sound_get_data_source(mDataSourceOwner.get());
	}

}
