#pragma once

#include <QtWidgets/QWidget>
#include "ui_screenToRtmpUI.h"
class CScreenToRtmp;

class screenToRtmpUI : public QWidget
{
	Q_OBJECT

public:
	screenToRtmpUI(QWidget *parent = Q_NULLPTR);

private:
	Ui::screenToRtmpUIClass ui;
	CScreenToRtmp * m_pScreenToRtmp = nullptr;
};
