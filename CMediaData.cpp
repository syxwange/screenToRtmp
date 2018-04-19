#include "CMediaData.h"

extern "C"
{
#include "libavformat/avformat.h"
}


CMediaData::CMediaData(AVFrame *pFrame)
{
	m_dataType = MediaDataType::AvFrame;
	p.m_pFrame = pFrame;
}

CMediaData::CMediaData(AVPacket * pPkt)
{
	m_dataType = MediaDataType::AvPacket;
	p.m_pPkt = pPkt;
}


CMediaData::~CMediaData()
{
	if (m_dataType == MediaDataType::AvFrame)
	{
		if (p.m_pFrame)
		{
			av_frame_free(&p.m_pFrame);
			p.m_pFrame = nullptr;
		}			
	}
	else if(m_dataType == MediaDataType::AvPacket)
	{
		if (p.m_pPkt)
		{
			av_packet_free(&p.m_pPkt);
			p.m_pPkt = nullptr;
		}
	}
}
