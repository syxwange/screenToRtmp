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
	//����
	int getScreenPic();


private:	
	QTimer *m_pScreenTimer=nullptr;									//������ʱ��
	QImage *m_pScreenImage=nullptr;								//����ͼƬ	
	QThread * m_audioThread = nullptr;								//
	QThread * m_videoThread = nullptr;	
	SwsContext * m_pVSwsCtx = nullptr;								//��Ƶת��������
	SwrContext * m_pASwrCtx = nullptr;
	WId m_screenID = 0;	
	QImage m_mouse;															//���ͼ��
	QAudioInput *m_pAInput = nullptr;
	QIODevice *m_pAIO = nullptr;
	int m_sampleRate = 44100;
	int m_channels = 2;
	int m_sampleSize = 2;
	long long m_beginTime = 0;
};

