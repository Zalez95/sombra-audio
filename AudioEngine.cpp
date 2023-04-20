#include <string>
#include <istream>
#include <algorithm>
#include <miniaudio.h>
#include <stdext/PackedVector.h>
#include "Context.h"
#include "AudioEngine.h"

namespace se::audio {

	static constexpr ma_format kDecodeFormat = ma_format_f32;
	static constexpr uint32_t kDecodeChannels = 0;				// native channel count
	static constexpr uint32_t kDecodeSampleRate = 48000;


	struct AudioEngine::MaVFS
	{
		struct StreamData
		{
			std::string path;
			std::size_t size;
			std::unique_ptr<std::istream> stream;
		};

		ma_vfs_callbacks callbacks;
		std::unique_ptr<IVFS> vfs;
		stdext::PackedVector<StreamData> streams;

		MaVFS(std::unique_ptr<IVFS> vfs);

		static ma_result onOpen(ma_vfs* pVFS, const char* pFilePath, ma_uint32 openMode, ma_vfs_file* pFile);
		static ma_result onOpenW(ma_vfs* pVFS, const wchar_t* pFilePath, ma_uint32 openMode, ma_vfs_file* pFile);
		static ma_result onClose(ma_vfs* pVFS, ma_vfs_file file);
		static ma_result onRead(ma_vfs* pVFS, ma_vfs_file file, void* pDst, size_t sizeInBytes, size_t* pBytesRead);
		static ma_result onWrite(ma_vfs* pVFS, ma_vfs_file file, const void* pSrc, size_t sizeInBytes, size_t* pBytesWritten);
		static ma_result onSeek(ma_vfs* pVFS, ma_vfs_file file, ma_int64 offset, ma_seek_origin origin);
		static ma_result onTell(ma_vfs* pVFS, ma_vfs_file file, ma_int64* pCursor);
		static ma_result onInfo(ma_vfs* pVFS, ma_vfs_file file, ma_file_info* pInfo);

		static bool toIndex(ma_vfs_file file, std::size_t& index);
		static bool toVFSFile(std::size_t index, ma_vfs_file& file);
	};


	AudioEngine::MaVFS::MaVFS(std::unique_ptr<IVFS> vfs) : vfs(std::move(vfs))
	{
		callbacks = {
			&onOpen, &onOpenW, &onClose, &onRead, &onWrite,
			&onSeek, &onTell, &onInfo
		};
	}


	ma_result AudioEngine::MaVFS::onOpen(
		ma_vfs* pVFS, const char* pFilePath, ma_uint32, ma_vfs_file* pFile
	) {
		auto pThis = static_cast<MaVFS*>(pVFS);
		auto it = std::find_if(pThis->streams.begin(), pThis->streams.end(), [&](const StreamData& sd) {
			return (sd.path == pFilePath);
		});
		if (it == pThis->streams.end()) {
			StreamData sd;
			sd.path = pFilePath;
			if (!pThis->vfs->getSize(pFilePath, sd.size)) {
				return MA_INVALID_ARGS;
			}
			if (!pThis->vfs->openR(pFilePath, sd.stream)) {
				return MA_INVALID_ARGS;
			}
			if (!sd.stream->good()) {
				return MA_ERROR;
			}

			it = pThis->streams.emplace(std::move(sd));
		}

		if (!toVFSFile(it.getIndex(), *pFile)) {
			return MA_INVALID_FILE;
		}

		return MA_SUCCESS;
	}


	ma_result AudioEngine::MaVFS::onOpenW(ma_vfs*, const wchar_t*, ma_uint32, ma_vfs_file*)
	{
		return MA_NOT_IMPLEMENTED;
	}


	ma_result AudioEngine::MaVFS::onClose(ma_vfs* pVFS, ma_vfs_file file)
	{
		auto pThis = static_cast<MaVFS*>(pVFS);
		std::size_t iSD;
		if (!toIndex(file, iSD)) {
			return MA_INVALID_FILE;
		}

		pThis->streams.erase( pThis->streams.begin().setIndex(iSD) );
		return MA_SUCCESS;
	}


