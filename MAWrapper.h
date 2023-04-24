#ifndef SEAUDIO_MAWRAPPER_H
#define SEAUDIO_MAWRAPPER_H

#include <miniaudio.h>
#include "Constants.h"

namespace se::audio {

	constexpr ma_format toMAFormat(Format format)
	{
		switch (format) {
			case Format::u8:					return ma_format_u8;
			case Format::s16:					return ma_format_s16;
			case Format::s24:					return ma_format_s24;
			case Format::s32:					return ma_format_s32;
			case Format::f32:					return ma_format_f32;
			default:							return ma_format_unknown;
		}
	}


	constexpr uint32_t bytesPerMAFormat(Format format)
	{
		switch (format) {
			case Format::u8:					return 1u;
			case Format::s16:					return 2u;
			case Format::s24:					return 3u;
			case Format::s32:					return 4u;
			case Format::f32:					return 4u;
			default:							return 0u;
		}
	}


	constexpr ma_channel toMAChannel(Channel channel)
	{
		switch (channel) {
			case Channel::Mono:					return MA_CHANNEL_MONO;
			case Channel::FrontLeft:			return MA_CHANNEL_FRONT_LEFT;
			case Channel::FrontRight:			return MA_CHANNEL_FRONT_RIGHT;
			case Channel::FrontCenter:			return MA_CHANNEL_FRONT_CENTER;
			case Channel::LFE:					return MA_CHANNEL_LFE;
			case Channel::BackLeft:				return MA_CHANNEL_BACK_LEFT;
			case Channel::BackRight:			return MA_CHANNEL_BACK_RIGHT;
			case Channel::FrontLeftCenter:		return MA_CHANNEL_FRONT_LEFT_CENTER;
			case Channel::FrontRightCenter:		return MA_CHANNEL_FRONT_RIGHT_CENTER;
			case Channel::BackCenter:			return MA_CHANNEL_BACK_CENTER;
			case Channel::SideLeft:				return MA_CHANNEL_SIDE_LEFT;
			case Channel::SideRight:			return MA_CHANNEL_SIDE_RIGHT;
			case Channel::TopCenter:			return MA_CHANNEL_TOP_CENTER;
			case Channel::TopFrontLeft:			return MA_CHANNEL_TOP_FRONT_LEFT;
			case Channel::TopFrontCenter:		return MA_CHANNEL_TOP_FRONT_CENTER;
			case Channel::TopFrontRight:		return MA_CHANNEL_TOP_FRONT_RIGHT;
			case Channel::TopBackLeft:			return MA_CHANNEL_TOP_BACK_LEFT;
			case Channel::TopBackCenter:		return MA_CHANNEL_TOP_BACK_CENTER;
			case Channel::TopBackRight:			return MA_CHANNEL_TOP_BACK_RIGHT;
			case Channel::Left:					return MA_CHANNEL_LEFT;
			case Channel::Right:				return MA_CHANNEL_RIGHT;
			default:							return MA_CHANNEL_NONE;
		}
	}

}

#endif		// SEAUDIO_MAWRAPPER_H
