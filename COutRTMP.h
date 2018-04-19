#pragma once
#include "CMediaOutput.h"

class AVFormatContext;
class AVCodecContext;
class COutRTMP :	public CMediaOutput
{
public:
	COutRTMP(QList <CMediaData *>& data);
	~COutRTMP();

	//打开并初始化音视频转化上下文，如视频转h264,音频转AAC
	virtual int open();
	//把视频送到网络端
	virtual int write();

private:
	int initAudio();
	int initVideo();

private:
	AVFormatContext * m_pOutFmt = nullptr;
	QString m_strOutURL;	
	AVCodecContext * m_pVCodecCtx = nullptr;
	AVCodecContext * m_pACodecCtx = nullptr;
};

