#pragma once
#include "CMediaInput.h"
#include <qdesktopwidget.h>

struct SwsContext;
struct SwrContext;
class QAudioInput;
class QIODevice;

class CInScrenAndMic :	public CMediaInput
{
	Q_OBJECT
public:
	CInScrenAndMic(QList <CMediaData *>& data);
	virtual ~CInScrenAndMic();

	int decode() { return 0; }
	int read();
	int open();
	void setBeginTime(long long n) { m_beginTime = n; }

private:
	//截屏
	int getScreenPic();


private:	
	QTimer *m_pScreenTimer=nullptr;									//截屏定时器
	QImage *m_pScreenImage=nullptr;								//截屏图片	
	QThread * m_audioThread = nullptr;								//
	QThread * m_videoThread = nullptr;	
	SwsContext * m_pVSwsCtx = nullptr;								//视频转换上下文
	SwrContext * m_pASwrCtx = nullptr;
	WId m_screenID = 0;	
	QImage m_mouse;															//鼠标图标
	QAudioInput *m_pAInput = nullptr;
	QIODevice *m_pAIO = nullptr;
	int m_sampleRate = 44100;
	int m_channels = 2;
	int m_sampleSize = 2;
	long long m_beginTime = 0;
};

