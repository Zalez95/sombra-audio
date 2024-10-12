#ifndef SAUDIO_CONSTANTS_H
#define SAUDIO_CONSTANTS_H

namespace saudio {

	enum class Format : unsigned char
	{
		Unknown = 0,
		u8,
		s16,
		s24,
		s32,
		f32
	};


	enum class Channel : unsigned char
	{
		None = 0,
		Mono,
		FrontLeft,
		FrontRight,
		FrontCenter,
		LFE,
		BackLeft,
		BackRight,
		FrontLeftCenter,
		FrontRightCenter,
		BackCenter,
		SideLeft,
		SideRight,
		TopCenter,
		TopFrontLeft,
		TopFrontCenter,
		TopFrontRight,
		TopBackLeft,
		TopBackCenter,
		TopBackRight,
		Left,
		Right,
		NumChannels
	};

}

#endif		// SAUDIO_CONSTANTS_H
