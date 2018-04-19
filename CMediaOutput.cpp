#include "CMediaOutput.h"
#include <qaudioformat.h>
#include <qaudiooutput.h>
#include <qbytearray.h>
#include <qmessagebox.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"swresample")


CMediaOutput::CMediaOutput(QList<CMediaData*>& data, QObject *parent )	: QObject(parent),m_dataList(data)
{

}

CMediaOutput::~CMediaOutput()
{
}

void CMediaOutput::free()
{
	
}

void CMediaOutput::filter()
{
	for (auto t : m_vecFilterType)
	{
		switch (t)
		{
		case Yuv2Rgb:
			break;
		case ReSize:
			break;
		case Brightness:
			break;
		case Contrast:
			break;
		default:
			break;
		}
	}
}

COutScreen::COutScreen(QList<CMediaData*>& data, QObject *parent):CMediaOutput(data,parent)
{
	
}

COutScreen::~COutScreen()
{
}

int COutScreen::open()
{

	m_pVSwsCtx = sws_getContext(m_nWidth, m_nHeight, AV_PIX_FMT_YUV420P,
		m_nWidth, m_nHeight, AV_PIX_FMT_RGB24, SWS_BICUBIC, nullptr, nullptr, nullptr);
	//������Ƶ��ʽת��������
	m_pASwrCtx = swr_alloc_set_opts(m_pASwrCtx, av_get_default_channel_layout(m_nChannels), AV_SAMPLE_FMT_S16, m_nSampleRate,
		av_get_default_channel_layout(m_nChannels), m_sampleFmt, m_nSampleRate, 0, nullptr);
	//һ��Ҫinit
	swr_init(m_pASwrCtx);
	auto ret = initAudio();
	return 0;
}

int COutScreen::write()
{
	auto pFrame = av_frame_alloc();
	pFrame->width =m_nWidth;
	pFrame->height = m_nHeight;
	pFrame->format = AV_PIX_FMT_RGB24;
	pFrame->pts = 0;
	auto ret = av_frame_get_buffer(pFrame, 32);
	m_starTime = av_gettime();
	while (m_bWrite)
	{
		m_mutex.lock();
		if(m_dataList.count()<1)
		{
			m_mutex.unlock();
			av_usleep(1000);
			continue;
		}
		CMediaData * tempData = m_dataList.first();
		m_dataList.removeFirst();
		m_mutex.unlock();
		if (tempData->m_nStreamId == 0)
		{			
			int h = sws_scale(m_pVSwsCtx, tempData->p.m_pFrame->data, tempData->p.m_pFrame->linesize, 0,
				m_nHeight, pFrame->data, pFrame->linesize);			
			filter();			
			emit sendPic(pFrame);
			delete tempData;
			tempData = nullptr;
		}
		else if (tempData->m_nStreamId == 1)
		{
			uint8_t *outData[AV_NUM_DATA_POINTERS] = {};			
			auto num = tempData->p.m_pFrame->nb_samples*m_nChannels * 2;
			outData[0] = new uint8_t[num];
		   auto len = swr_convert(m_pASwrCtx,outData, tempData->p.m_pFrame->nb_samples, (const uint8_t **)tempData->p.m_pFrame->data, tempData->p.m_pFrame->nb_samples);
			if (len < 0)
			{
				delete tempData;
				continue;
			}		
			//����Ƶ����ͬ���������ٶ���Ƶ����ͬ���������Ƶ������PTSͬ���ᷢ����Ƶ��������
			//�����ö��̻߳������⣬û����
			AVRational tb = tempData->m_timeBase;
			//�Ѿ���ȥ��ʱ��
			long long now = av_gettime() - m_starTime;
			long long pts = static_cast<long long>(tempData->p.m_pFrame->pts * (1000 * 1000 * av_q2d(tb)));
			if (pts > now)
			{
				av_usleep(static_cast<unsigned int>(pts - now));
			}		
			//д�뻺��
			m_pIODeviceOut->write((char *)outData[0], num);
			delete tempData;
			tempData = nullptr;
			delete [] outData[0];		
		}		
	}	
	av_frame_free(&pFrame);
	return 0;
}

void COutScreen::free()
{
	
	if (m_pVSwsCtx)
	{
		sws_freeContext(m_pVSwsCtx);
		m_pVSwsCtx = nullptr;
	}
}

int COutScreen::initAudio()
{	
	QAudioFormat fmt;
	fmt.setSampleRate(m_nSampleRate);
	fmt.setChannelCount(m_nChannels);
	fmt.setSampleSize(16);	
	fmt.setCodec("audio/pcm");
	fmt.setByteOrder(QAudioFormat::LittleEndian);
	fmt.setSampleType(QAudioFormat::UnSignedInt);
	m_pAudioOutput = new QAudioOutput(fmt);
	m_pIODeviceOut = m_pAudioOutput->start();
	return 0;
}
