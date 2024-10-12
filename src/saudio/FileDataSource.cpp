#include <iostream>
#include <miniaudio.h>
#include "saudio/FileDataSource.h"
#include "saudio/AudioEngine.h"

namespace saudio {

	FileDataSource::FileDataSource(AudioEngine& engine, const char* path) : IDataSource()
	{
		mDataSourceOwner = std::make_unique<ma_sound>();
		ma_result res = ma_sound_init_from_file(engine.getMAEngine(), path, MA_SOUND_FLAG_DECODE, nullptr, nullptr, mDataSourceOwner.get());
		if (res != MA_SUCCESS) {
			Context::getLogHandler()->error("Failed to create the DataSourceOwner");
			mDataSourceOwner = nullptr;
		}
		else {
			std::cout << "Created DataSourceOwner " << mDataSourceOwner.get() << std::endl;
		}
	}


	FileDataSource::FileDataSource(FileDataSource&& other) :
		mDataSourceOwner(std::move(other.mDataSourceOwner)) {}


	FileDataSource::~FileDataSource()
	{
		if (mDataSourceOwner) {
			ma_sound_uninit(mDataSourceOwner.get());
			std::cout << "Deleted DataSourceOwner " << mDataSourceOwner.get() << std::endl;
			mDataSourceOwner = nullptr;
		}
	}


	FileDataSource& FileDataSource::operator=(FileDataSource&& other)
	{
		if (mDataSourceOwner) {
			ma_sound_uninit(mDataSourceOwner.get());
			std::cout << "Deleted DataSourceOwner " << mDataSourceOwner.get() << std::endl;
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
