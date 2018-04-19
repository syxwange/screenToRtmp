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
	//�򿪲���ʼ������Ƶת�������ģ�����ƵתYUV-RGB,��ƵFLT--S16
	virtual int open() = 0;
	//д����Ƶ
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
	//�򿪲���ʼ������Ƶת�������ģ�����ƵתYUV-RGB,��ƵFLT--S16
	virtual int open() ;
	//����Ƶ�͵���Ļ����Ƶ�͵�����
	virtual int write() ;	
	void free();
signals:
	void sendPic(AVFrame * pic);
private:
	//ͨ��QT��QAudioOutput������������ʼ��QAudioFormat����
	int initAudio();
	//��Ƶת��������
	SwsContext * m_pVSwsCtx = nullptr;
	long long m_starTime = 0;
	QAudioOutput* m_pAudioOutput = nullptr;
	QIODevice * m_pIODeviceOut = nullptr;
	//��Ƶת��������
	SwrContext *m_pASwrCtx = nullptr;
};

