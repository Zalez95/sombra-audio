#ifndef SEAUDIO_I_DATA_SOURCE_H
#define SEAUDIO_I_DATA_SOURCE_H

typedef void ma_data_source;

namespace se::audio {

	/**
	 * Class IDataSource, a data source is an object used to create, delete and
	 * access to the audio data. A IDataSource can be shared between
	 * multiple Sounds to reduce memory consumption.
	 */
	class IDataSource
	{
	public:		// Functions
		/** Creates a new DataSource */
		IDataSource() = default;

		/** Class destructor */
		virtual ~IDataSource() = default;

		/** @return	true if the IDataSource was initialized successfully,
		 *			false otherwise */
		virtual bool good() const = 0;

		/** @return	a pointer to the miniaudio data source */
		virtual ma_data_source* getMADataSource() const = 0;
	};

}

#endif		// SEAUDIO_I_DATA_SOURCE_H
