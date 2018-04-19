#pragma once
#include "CMediaOutput.h"

class AVFormatContext;
class AVCodecContext;
class COutRTMP :	public CMediaOutput
{
public:
	COutRTMP(QList <CMediaData *>& data);
	~COutRTMP();

	//�򿪲���ʼ������Ƶת�������ģ�����Ƶתh264,��ƵתAAC
	virtual int open();
	//����Ƶ�͵������
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