	ma_result AudioEngine::MaVFS::onRead(
		ma_vfs* pVFS, ma_vfs_file file, void* pDst, size_t sizeInBytes, size_t* pBytesRead
	) {
		auto pThis = static_cast<MaVFS*>(pVFS);
		std::size_t iSD;
		if (!toIndex(file, iSD)) {
			return MA_INVALID_FILE;
		}

		pThis->streams[iSD].stream->read(reinterpret_cast<char*>(pDst), sizeInBytes);
		std::size_t result = pThis->streams[iSD].stream->gcount();

		if (pBytesRead) {
			*pBytesRead = result;
		}

		if (result != sizeInBytes) {
			if ((result == 0) && pThis->streams[iSD].stream->eof()) {
				return MA_AT_END;
			}
			else {
				return MA_ERROR;
			}
		}

		return MA_SUCCESS;
	}


	ma_result AudioEngine::MaVFS::onWrite(ma_vfs*, ma_vfs_file, const void*, size_t, size_t*)
	{
		return MA_NOT_IMPLEMENTED;
	}


	ma_result AudioEngine::MaVFS::onSeek(
		ma_vfs* pVFS, ma_vfs_file file, ma_int64 offset, ma_seek_origin origin
	) {
		auto pThis = static_cast<MaVFS*>(pVFS);
		std::size_t iSD;
		if (!toIndex(file, iSD)) {
			return MA_INVALID_FILE;
		}

		if (pThis->streams[iSD].stream->bad()) {
			return MA_ERROR;
		}

		std::ios_base::seekdir ori =
			(origin == ma_seek_origin_start)? std::ios_base::beg :
			(origin == ma_seek_origin_end)? std::ios_base::end :
			std::ios_base::cur;

		pThis->streams[iSD].stream->clear();
		pThis->streams[iSD].stream->seekg(offset, ori);		// FIXME: Without seek this doesn't work
		if (pThis->streams[iSD].stream->fail()) {
			return MA_ERROR;
		}

		return MA_SUCCESS;
	}


	ma_result AudioEngine::MaVFS::onTell(ma_vfs* pVFS, ma_vfs_file file, ma_int64* pCursor)
	{
		auto pThis = static_cast<MaVFS*>(pVFS);
		std::size_t iSD;
		if (!toIndex(file, iSD)) {
			return MA_INVALID_FILE;
		}

		*pCursor = pThis->streams[iSD].stream->tellg();
		return MA_SUCCESS;
	}


	ma_result AudioEngine::MaVFS::onInfo(ma_vfs* pVFS, ma_vfs_file file, ma_file_info* pInfo)
	{
		auto pThis = static_cast<MaVFS*>(pVFS);
		std::size_t iSD;
		if (!toIndex(file, iSD)) {
			return MA_INVALID_FILE;
		}

		pInfo->sizeInBytes = pThis->streams[iSD].size;
		return MA_SUCCESS;
	}


	bool AudioEngine::MaVFS::toVFSFile(std::size_t index, ma_vfs_file& file)
	{
		// Add always 1 so the file value is never NULL
		file = reinterpret_cast<ma_vfs_file>(index + 1);
		return true;
	}


	bool AudioEngine::MaVFS::toIndex(ma_vfs_file file, std::size_t& index)
	{
		if (!file) {
			return false;
		}

		index = reinterpret_cast<std::size_t>(file) - 1;
		return true;
	}


