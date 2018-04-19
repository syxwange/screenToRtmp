#include "COutRTMP.h"


extern "C"
{
#include <libavdevice/avdevice.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avdevice.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"swresample")


COutRTMP::COutRTMP(QList <CMediaData *>& data) :CMediaOutput(data)
{	
	avcodec_register_all();
}


COutRTMP::~COutRTMP()
{
}

int COutRTMP::open()
{
	m_strOutURL = "rtmp://shaoyang.work/live";
	auto ret = avformat_alloc_output_context2(&m_pOutFmt, NULL, "flv", "rtmp://shaoyang.work/live");
	//output encoder initialize  
	ret = initVideo();
	if (ret < 0) return -1;
	ret = initAudio();
	if (ret < 0) return -1;	
	ret = avio_open(&m_pOutFmt->pb, "rtmp://shaoyang.work/live", AVIO_FLAG_WRITE);
	if (ret < 0) return -1;
	ret = avformat_write_header(m_pOutFmt, NULL);
	if (ret < 0) return -1;
	return 0;
}

int COutRTMP::write()
{
	AVPacket pkt = {};
	while (m_bWrite)
	{		
		if (m_dataList.count()<1)
		{			
			av_usleep(1000);
			continue;
		}
		CMediaData * tempData = m_dataList.first();
		m_dataList.removeFirst();		
		if (tempData->m_nStreamId == 0)
		{
			auto ret = avcodec_send_frame(m_pVCodecCtx, tempData->p.m_pFrame);
			if (ret < 0)
			{
				delete tempData;
				tempData = nullptr;
				continue;
			}
			av_packet_unref(&pkt);
			ret = avcodec_receive_packet(m_pVCodecCtx, &pkt);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			{
				delete tempData;
				tempData = nullptr;
				continue;
			}
			else if (ret < 0) return -1;			
			av_packet_rescale_ts(&pkt, tempData->m_timeBase, m_pOutFmt->streams[0]->time_base);
			delete tempData;
			tempData = nullptr;
			ret = av_interleaved_write_frame(m_pOutFmt, &pkt);			
			continue;
		}
		if (tempData->m_nStreamId == 1)
		{
			auto ret = avcodec_send_frame(m_pACodecCtx, tempData->p.m_pFrame);
			if (ret < 0)
			{
				delete tempData;
				tempData = nullptr;
				continue;
			}
			av_packet_unref(&pkt);
			ret = avcodec_receive_packet(m_pACodecCtx, &pkt);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			{
				delete tempData;
				tempData = nullptr;
				continue;
			}			
			else if (ret < 0) return -1;
			pkt.stream_index = 1;
			av_packet_rescale_ts(&pkt, tempData->m_timeBase, m_pOutFmt->streams[1]->time_base);
			delete tempData;
			tempData = nullptr;
			ret = av_interleaved_write_frame(m_pOutFmt, &pkt);
			
		}			
	}
	return 0;
}

int COutRTMP::initAudio()
{
	AVCodec *audioEncodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
	if (!audioEncodec) return -1;
	m_pACodecCtx = avcodec_alloc_context3(audioEncodec);
	if (!m_pACodecCtx) return -1;
	m_pACodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	m_pACodecCtx->thread_count = 8;
	m_pACodecCtx->bit_rate = 40000;
	m_pACodecCtx->sample_rate = 44100;
	m_pACodecCtx->sample_fmt = AV_SAMPLE_FMT_FLTP;
	m_pACodecCtx->channels = 2;
	m_pACodecCtx->channel_layout = av_get_default_channel_layout(2);
	m_pACodecCtx->time_base = { 1,1000000 };
	auto ret = avcodec_open2(m_pACodecCtx, audioEncodec, nullptr);
	if (ret) return -1;
	AVStream *audioStream = avformat_new_stream(m_pOutFmt, audioEncodec);
	if (!audioStream) return -1;	
	//从编码器复制参数
	ret = avcodec_parameters_from_context(audioStream->codecpar, m_pACodecCtx);
	if (ret < 0) return -1;
	audioStream->codecpar->codec_tag = 0;
	return 0;
}

int COutRTMP::initVideo()
{
	auto pOutCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!pOutCodec) return -1;
	m_pVCodecCtx = avcodec_alloc_context3(pOutCodec);
	if (!m_pVCodecCtx) return -1;
	m_pVCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	m_pVCodecCtx->width = m_nWidth;
	m_pVCodecCtx->height = m_nHeight;
	m_pVCodecCtx->time_base = { 1,1000000 };
	m_pVCodecCtx->bit_rate = 400000;
	m_pVCodecCtx->gop_size = 10;
	if (m_pOutFmt->oformat->flags & AVFMT_GLOBALHEADER)
		m_pVCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;
	m_pVCodecCtx->qmin = 2;
	m_pVCodecCtx->qmax = 10;
	m_pVCodecCtx->max_b_frames = 0;
	// Set H264 preset and tune这个可以减少网络延时，不知道质量下降多少  
	AVDictionary *param = nullptr;
	av_dict_set(&param, "preset", "faster", 0);
	av_dict_set(&param, "tune", "zerolatency", 0);
	
	auto ret = avcodec_open2(m_pVCodecCtx, pOutCodec, &param);
	if (ret) return -1;
	auto vOutStream = avformat_new_stream(m_pOutFmt, pOutCodec);
	if (!vOutStream)		return -1;
	ret = avcodec_parameters_from_context(vOutStream->codecpar, m_pVCodecCtx);
	if (ret < 0) return -1;	
	vOutStream->codecpar->codec_tag = 0;
	return 0;
}
