#ifndef SAUDIO_STREAM_DATA_SOURCE_H
#define SAUDIO_STREAM_DATA_SOURCE_H

#include <memory>
#include "IDataSource.h"
#include "Constants.h"

namespace saudio {

	class AudioEngine;


	/**
	 * Class StreamDataSource, it's a data source whose original audio data is
	 * stored in a file
	 */
	class StreamDataSource : public IDataSource
	{
	private:	// Nested types
		struct CircularBuffer;
		struct MaDataSource;

	private:	// Attributes
		/** A pointer to the data source */
		std::unique_ptr<MaDataSource> mMaDataSource;

	public:		// Functions
		/** Creates a new DataSource
		 *
		 * @param	bufferedSamples the number of buffered samples in the
		 *			StreamDataSource. It must be at least 2 */
		StreamDataSource(std::size_t bufferedSamples);
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

		/** Sets the number of channels of the StreamDataSource
		 *
		 * @param	numChannels the number of channels
		 * @return	a reference to the current StreamDataSource */
		StreamDataSource& setNumChannels(int numChannels);

		/** Sets the channels of the StreamDataSource
		 *
		 * @param	channels a pointer to the channels of the StreamDataSource
		 * @param	channelCount the number of channels
		 * @return	a reference to the current StreamDataSource */
		StreamDataSource& setChannels(
			const Channel* channels, std::size_t channelCount
		);

		/** Adds the given data to the StreamDataSource so it can be played
		 *
		 * @param	data the new data of the StreamDataSource
		 * @param	numSamples the number of samples in data
		 * @return	a reference to the current StreamDataSource */
		StreamDataSource& onNewSamples(
			const unsigned char* data, std::size_t numSamples
		);
	};

}

#endif		// SAUDIO_STREAM_DATA_SOURCE_H