	AudioEngine::AudioEngine(std::unique_ptr<IVFS> vfs)
	{
		ma_resource_manager_config resourceManagerConfig = ma_resource_manager_config_init();
		resourceManagerConfig.pLog = static_cast<ma_log*>(Context::getMALog());
		resourceManagerConfig.decodedFormat = kDecodeFormat;
		resourceManagerConfig.decodedChannels = kDecodeChannels;
		resourceManagerConfig.decodedSampleRate = kDecodeSampleRate;
		if (vfs) {
			mVFS = std::make_unique<MaVFS>(std::move(vfs));
			resourceManagerConfig.pVFS = static_cast<ma_vfs*>(mVFS.get());
		}

		mResourceManager = std::make_unique<ma_resource_manager>();
		ma_result result = ma_resource_manager_init(&resourceManagerConfig, mResourceManager.get());
		if (result != MA_SUCCESS) {
			Context::getLogHandler()->error("ResourceManager creation error");
			mResourceManager = nullptr;
			return;
		}

		ma_engine_config engineConfig = ma_engine_config_init();
		engineConfig.pResourceManager = mResourceManager.get();
		engineConfig.pContext = Context::getMAContext();
		engineConfig.pDevice = Context::getMADevice();
		engineConfig.listenerCount = 1;

		mEngine = std::make_unique<ma_engine>();
		result = ma_engine_init(&engineConfig, mEngine.get());
		if (result != MA_SUCCESS) {
			Context::getLogHandler()->error("Engine creation error");
			mEngine = nullptr;
			return;
		}

		if (!Context::addDeviceDataListener(this)) {
			Context::getLogHandler()->error("Failed to add as a Device listener");
			return;
		}
	}


	AudioEngine::~AudioEngine()
	{
		Context::removeDeviceDataListener(this);

		if (mEngine) {
			ma_engine_uninit(mEngine.get());
			mEngine = nullptr;
		}

		if (mResourceManager) {
			ma_resource_manager_uninit(mResourceManager.get());
			mResourceManager = nullptr;
		}
	}


	bool AudioEngine::good()
	{
		return mEngine && mResourceManager;
	}


	ma_engine* AudioEngine::getMAEngine() const
	{
		return mEngine.get();
	}


	glm::vec3 AudioEngine::getListenerPosition() const
	{
		ma_vec3f pos = ma_engine_listener_get_position(mEngine.get(), kListenerIndex);
		return { pos.x, pos.y, pos.z };
	}


	AudioEngine& AudioEngine::setListenerPosition(const glm::vec3& position)
	{
		ma_engine_listener_set_position(mEngine.get(), kListenerIndex, position.x, position.y, position.z);
		return *this;
	}


	void AudioEngine::getListenerOrientation(glm::vec3& forwardVector, glm::vec3& upVector) const
	{
		ma_vec3f dirFor = ma_engine_listener_get_direction(mEngine.get(), kListenerIndex);
		forwardVector = { dirFor.x, dirFor.y, dirFor.z };

		ma_vec3f dirUp = ma_engine_listener_get_world_up(mEngine.get(), kListenerIndex);
		upVector = { dirUp.x, dirUp.y, dirUp.z };
	}


	AudioEngine& AudioEngine::setListenerOrientation(const glm::vec3& forwardVector, const glm::vec3& upVector)
	{
		ma_engine_listener_set_direction(mEngine.get(), kListenerIndex, forwardVector.x, forwardVector.y, forwardVector.z);
		ma_engine_listener_set_world_up(mEngine.get(), kListenerIndex, upVector.x, upVector.y, upVector.z);
		return *this;
	}


	void AudioEngine::getListenerCone(float& innerAngle, float& outerAngle, float& outerGain) const
	{
		ma_engine_listener_get_cone(mEngine.get(), kListenerIndex, &innerAngle, &outerAngle, &outerGain);
	}


	AudioEngine& AudioEngine::setListenerCone(float innerAngle, float outerAngle, float outerGain)
	{
		ma_engine_listener_set_cone(mEngine.get(), kListenerIndex, innerAngle, outerAngle, outerGain);
		return *this;
	}


	glm::vec3 AudioEngine::getListenerVelocity() const
	{
		ma_vec3f vel = ma_engine_listener_get_velocity(mEngine.get(), kListenerIndex);
		return { vel.x, vel.y, vel.z };
	}


	AudioEngine& AudioEngine::setListenerVelocity(const glm::vec3& velocity)
	{
		ma_engine_listener_set_velocity(mEngine.get(), kListenerIndex, velocity.x, velocity.y, velocity.z);
		return *this;
	}


	void AudioEngine::onDeviceData(void* output, const void*, unsigned int frameCount)
	{
		ma_engine_read_pcm_frames(mEngine.get(), output, frameCount, nullptr);
	}

}
