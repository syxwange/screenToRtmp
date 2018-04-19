#include "screenToRtmpUI.h"
#include "CScreenToRtmp.h"

screenToRtmpUI::screenToRtmpUI(QWidget *parent) : QWidget(parent),m_pScreenToRtmp(new CScreenToRtmp(this))
{
	ui.setupUi(this);
	m_pScreenToRtmp->run();
}
