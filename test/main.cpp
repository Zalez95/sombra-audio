#include <chrono>
#include <thread>
#include <iomanip>
#include <iostream>
#include <saudio/Context.h>
#include <saudio/AudioEngine.h>
#include <saudio/Sound.h>
#include <saudio/FileDataSource.h>

static std::ostream& putTime(std::ostream& os)
{
	static const char timeFormat[] = "%Y/%m/%d %H:%M:%S";
	using namespace std::chrono;

	system_clock::time_point now = system_clock::now();
	system_clock::duration tp = now.time_since_epoch();
	tp -= duration_cast<seconds>(tp);

	std::time_t tt = system_clock::to_time_t(now);
	return os
		<< std::put_time(std::localtime(&tt), timeFormat)
		<< '.' << std::setw(3) << std::setfill('0') << tp / milliseconds(1);
}


class AudioLogHandler : public saudio::LogHandler
{
public:		// Functions
	virtual void error(const char* str) override
	{ putTime(std::cout) << " [error]\t" << str << std::endl; };
	virtual void warning(const char* str) override
	{ putTime(std::cout) << " [warn]\t" << str << std::endl; };
	virtual void info(const char* str) override
	{ putTime(std::cout) << " [info]\t" << str << std::endl; };
	virtual void debug(const char* str) override
	{ putTime(std::cout) << " [debug]\t" << str << std::endl; };
};


int main()
{
	AudioLogHandler sAudioLogHandler;

	// Create the audio context
	saudio::Context::ContextConfig audioContextConfig;
	audioContextConfig.logHandler = &sAudioLogHandler;
	if (!saudio::Context::start(audioContextConfig)) {
		std::cerr << "Failed to start the audio Context" << std::endl;
		return -1;
	}

	std::size_t defaultId = 0;
	for (const auto& device : saudio::Context::getDevices()) {
		if (device.isDefault) {
			defaultId = device.id;
		}
	}

	saudio::Context::DeviceConfig audioDeviceConfig;
	if (!saudio::Context::setDevice(defaultId, audioDeviceConfig)) {
		std::cerr << "Failed to set the audio Device" << std::endl;
		saudio::Context::stop();
		return -1;
	}

	// Create the AudioEngine and Sound
	auto audioEngine = std::make_unique<saudio::AudioEngine>();
	if (!audioEngine->good()) {
		std::cerr << "Failed to Create the AudioEngine" << std::endl;
		audioEngine = nullptr;
		saudio::Context::stop();
		return -1;
	}

	auto dataSource = std::make_unique<saudio::FileDataSource>(*audioEngine, "file_example_MP3_1MG.mp3");
	if (!dataSource->good()) {
		std::cerr << "Failed to Create the AudioEngine" << std::endl;
		dataSource = nullptr;
		audioEngine = nullptr;
		saudio::Context::stop();
		return -1;
	}

	auto sound = std::make_unique<saudio::Sound>(audioEngine.get());
	if (!sound->good()) {
		std::cerr << "Failed to Create the Sound" << std::endl;
		sound = nullptr;
		dataSource = nullptr;
		audioEngine = nullptr;
		saudio::Context::stop();
		return -1;
	}
	sound->setLooping(false)
		.bind(dataSource.get());

	// Run
	std::cout << "==== START ====" << std::endl;
	bool stop = false;
	while (!stop) {
		int option = -1;
		std::cout << "Options:\n\t1 -> stop test program\n"
			<< "\t2 -> list devices\n\t3 -> change device\n"
			<< "\t4 -> play\n\t5 -> pause\n\t6 -> stop\n\t7 -> change loop"
			<< std::endl;
		std::cin >> option;

		switch (option) {
			case 1: {
				stop = true;
			} break;
			case 2: {
				std::cout << "Devices:" << std::endl;
				for (const auto& device : saudio::Context::getDevices()) {
					std::cout << "\t" << device.id << " " << device.name << " " << device.isDefault << std::endl;
				}
			} break;
			case 3: {
				std::cin >> option;
				saudio::Context::DeviceConfig audioDeviceConfig;
				if (!saudio::Context::setDevice(option, audioDeviceConfig)) {
					std::cerr << "Failed to set the audio Device" << std::endl;
					stop = true;
				}
			} break;
			case 4: {
				sound->play();
			} break;
			case 5: {
				sound->pause();
			} break;
			case 6: {
				sound->stop();
			} break;
			case 7: {
				sound->setLooping(!sound->isLooping());
			} break;
		}
	}
	std::cout << "==== END ====" << std::endl;

	// Release resources
	sound = nullptr;
	dataSource = nullptr;
	audioEngine = nullptr;
	saudio::Context::stop();

	return 0;
}
