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
	//打开并初始化媒体输入
	virtual int open()=0;
	//读取输入数据，但没有解码，把avpacket放入m_dataList中
	virtual int read()=0;
	//读取输入数据，并解码 把avframe放入m_dataList中
	virtual int decode()=0;
	//设置输入文件URL
	void setUrl(QString url) { m_strFileUrl = url; }
	//线程退出 
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
	//计算视频时长    ???不知道是不是所有格式都支持
	int getMediaTime();
	//找到视频或音频流
	int findStream();
	//找到并打开解码器
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
	int m_nStreamCount = 0;							//流的数量	
	long long m_lTime = 0;									//视频时长
	long long m_duration = 0;							//持续时间
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
