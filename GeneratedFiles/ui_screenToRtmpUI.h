/********************************************************************************
** Form generated from reading UI file 'screenToRtmpUI.ui'
**
** Created by: Qt User Interface Compiler version 5.9.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SCREENTORTMPUI_H
#define UI_SCREENTORTMPUI_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_screenToRtmpUIClass
{
public:

    void setupUi(QWidget *screenToRtmpUIClass)
    {
        if (screenToRtmpUIClass->objectName().isEmpty())
            screenToRtmpUIClass->setObjectName(QStringLiteral("screenToRtmpUIClass"));
        screenToRtmpUIClass->resize(600, 400);

        retranslateUi(screenToRtmpUIClass);

        QMetaObject::connectSlotsByName(screenToRtmpUIClass);
    } // setupUi

    void retranslateUi(QWidget *screenToRtmpUIClass)
    {
        screenToRtmpUIClass->setWindowTitle(QApplication::translate("screenToRtmpUIClass", "screenToRtmpUI", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class screenToRtmpUIClass: public Ui_screenToRtmpUIClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SCREENTORTMPUI_H
