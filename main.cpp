#include "screenToRtmpUI.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	screenToRtmpUI w;
	w.show();
	return a.exec();
}
