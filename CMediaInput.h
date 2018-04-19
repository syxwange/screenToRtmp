#pragma once

#include "CMediaData.h"
#include <qstring.h>
#include <qlist.h>
#include <qmutex.h>
#include <qobject.h>
#include <qthread.h>
extern "C"
{
#include "libavformat/avformat.h"
}

class CInFile;
struct AVCodecContext;

class CMediaInput :public QThread
{	
	Q_OBJECT
public:
	static CMediaInput* getInput(QList <CMediaData *>& data,MediaInType type=MediaInType::File);
	virtual ~CMediaInput();
	//�򿪲���ʼ��ý������
	virtual int open()=0;
	//��ȡ�������ݣ���û�н��룬��avpacket����m_dataList��
	virtual int read()=0;
	//��ȡ�������ݣ������� ��avframe����m_dataList��
	virtual int decode()=0;
	//���������ļ�URL
	void setUrl(QString url) { m_strFileUrl = url; }
	//�߳��˳� 
	void run(){}
	void setExit() { m_bRead = false; }

public:
	AVFormatContext* getAVFmtCtx() { return m_pAvFmtCtx;	}
	AVCodecParameters* getVCodecPara() { return m_pVCodecPara; }
	AVCodecParameters* getACodecPara() { return m_pACodecPara; }
	AVCodecID getVCodecID() { return m_vCodecID; }
	AVCodecID getACodecID() { return m_aCodecID; }	
	int getVIndex() { return m_nVStreamIndex; }
	long long getTime() { return m_lTime; }
	int getAIndex() { return m_nAStreamIndex; }
	void free();
	int getWidth() { return m_nWidth; }
	int getHeight() { return m_nHeight; }

protected:
	CMediaInput(QList <CMediaData *>& data, QObject *parent=nullptr);
	//������Ƶʱ��    ???��֪���ǲ������и�ʽ��֧��
	int getMediaTime();
	//�ҵ���Ƶ����Ƶ��
	int findStream();
	//�ҵ����򿪽�����
	int initDecode();

protected:	
	AVFormatContext* m_pAvFmtCtx = nullptr;
	int m_nVStreamIndex = AVERROR_STREAM_NOT_FOUND;
	int m_nAStreamIndex = AVERROR_STREAM_NOT_FOUND;
	AVStream* m_pVStream = nullptr;
	AVStream* m_pAStream = nullptr;
	AVCodecParameters* m_pVCodecPara = nullptr;
	AVCodecParameters* m_pACodecPara = nullptr;
	int m_nWidth=0;
	int m_nHeight=0;	
	int m_nStreamCount = 0;							//��������	
	long long m_lTime = 0;									//��Ƶʱ��
	long long m_duration = 0;							//����ʱ��
	QString m_strFileUrl;
	AVPacket m_packet = {};
	AVCodecID m_vCodecID = AVCodecID::AV_CODEC_ID_NONE;
	AVCodecID m_aCodecID = AVCodecID::AV_CODEC_ID_NONE;
	AVCodecContext * m_pVCodecCtx=nullptr;
	AVCodecContext * m_pACodecCtx = nullptr;
	QList <CMediaData *>& m_dataList;
	QMutex m_mutex;
	bool m_bRead = true;	
};

class CInFile :public CMediaInput
{	
public:
	CInFile(QList <CMediaData *>& data);
	~CInFile();
	long long getFileTime() { return m_lTime; }
	int open();
	int read();
	int decode();
};
