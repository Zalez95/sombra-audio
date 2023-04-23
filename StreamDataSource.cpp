#include <iostream>
#include <algorithm>
#include <miniaudio.h>
#include "StreamDataSource.h"
#include "Context.h"
#include "MAWrapper.h"

namespace se::audio {

	struct StreamDataSource::MaDataSource
	{
		ma_data_source_base base;
		ma_format format;
		uint32_t sampleRate;
		std::vector<ma_channel> channels;

		MaDataSource();
		~MaDataSource();

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


	StreamDataSource::MaDataSource::MaDataSource()
	{
		ma_data_source_vtable vTable = {
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
		// TODO: Read data here. Output in the same format returned by my_data_source_get_data_format().
		return MA_ERROR;
	}


	ma_result StreamDataSource::MaDataSource::onSeek(ma_data_source*, ma_uint64 frameIndex)
	{
		return MA_NOT_IMPLEMENTED;
	}


	ma_result StreamDataSource::MaDataSource::onGetDataFormat(
		ma_data_source* pDataSource, ma_format* pFormat, ma_uint32* pChannels, ma_uint32* pSampleRate,
		ma_channel* pChannelMap, size_t channelMapCap
	) {
		if (!pDataSource) {
			return MA_ERROR;
		}

		auto pThis = static_cast<MaDataSource*>(pDataSource);
		std::size_t channelCount = std::min(pThis->channels.size(), channelMapCap);

		*pFormat = pThis->format;
		*pSampleRate = pThis->sampleRate;
		*pChannels = static_cast<ma_uint32>(channelCount);
		for (std::size_t i = 0; i < channelCount; ++i) {
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


	StreamDataSource::StreamDataSource() : IDataSource()
	{
		mMaDataSource = std::make_unique<MaDataSource>();
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
		mMaDataSource->format = toMAFormat(format);
		return *this;
	}


	StreamDataSource& StreamDataSource::setSampleRate(uint32_t sampleRate)
	{
		mMaDataSource->sampleRate = sampleRate;
		return *this;
	}


	StreamDataSource& StreamDataSource::setChannels(const Channel* channels, std::size_t channelCount)
	{
		mMaDataSource->channels.reserve(channelCount);
		for (std::size_t i = 0; i < channelCount; ++i) {
			mMaDataSource->channels.push_back( toMAChannel(channels[i]) );
		}

		return *this;
	}


	void StreamDataSource::onData(const unsigned char* data, std::size_t samplesSize)
	{

	}

}
