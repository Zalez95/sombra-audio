#ifndef SAUDIO_FILE_DATA_SOURCE_H
#define SAUDIO_FILE_DATA_SOURCE_H

#include <memory>
#include "IDataSource.h"

struct ma_sound;

namespace saudio {

	class AudioEngine;


	/**
	 * Class FileDataSource, it's a data source whose original audio data is
	 * stored in a file
	 */
	class FileDataSource : public IDataSource
	{
	private:	// Attributes
		/** The data source owner (sound) */
		std::unique_ptr<ma_sound> mDataSourceOwner;

	public:		// Functions
		/** Creates a new DataSource */
		FileDataSource(AudioEngine& engine, const char* path);
		FileDataSource(const FileDataSource& other) = delete;
		FileDataSource(FileDataSource&& other);

		/** Class destructor */
		~FileDataSource();

		/** Assignment operator */
		FileDataSource& operator=(const FileDataSource& other) = delete;
		FileDataSource& operator=(FileDataSource&& other);

		/** @copydoc IDataSource::good() */
		virtual bool good() const;

		/** @copydoc IDataSource::getMADataSource() */
		virtual ma_data_source* getMADataSource() const;
	};

}

#endif		// SAUDIO_FILE_DATA_SOURCE_H
