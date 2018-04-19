#include "CScreenToRtmp.h"
#include "CInScrenAndMic.h"
#include "COutRTMP.h"
#include "CMediaData.h"
#include <qthread.h>


CScreenToRtmp::CScreenToRtmp(QObject *parent):QObject(parent)
{
}


CScreenToRtmp::~CScreenToRtmp()
{
	free();
}

int CScreenToRtmp::run()
{
	//创建屏幕截屏输入线程
	m_pIn = new CInScrenAndMic(m_dataList);
	m_pInThread = new QThread();
	m_pIn->moveToThread(m_pInThread);
	connect(m_pInThread, &QThread::started, m_pIn, &CInScrenAndMic::read);
	auto ret = m_pIn->open();
	if (ret < 0) return -1;
	m_pInThread->start();
	//创建RTMP输出线程
	m_pOut = new COutRTMP(m_dataList);
	m_pOutThread = new QThread();
	m_pOut->moveToThread(m_pOutThread);
	connect(m_pOutThread, &QThread::started, m_pOut, &COutRTMP::write);
	m_pOut->setWidth(m_pIn->getWidth());
	m_pOut->setHeight(m_pIn->getHeight());
	ret = m_pOut->open();
	if (ret<0) return -1;
	m_pOutThread->start();
	return 0;
}

void CScreenToRtmp::free()
{
	if (m_pInThread)
	{
		if (m_pIn)		m_pIn->setExit();
		m_pInThread->exit();
		m_pInThread->deleteLater();
		m_pInThread->wait();
		m_pInThread = nullptr;
	}
	if (m_pOutThread)
	{
		if (m_pOut)   m_pOut->setExit();
		m_pOutThread->exit();
		m_pOutThread->deleteLater();
		m_pOutThread->wait();
		m_pOutThread = nullptr;
	}
	if (m_pIn)
	{
		m_pIn->free();
		m_pIn = nullptr;
	}
	if (m_pOut)
	{
		m_pOut->free();
		m_pOut = nullptr;
	}
	for (auto i : m_dataList)
	{
		delete i;
	}
	m_dataList.clear();
}
