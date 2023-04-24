#include <mutex>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <miniaudio.h>
#include "StreamDataSource.h"
#include "Context.h"
#include "MAWrapper.h"

namespace se::audio {

	class StreamDataSource::CircularBuffer
	{
	private:
		std::vector<unsigned char> mData;
		std::size_t mFirstByte = 0, mLastByte = 0;

	public:
		CircularBuffer(std::size_t bufferSize = 0) : mData(bufferSize) {};

		bool empty() const
		{
			return (mFirstByte == mLastByte);
		};

		std::size_t read(unsigned char* data, std::size_t size)
		{
			std::size_t bytesRead = 0;

			// Copy the last part of the circular buffer
			if (mLastByte < mFirstByte) {
				std::size_t bytesToCopy = std::min(mData.size() - mFirstByte, size);
				std::memcpy(data, &mData[mFirstByte], bytesToCopy);

				mFirstByte = (mFirstByte + bytesToCopy) % mData.size();
				bytesRead += bytesToCopy;
			}

			// Copy the first part of the circular buffer
			std::size_t bytesToCopy = std::min(mLastByte + 1 - mFirstByte, size - bytesRead);
			std::memcpy(data + bytesRead, &mData[mFirstByte], bytesToCopy);

			mFirstByte = (mFirstByte + bytesToCopy) % mData.size();
			bytesRead += bytesToCopy;

			return bytesRead;
		};

		void write(const unsigned char* data, std::size_t size)
		{
			if (!data || (size == 0)) { return; }

			// We can't copy more than the last mData.size() bytes
			std::size_t rSize = std::min(size, mData.size());
			const unsigned char* rData = data + (size - rSize);

			std::size_t bytesWritten = 0;
			while (bytesWritten < rSize) {
				std::size_t nextByte = (mLastByte + 1) % mData.size();
				std::size_t bytesToCopy = std::min(mData.size() - nextByte, rSize - bytesWritten);
				std::memcpy(&mData[nextByte], rData + bytesWritten, bytesToCopy);

				std::size_t previousLastByte = mLastByte;
				mLastByte = (mLastByte + bytesToCopy) % mData.size();
				if ((mFirstByte <= mLastByte)
					&& ((mFirstByte >= previousLastByte) || (mLastByte <= previousLastByte))
				) {
					mFirstByte = (mLastByte + 1) % mData.size();
				}

				bytesWritten += bytesToCopy;
			}
		};
	};


	struct StreamDataSource::MaDataSource
	{
		ma_data_source_base base;
		ma_data_source_vtable vTable;

		std::mutex mutex;

		ma_format format;
		uint32_t sampleRate;
		uint32_t numChannels;
		std::vector<ma_channel> channels;

		std::size_t numSamples = 0;
		std::size_t sampleSize = 0;
		CircularBuffer buffer;

		MaDataSource(std::size_t numSamples);
		MaDataSource(const MaDataSource& other) = delete;
		MaDataSource(MaDataSource&& other) = delete;

		~MaDataSource();

		MaDataSource& operator=(const MaDataSource& other) = delete;
		MaDataSource& operator=(MaDataSource&& other) = delete;

		static ma_result onRead(
			ma_data_source* pDataSource, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead
		);
		static ma_result onSeek(
			ma_data_source* pDataSource, ma_uint64 frameIndex
		);
		static ma_result onGetDataFormat(
			ma_data_source* pDataSource, ma_format* pFormat, ma_uint32* pChannels, ma_uint32* pSampleRate,
			ma_channel* pChannelMap, size_t channelMapCap
		);
		static ma_result onGetCursor(
			ma_data_source* pDataSource, ma_uint64* pCursor
		);
		static ma_result onGetLength(
			ma_data_source* pDataSource, ma_uint64* pLength
		);
	};


	StreamDataSource::MaDataSource::MaDataSource(std::size_t numSamples) :
		numSamples(numSamples)
	{
		vTable = {
			&onRead, &onSeek, &onGetDataFormat, &onGetCursor, &onGetLength,
			nullptr, 0
		};

		ma_data_source_config baseConfig;
		baseConfig = ma_data_source_config_init();
		baseConfig.vtable = &vTable;

		ma_result result = ma_data_source_init(&baseConfig, &base);
		if (result != MA_SUCCESS) {
			Context::getLogHandler()->error("Failed to initialize the DataSource");
			return;
		}

		std::cout << "Created the base data source " << &base << std::endl;
	}


	StreamDataSource::MaDataSource::~MaDataSource()
	{
		ma_data_source_uninit(&base);
	}


