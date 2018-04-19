#pragma once

#include <QObject>
#include "CMediaData.h"
#include <qvector.h>
#include <qmutex.h>
class SwsContext;
class QAudioOutput;
class QIODevice;
class SwrContext;

class CMediaOutput : public QObject
{
	Q_OBJECT

public:
	CMediaOutput(QList<CMediaData*>& data, QObject *parent=nullptr);
	~CMediaOutput();
	//打开并初始化音视频转化上下文，如视频转YUV-RGB,音频FLT--S16
	virtual int open() = 0;
	//写音视频
	virtual int write() = 0;	
	virtual void free();
	void setWidth(int width) { m_nWidth = width; }
	void setHeight(int height) { m_nHeight = height; }
	void setReWidth(int width) { m_nResizeWidth = width; }
	void setReHeight(int height) { m_nResizeHeight = height; }
	void setAudioCodecPara(AVCodecParameters* para) { m_nSampleRate = para->sample_rate; m_nChannels = para->channels; m_sampleFmt =static_cast<AVSampleFormat>( para->format); }
	void setExit() { m_bWrite = false; }
protected:
	void filter();

protected:
	QList<CMediaData*>& m_dataList;
	QVector<FilterType> m_vecFilterType;
	bool m_bWrite = true;
	int m_nWidth = 0;
	int m_nHeight = 0;
	int m_nResizeWidth = 0;
	int m_nResizeHeight = 0;
	QMutex m_mutex;	
	int m_nSampleRate = 0;
	int m_nChannels = 0;
	AVSampleFormat m_sampleFmt = AV_SAMPLE_FMT_NONE;
};

class COutScreen:public CMediaOutput
{
	Q_OBJECT
public:
	COutScreen(QList<CMediaData*>& data, QObject *parent = nullptr);
	~COutScreen();
	//打开并初始化音视频转化上下文，如视频转YUV-RGB,音频FLT--S16
	virtual int open() ;
	//把视频送到屏幕，音频送到声卡
	virtual int write() ;	
	void free();
signals:
	void sendPic(AVFrame * pic);
private:
	//通过QT的QAudioOutput播放声音，初始化QAudioFormat参数
	int initAudio();
	//视频转换上下文
	SwsContext * m_pVSwsCtx = nullptr;
	long long m_starTime = 0;
	QAudioOutput* m_pAudioOutput = nullptr;
	QIODevice * m_pIODeviceOut = nullptr;
	//音频转换上下文
	SwrContext *m_pASwrCtx = nullptr;
};

