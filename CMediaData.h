#pragma once


extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/time.h"
}

enum MediaInType
{
	File,
	Cap,
	Screen,
	Net
};

enum FilterType
{
	Yuv2Rgb,
	ReSize,
	Brightness,
	Contrast
};

enum MediaDataType
{
	AvFrame,
	AvPacket
};
class AVFrame;
class AVPacket;

class CMediaData
{
public:
	CMediaData(AVFrame *pFrame);
	CMediaData(AVPacket *pPkt);
	~CMediaData();

public:
	int m_nStreamId = 0;
	AVRational m_timeBase = {};
	long long m_pts = 0;
	MediaDataType m_dataType = MediaDataType::AvFrame;
	union 
	{
		AVFrame *m_pFrame=nullptr;
		AVPacket *m_pPkt;
	}p;
};