	ma_result StreamDataSource::MaDataSource::onRead(
		ma_data_source* pDataSource, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead
	) {
		if (!pDataSource) { return MA_ERROR; }

		auto pThis = static_cast<MaDataSource*>(pDataSource);
		std::unique_lock lock(pThis->mutex);

		std::size_t bytesToRead = frameCount * pThis->numChannels * pThis->sampleSize;

		if (pThis->buffer.empty()) {
			// pFramesRead must not be zero (it marks the end of the data source)
			memset(pFramesOut, 0, bytesToRead);
			*pFramesRead = frameCount;
		}
		else {
			std::size_t bytesRead = pThis->buffer.read(reinterpret_cast<unsigned char*>(pFramesOut), bytesToRead);
			*pFramesRead = static_cast<ma_uint64>(bytesRead / (pThis->numChannels * pThis->sampleSize));
		}

		return MA_SUCCESS;
	}


	ma_result StreamDataSource::MaDataSource::onSeek(ma_data_source*, ma_uint64)
	{
		return MA_NOT_IMPLEMENTED;
	}


	ma_result StreamDataSource::MaDataSource::onGetDataFormat(
		ma_data_source* pDataSource, ma_format* pFormat, ma_uint32* pChannels, ma_uint32* pSampleRate,
		ma_channel* pChannelMap, size_t channelMapCap
	) {
		if (!pDataSource) { return MA_ERROR; }

		auto pThis = static_cast<MaDataSource*>(pDataSource);
		std::unique_lock lock(pThis->mutex);

		*pFormat = pThis->format;
		*pSampleRate = pThis->sampleRate;
		*pChannels = static_cast<ma_uint32>(pThis->channels.size());

		std::size_t channelMapSize = std::min(pThis->channels.size(), channelMapCap);
		for (std::size_t i = 0; i < channelMapSize; ++i) {
			pChannelMap[i] = pThis->channels[i];
		}

		return MA_SUCCESS;
	}


	ma_result StreamDataSource::MaDataSource::onGetCursor(ma_data_source*, ma_uint64* pCursor)
	{
		*pCursor = 0;
		return MA_NOT_IMPLEMENTED;
	}


	ma_result StreamDataSource::MaDataSource::onGetLength(ma_data_source*, ma_uint64* pLength)
	{
		*pLength = 0;
		return MA_NOT_IMPLEMENTED;
	}


	StreamDataSource::StreamDataSource(std::size_t bufferedSamples) : IDataSource()
	{
		if (bufferedSamples < 2) {
			Context::getLogHandler()->error("The number of bufferedSamples must be at least 2");
			return;
		}

		mMaDataSource = std::make_unique<MaDataSource>(bufferedSamples);
	}


	StreamDataSource::StreamDataSource(StreamDataSource&& other) :
		mMaDataSource(std::move(other.mMaDataSource)) {}


	StreamDataSource::~StreamDataSource()
	{
		mMaDataSource = nullptr;
	}


	StreamDataSource& StreamDataSource::operator=(StreamDataSource&& other)
	{
		mMaDataSource = std::move(other.mMaDataSource);
		return *this;
	}


	bool StreamDataSource::good() const
	{
		return (mMaDataSource != nullptr);
	}


	ma_data_source* StreamDataSource::getMADataSource() const
	{
		return static_cast<ma_data_source*>(mMaDataSource.get());
	}


	StreamDataSource& StreamDataSource::setFormat(Format format)
	{
		std::unique_lock lock(mMaDataSource->mutex);

		mMaDataSource->format = toMAFormat(format);
		mMaDataSource->sampleSize = bytesPerMAFormat(format);

		std::size_t bufferSize = mMaDataSource->numSamples * mMaDataSource->numChannels * mMaDataSource->sampleSize;
		mMaDataSource->buffer = CircularBuffer(bufferSize);

		return *this;
	}


	StreamDataSource& StreamDataSource::setSampleRate(uint32_t sampleRate)
	{
		std::unique_lock lock(mMaDataSource->mutex);
		mMaDataSource->sampleRate = sampleRate;
		return *this;
	}


	StreamDataSource& StreamDataSource::setNumChannels(int numChannels)
	{
		std::unique_lock lock(mMaDataSource->mutex);

		mMaDataSource->numChannels = static_cast<uint32_t>(numChannels);

		std::size_t bufferSize = mMaDataSource->numSamples * mMaDataSource->numChannels * mMaDataSource->sampleSize;
		mMaDataSource->buffer = CircularBuffer(bufferSize);

		return *this;
	}


	StreamDataSource& StreamDataSource::setChannels(const Channel* channels, std::size_t channelCount)
	{
		std::unique_lock lock(mMaDataSource->mutex);

		mMaDataSource->channels.reserve(channelCount);
		for (std::size_t i = 0; i < channelCount; ++i) {
			mMaDataSource->channels.push_back( toMAChannel(channels[i]) );
		}

		return *this;
	}


	StreamDataSource& StreamDataSource::onNewSamples(const unsigned char* data, std::size_t numSamples)
	{
		std::unique_lock lock(mMaDataSource->mutex);

		std::size_t byteToWrite = numSamples * mMaDataSource->numChannels * mMaDataSource->sampleSize;
		mMaDataSource->buffer.write(reinterpret_cast<const unsigned char*>(data), byteToWrite);

		return *this;
	}

}
