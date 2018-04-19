#include "CInScrenAndMic.h"
#include <qpixmap.h>
#include <qpainter.h>
#include <qthread.h>
#include <QCursor>
#include <qscreen.h>
#include <qtimer.h>
#include <QApplication>
#include <qaudioinput.h>



extern "C"
{
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

CInScrenAndMic::CInScrenAndMic(QList <CMediaData *>& data):CMediaInput(data)
{
}


CInScrenAndMic::~CInScrenAndMic()
{
}

int CInScrenAndMic::read()
{
	AVSampleFormat inSampleFmt = AV_SAMPLE_FMT_S16;
	AVSampleFormat outSampleFmt = AV_SAMPLE_FMT_FLTP;
	QAudioFormat fmt;
	fmt.setSampleRate(m_sampleRate);
	fmt.setChannelCount(m_channels);
	fmt.setSampleSize(m_sampleSize * 8);
	fmt.setCodec("audio/pcm");
	fmt.setByteOrder(QAudioFormat::LittleEndian);
	fmt.setSampleType(QAudioFormat::UnSignedInt);
	QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
	auto b = info.isFormatSupported(fmt);
	if (!b)
	{
		return -1;
	}
	int audioPts = 0;
	int readSize = 1024 *m_channels*m_sampleSize;
	char *buff = new char[readSize];	
	m_pAInput = new QAudioInput(fmt);
	//开始录制音频
	m_beginTime = av_gettime();
	m_pAIO = m_pAInput->start();
	m_videoThread->start();	
	long long lastTime = -1;		
	while (m_bRead)
	{
		if (m_pAInput->bytesReady() < readSize)
		{
			QThread::msleep(1);
			continue;
		}
		if (m_dataList.count() > 50)
		{
			QThread::msleep(1);
			continue;
		}				
		int size = 0;
		while (size != readSize)
		{
			int len = m_pAIO->read(buff + size, readSize - size);
			if (len < 0)  break;
			size += len;
		}
		if (size != readSize)  continue;
		auto currentTime = av_gettime();
		AVFrame *pPcmFrame = av_frame_alloc();
		pPcmFrame->format = AV_SAMPLE_FMT_FLTP;
		pPcmFrame->sample_rate = m_sampleRate;
		pPcmFrame->channel_layout = av_get_default_channel_layout(m_channels);
		pPcmFrame->channels = m_channels;
		pPcmFrame->nb_samples = 1024;  //一帧音频包含的一通道的采样数量
		auto ret = av_frame_get_buffer(pPcmFrame, 0);
		const uint8_t *inData[AV_NUM_DATA_POINTERS] = {};
		inData[0] = reinterpret_cast<uint8_t*>(buff);
		//转换数据格式
		auto len = swr_convert(m_pASwrCtx, pPcmFrame->data, pPcmFrame->nb_samples, inData, pPcmFrame->nb_samples);
		if (len > 0)
		{
			auto data = new CMediaData(pPcmFrame);
			m_dataList.append(data);
			data->m_nStreamId = 1;
			//pts运算		
			data->m_timeBase = { 1,1000000 };
			data->p.m_pFrame->pts = currentTime - m_beginTime;
			if (data->p.m_pFrame->pts == lastTime)
			{
				data->p.m_pFrame->pts += 2000;
			}
			lastTime = data->p.m_pFrame->pts;
		}
	}
	delete[] buff;
	return 0;
}

int CInScrenAndMic::open()
{
	//设置音频转换上下文
	m_pASwrCtx = swr_alloc_set_opts(m_pASwrCtx, av_get_default_channel_layout(m_channels), AV_SAMPLE_FMT_FLTP, m_sampleRate,
		av_get_default_channel_layout(m_channels), AV_SAMPLE_FMT_S16, m_sampleRate, 0, nullptr);
	auto ret = swr_init(m_pASwrCtx);
	//加载鼠标图标
	m_mouse.load("./mouse.png");	
	//得到窗口的ID和大小
	m_screenID = QApplication::desktop()->winId();
	m_nWidth= QApplication::desktop()->width();
	m_nHeight = QApplication::desktop()->height();
	//设置RGB24转YUV420P上下文
	m_pVSwsCtx = sws_getContext(m_nWidth, m_nHeight, AV_PIX_FMT_RGB24,m_nWidth, m_nHeight, AV_PIX_FMT_YUV420P, SWS_BICUBIC, nullptr, nullptr, nullptr);
	//在线程中开启定时器，每100毫秒截个屏
	m_pScreenTimer = new QTimer();
	m_videoThread = new QThread();
	m_pScreenTimer->setInterval(100);
	m_pScreenTimer->moveToThread(m_videoThread);
	connect(m_videoThread, &QThread::started, m_pScreenTimer, static_cast<void (QTimer::*)()> (&QTimer::start));
	connect(m_pScreenTimer, &QTimer::timeout, this, &CInScrenAndMic::getScreenPic, Qt::DirectConnection);

	return 0;
}


int CInScrenAndMic::getScreenPic()
{
	if (m_dataList.count() > 50)  return 0;	
	//截屏
	QScreen *screen = QGuiApplication::primaryScreen();
	auto screenPixmap = screen->grabWindow(m_screenID);
	//画出鼠标
	QPainter pain(&screenPixmap);
	pain.drawImage(QCursor::pos(), m_mouse);	
	m_pScreenImage = new QImage(screenPixmap.toImage().convertToFormat(QImage::Format_RGB888));	
	auto currentTime = av_gettime();
	int linesize[AV_NUM_DATA_POINTERS] = {};
	linesize[0] = m_nWidth * 3;
	uint8_t*  dataTemp[AV_NUM_DATA_POINTERS] = {};
	auto pFrame = av_frame_alloc();
	pFrame->width = m_nWidth;
	pFrame->height = m_nHeight;
	pFrame->format = AV_PIX_FMT_YUV420P;	
	auto ret = av_frame_get_buffer(pFrame, 32);
	//转换成YUV格式			
	dataTemp[0] = m_pScreenImage->bits();
	int h = sws_scale(m_pVSwsCtx, dataTemp, linesize, 0, m_nHeight, pFrame->data, pFrame->linesize);
	if (h > 0)
	{
		auto data = new CMediaData(pFrame);
		m_dataList.append(data);
		data->m_nStreamId = 0;
		
		data->m_timeBase = { 1,1000000 };		
		data->p.m_pFrame->pts = currentTime - m_beginTime;
		
	}
	
	delete m_pScreenImage;
	m_pScreenImage = nullptr;
	return 0;
}


/////////////////////////////////////////////////////////////////////////
