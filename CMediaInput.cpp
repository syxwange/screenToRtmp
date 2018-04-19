#include "CMediaInput.h"


#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avcodec.lib")

#include <qthread.h>
CMediaInput::CMediaInput(QList <CMediaData *>& data,QObject * parent):QThread(parent), m_dataList(data)
{
	av_register_all();
	avformat_network_init();	
}

CMediaInput* CMediaInput::getInput(QList <CMediaData *>& data,MediaInType type)
{
	switch (type)
	{
	case File:
	{
		static CInFile * pIn = new CInFile(data);
		return pIn;		
	}	
	case Cap:
		break;
	case Screen:
		break;
	case Net:
		break;
	default:
		break;
	}
	return  nullptr;
}

CMediaInput::~CMediaInput()
{
	free();
}

void CMediaInput::free()
{
	if (m_pAvFmtCtx)
	{
		avformat_close_input(&m_pAvFmtCtx);
		m_pAvFmtCtx = nullptr;
	}
	if (m_pVCodecCtx)
	{
		avcodec_free_context(&m_pVCodecCtx);
		m_pVCodecCtx = nullptr;
	}
	if (m_pACodecCtx)
	{
		avcodec_free_context(&m_pACodecCtx);
		m_pACodecCtx = nullptr;
	}
}

int CMediaInput::getMediaTime()
{
	if (m_nVStreamIndex != AVERROR_STREAM_NOT_FOUND)
	{
		if (m_pVStream->duration != AV_NOPTS_VALUE)
		{
			m_duration = m_pVStream->duration;
			m_lTime = m_duration * av_q2d(m_pVStream->time_base);
		}
		else
		{
			m_duration = m_pAvFmtCtx->duration;
			m_lTime = m_pAvFmtCtx->duration *av_q2d(m_pVStream->time_base) / 1000;
		}
		return 0;
	}	
	return -1;
}

int CMediaInput::findStream()
{
	//找到流信息
	int ret;
	if (ret = avformat_find_stream_info(m_pAvFmtCtx, nullptr))
		return ret;
	m_nStreamCount = m_pAvFmtCtx->nb_streams;
	//找到视频流
	m_nVStreamIndex = av_find_best_stream(m_pAvFmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	if (m_nVStreamIndex != AVERROR_STREAM_NOT_FOUND)
	{
		m_pVStream = m_pAvFmtCtx->streams[m_nVStreamIndex];
		m_pVCodecPara = m_pVStream->codecpar;
		m_nWidth = m_pVCodecPara->width;
		m_nHeight = m_pVCodecPara->height;
		m_vCodecID = m_pVCodecPara->codec_id;
	}
	//找到音频流
	m_nAStreamIndex = av_find_best_stream(m_pAvFmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
	if (m_nAStreamIndex != AVERROR_STREAM_NOT_FOUND)
	{
		m_pAStream = m_pAvFmtCtx->streams[m_nAStreamIndex];
		m_pACodecPara = m_pAStream->codecpar;
		m_aCodecID = m_pACodecPara->codec_id;
	}
	return 0;
}

int CMediaInput::initDecode()
{
	if (m_vCodecID != AVCodecID::AV_CODEC_ID_NONE)
	{
		auto pInCodec = avcodec_find_decoder(m_vCodecID);
		m_pVCodecCtx = avcodec_alloc_context3(pInCodec);
		if (!m_pVCodecCtx)	return -1;

		int  ret = avcodec_parameters_to_context(m_pVCodecCtx, m_pVCodecPara);
		if (ret < 0)	 return ret;
		ret = avcodec_open2(m_pVCodecCtx, pInCodec, nullptr);
		if (ret)	 return ret;
	}
	if (m_aCodecID != AVCodecID::AV_CODEC_ID_NONE)
	{
		auto pInCodec = avcodec_find_decoder(m_aCodecID);
		m_pACodecCtx = avcodec_alloc_context3(pInCodec);
		if (!m_pVCodecCtx)	return -1;

		
		int  ret = avcodec_parameters_to_context(m_pACodecCtx, m_pACodecPara);
		if (ret < 0)	 return ret;
		ret = avcodec_open2(m_pACodecCtx, pInCodec, nullptr);
		if (ret)	 return ret;
	}		
	return 0;
}

CInFile::CInFile(QList <CMediaData *>& data):CMediaInput(data)
{

}

CInFile::~CInFile()
{
}

int CInFile::open()
{
	int64_t ret;
	//打开输入文件
	if (ret = avformat_open_input(&m_pAvFmtCtx, m_strFileUrl.toLocal8Bit(), nullptr, nullptr))
		return ret;
	//找到流
	if(ret = findStream())
		return ret;
	//如音视频都没有退出
	if (m_nAStreamIndex == AVERROR_STREAM_NOT_FOUND && m_nVStreamIndex == AVERROR_STREAM_NOT_FOUND)
		return -1;
	//计算视频时长  
	ret = getMediaTime();
	//找到并打开解码器
	ret = initDecode();
	return ret;
}

int CInFile::read()
{
	while (m_bRead)
	{
		m_mutex.lock();
		if (m_dataList.count() >100)
		{
			m_mutex.unlock();
			continue;
		}
		auto tempPkt = av_packet_alloc();
		auto ret = av_read_frame(m_pAvFmtCtx, &m_packet);
		if (ret < 0)  return -1;
		auto tempData = new CMediaData(tempPkt);
		m_mutex.lock();
		m_dataList.append(tempData);		
		m_mutex.unlock();		
	}	
	return 0;
}

int CInFile::decode()
{
	AVPacket pkt = {};
	while (m_bRead)
	{
		m_mutex.lock();
		if (m_dataList.count() > 100)
		{
			m_mutex.unlock();
			av_usleep(1000);
			continue;
		}			
		m_mutex.unlock();
		auto ret = av_read_frame(m_pAvFmtCtx, &pkt);
		if (ret < 0)	
			return ret;	
			
		if (pkt.stream_index == m_nVStreamIndex)
		{
			ret = avcodec_send_packet(m_pVCodecCtx, &pkt);
			if (ret < 0)  continue;
			AVFrame *frame = av_frame_alloc();
			ret = avcodec_receive_frame(m_pVCodecCtx, frame);
			//一定要，否则会造成内存泄露
			av_packet_unref(&pkt);			
			/*if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			{
			av_frame_free(&frame);
			continue;
			}*/
			if (ret < 0)
			{
				av_frame_free(&frame);
				continue;
			}
			auto tempData = new CMediaData(frame);
			tempData->m_timeBase = m_pVStream->time_base;
			tempData->m_nStreamId = m_nVStreamIndex;
			m_mutex.lock();
			m_dataList.append(tempData);
			m_mutex.unlock();
		}			
		if (pkt.stream_index == m_nAStreamIndex)
		{
			ret = avcodec_send_packet(m_pACodecCtx, &pkt);
			if (ret < 0)  continue;
			AVFrame *frame = av_frame_alloc();
			ret = avcodec_receive_frame(m_pACodecCtx, frame);			
			if (ret < 0)
			{
				av_frame_free(&frame);
				continue;
			}
			//一定要，否则会造成内存泄露
			av_packet_unref(&pkt);
			auto tempData = new CMediaData(frame);
			tempData->m_timeBase = m_pAStream->time_base;
			tempData->m_nStreamId = m_nAStreamIndex;
			m_mutex.lock();
			m_dataList.append(tempData);
			m_mutex.unlock();
		}		
	}
	return 0;
}
