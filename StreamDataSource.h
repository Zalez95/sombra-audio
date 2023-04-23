#ifndef SEAUDIO_STREAM_DATA_SOURCE_H
#define SEAUDIO_STREAM_DATA_SOURCE_H

#include <memory>
#include "IDataSource.h"
#include "Constants.h"

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

		/** Sets the format of the StreamDataSource
		 *
		 * @param	format the format of the StreamDataSource
		 * @return	a reference to the current StreamDataSource */
		StreamDataSource& setFormat(Format format);

		/** Sets the sample rate of the StreamDataSource
		 *
		 * @param	sampleRate the sample rate of the StreamDataSource
		 * @return	a reference to the current StreamDataSource */
		StreamDataSource& setSampleRate(uint32_t sampleRate);

		/** Sets the channels of the StreamDataSource
		 *
		 * @param	channels a pointer to the channels of the StreamDataSource
		 * @param	channelCount the number of channels
		 * @return	a reference to the current StreamDataSource */
		StreamDataSource& setChannels(
			const Channel* channels, std::size_t channelCount
		);

		void onData(const unsigned char* data, std::size_t samplesSize);
	};

}

#endif		// SEAUDIO_STREAM_DATA_SOURCE_H
