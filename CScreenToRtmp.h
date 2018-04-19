#pragma once


#include <QObject>
class CInScrenAndMic;
class QThread;
class COutRTMP;
class CMediaData;

class CScreenToRtmp:public QObject
{
public:
	CScreenToRtmp(QObject *parent=nullptr);
	~CScreenToRtmp();
	int run();
	void free();

private:
	CInScrenAndMic * m_pIn = nullptr;
	QThread* m_pInThread = nullptr;
	QList<CMediaData *> m_dataList;
	COutRTMP * m_pOut = nullptr;
	QThread* m_pOutThread = nullptr;
};

