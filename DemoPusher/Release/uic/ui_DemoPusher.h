/********************************************************************************
** Form generated from reading UI file 'DemoPusher.ui'
**
** Created by: Qt User Interface Compiler version 5.12.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DEMOPUSHER_H
#define UI_DEMOPUSHER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DemoPusherClass
{
public:
    QFrame *m_videoShow;
    QPushButton *LiveButton_;
    QPushButton *PreviewButton_;
    QLineEdit *streamId_;
    QLineEdit *postUrl;
    QLabel *label;
    QLabel *label_2;

    void setupUi(QWidget *DemoPusherClass)
    {
        if (DemoPusherClass->objectName().isEmpty())
            DemoPusherClass->setObjectName(QString::fromUtf8("DemoPusherClass"));
        DemoPusherClass->resize(961, 400);
        m_videoShow = new QFrame(DemoPusherClass);
        m_videoShow->setObjectName(QString::fromUtf8("m_videoShow"));
        m_videoShow->setGeometry(QRect(0, 0, 601, 401));
        m_videoShow->setFrameShape(QFrame::StyledPanel);
        m_videoShow->setFrameShadow(QFrame::Raised);
        LiveButton_ = new QPushButton(DemoPusherClass);
        LiveButton_->setObjectName(QString::fromUtf8("LiveButton_"));
        LiveButton_->setGeometry(QRect(610, 20, 75, 23));
        PreviewButton_ = new QPushButton(DemoPusherClass);
        PreviewButton_->setObjectName(QString::fromUtf8("PreviewButton_"));
        PreviewButton_->setGeometry(QRect(610, 60, 75, 23));
        streamId_ = new QLineEdit(DemoPusherClass);
        streamId_->setObjectName(QString::fromUtf8("streamId_"));
        streamId_->setGeometry(QRect(680, 100, 71, 20));
        postUrl = new QLineEdit(DemoPusherClass);
        postUrl->setObjectName(QString::fromUtf8("postUrl"));
        postUrl->setGeometry(QRect(680, 150, 281, 21));
        label = new QLabel(DemoPusherClass);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(610, 100, 54, 16));
        label_2 = new QLabel(DemoPusherClass);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(610, 150, 54, 16));

        retranslateUi(DemoPusherClass);

        QMetaObject::connectSlotsByName(DemoPusherClass);
    } // setupUi

    void retranslateUi(QWidget *DemoPusherClass)
    {
        DemoPusherClass->setWindowTitle(QApplication::translate("DemoPusherClass", "DemoPusher", nullptr));
        LiveButton_->setText(QApplication::translate("DemoPusherClass", "\345\274\200\345\247\213\347\233\264\346\222\255", nullptr));
        PreviewButton_->setText(QApplication::translate("DemoPusherClass", "\345\205\263\351\227\255\351\242\204\350\247\210", nullptr));
        label->setText(QApplication::translate("DemoPusherClass", "streamid", nullptr));
        label_2->setText(QApplication::translate("DemoPusherClass", "postUrl", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DemoPusherClass: public Ui_DemoPusherClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DEMOPUSHER_H
