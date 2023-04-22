#ifndef SEAUDIO_STREAM_DATA_SOURCE_H
#define SEAUDIO_STREAM_DATA_SOURCE_H

#include <memory>
#include "IDataSource.h"

namespace se::audio {

	class AudioEngine;


	/**
	 * Class StreamDataSource, it's a data source whose original audio data is
	 * stored in a file
	 */
	class StreamDataSource : public IDataSource
	{
	private:	// Nested types
		struct MaDataSource;

	private:	// Attributes
		/** A pointer to the data source */
		std::unique_ptr<MaDataSource> mMaDataSource;

	public:		// Functions
		/** Creates a new DataSource */
		StreamDataSource();
		StreamDataSource(const StreamDataSource& other) = delete;
		StreamDataSource(StreamDataSource&& other);

		/** Class destructor */
		~StreamDataSource();

		/** Assignment operator */
		StreamDataSource& operator=(const StreamDataSource& other) = delete;
		StreamDataSource& operator=(StreamDataSource&& other);

		/** @copydoc IDataSource::good() */
		virtual bool good() const;

		/** @copydoc IDataSource::getMADataSource() */
		virtual ma_data_source* getMADataSource() const;

		void onData(const unsigned char* data, std::size_t samplesSize);
	};

}

#endif		// SEAUDIO_STREAM_DATA_SOURCE_H
