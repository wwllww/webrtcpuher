/********************************************************************************
** Form generated from reading UI file 'devicesetting.ui'
**
** Created by: Qt User Interface Compiler version 5.12.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DEVICESETTING_H
#define UI_DEVICESETTING_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSlider>

QT_BEGIN_NAMESPACE

class Ui_DeviceSetting
{
public:
    QDialogButtonBox *buttonBox;
    QLabel *label;
    QCheckBox *videoSource;
    QSlider *micVolume;
    QLabel *label_2;
    QComboBox *cameralIndex;
    QLabel *label_3;
    QComboBox *micList_;
    QSlider *playOutVolume_;
    QComboBox *playOutList_;
    QLabel *label_4;
    QLabel *label_5;
    QCheckBox *AudioSourceCheckBox_;
    QLabel *label_6;
    QLabel *label_8;
    QLabel *label_9;
    QCheckBox *IsMirror_;
    QLineEdit *Reslution_;
    QLineEdit *Bitrate_;
    QLineEdit *FrameRate_;

    void setupUi(QDialog *DeviceSetting)
    {
        if (DeviceSetting->objectName().isEmpty())
            DeviceSetting->setObjectName(QString::fromUtf8("DeviceSetting"));
        DeviceSetting->resize(549, 530);
        buttonBox = new QDialogButtonBox(DeviceSetting);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setGeometry(QRect(450, 460, 71, 71));
        buttonBox->setOrientation(Qt::Vertical);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        label = new QLabel(DeviceSetting);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 20, 71, 16));
        videoSource = new QCheckBox(DeviceSetting);
        videoSource->setObjectName(QString::fromUtf8("videoSource"));
        videoSource->setGeometry(QRect(270, 20, 221, 16));
        micVolume = new QSlider(DeviceSetting);
        micVolume->setObjectName(QString::fromUtf8("micVolume"));
        micVolume->setGeometry(QRect(150, 340, 160, 22));
        micVolume->setOrientation(Qt::Horizontal);
        label_2 = new QLabel(DeviceSetting);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(20, 340, 151, 16));
        cameralIndex = new QComboBox(DeviceSetting);
        cameralIndex->setObjectName(QString::fromUtf8("cameralIndex"));
        cameralIndex->setGeometry(QRect(100, 20, 141, 22));
        label_3 = new QLabel(DeviceSetting);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(10, 70, 71, 16));
        micList_ = new QComboBox(DeviceSetting);
        micList_->setObjectName(QString::fromUtf8("micList_"));
        micList_->setGeometry(QRect(100, 70, 141, 22));
        playOutVolume_ = new QSlider(DeviceSetting);
        playOutVolume_->setObjectName(QString::fromUtf8("playOutVolume_"));
        playOutVolume_->setGeometry(QRect(150, 410, 160, 22));
        playOutVolume_->setOrientation(Qt::Horizontal);
        playOutList_ = new QComboBox(DeviceSetting);
        playOutList_->setObjectName(QString::fromUtf8("playOutList_"));
        playOutList_->setGeometry(QRect(100, 290, 141, 22));
        label_4 = new QLabel(DeviceSetting);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(20, 410, 151, 16));
        label_5 = new QLabel(DeviceSetting);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(10, 290, 71, 16));
        AudioSourceCheckBox_ = new QCheckBox(DeviceSetting);
        AudioSourceCheckBox_->setObjectName(QString::fromUtf8("AudioSourceCheckBox_"));
        AudioSourceCheckBox_->setGeometry(QRect(270, 70, 171, 16));
        label_6 = new QLabel(DeviceSetting);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(10, 130, 61, 21));
        label_8 = new QLabel(DeviceSetting);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setGeometry(QRect(10, 170, 61, 21));
        label_9 = new QLabel(DeviceSetting);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setGeometry(QRect(10, 210, 61, 21));
        IsMirror_ = new QCheckBox(DeviceSetting);
        IsMirror_->setObjectName(QString::fromUtf8("IsMirror_"));
        IsMirror_->setGeometry(QRect(10, 250, 221, 16));
        Reslution_ = new QLineEdit(DeviceSetting);
        Reslution_->setObjectName(QString::fromUtf8("Reslution_"));
        Reslution_->setGeometry(QRect(100, 130, 141, 20));
        Bitrate_ = new QLineEdit(DeviceSetting);
        Bitrate_->setObjectName(QString::fromUtf8("Bitrate_"));
        Bitrate_->setGeometry(QRect(100, 170, 141, 20));
        FrameRate_ = new QLineEdit(DeviceSetting);
        FrameRate_->setObjectName(QString::fromUtf8("FrameRate_"));
        FrameRate_->setGeometry(QRect(100, 210, 141, 20));

        retranslateUi(DeviceSetting);
        QObject::connect(buttonBox, SIGNAL(accepted()), DeviceSetting, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), DeviceSetting, SLOT(reject()));

        QMetaObject::connectSlotsByName(DeviceSetting);
    } // setupUi

    void retranslateUi(QDialog *DeviceSetting)
    {
        DeviceSetting->setWindowTitle(QApplication::translate("DeviceSetting", "Dialog", nullptr));
        label->setText(QApplication::translate("DeviceSetting", "\351\200\211\346\213\251\346\221\204\345\203\217\345\244\264", nullptr));
        videoSource->setText(QApplication::translate("DeviceSetting", "\346\216\250\350\247\206\351\242\221\346\225\260\346\215\256\357\274\214\344\270\215\344\275\277\347\224\250\346\221\204\345\203\217\345\244\264", nullptr));
        label_2->setText(QApplication::translate("DeviceSetting", "\350\260\203\350\212\202\351\272\246\345\205\213\351\243\216\351\237\263\351\207\217", nullptr));
        label_3->setText(QApplication::translate("DeviceSetting", "\351\200\211\346\213\251\351\272\246\345\205\213\351\243\216", nullptr));
        label_4->setText(QApplication::translate("DeviceSetting", "\350\260\203\350\212\202\346\211\254\345\243\260\345\231\250\351\237\263\351\207\217", nullptr));
        label_5->setText(QApplication::translate("DeviceSetting", "\351\200\211\346\213\251\346\211\254\345\243\260\345\231\250", nullptr));
        AudioSourceCheckBox_->setText(QApplication::translate("DeviceSetting", "\346\216\250\351\237\263\351\242\221\346\225\260\346\215\256\357\274\214\344\270\215\344\275\277\347\224\250\351\272\246\345\205\213\351\243\216", nullptr));
        label_6->setText(QApplication::translate("DeviceSetting", "\345\210\206\350\276\250\347\216\207", nullptr));
        label_8->setText(QApplication::translate("DeviceSetting", "\347\240\201\347\216\207", nullptr));
        label_9->setText(QApplication::translate("DeviceSetting", "\345\270\247\347\216\207", nullptr));
        IsMirror_->setText(QApplication::translate("DeviceSetting", "\345\274\200\345\220\257\351\225\234\345\203\217", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DeviceSetting: public Ui_DeviceSetting {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DEVICESETTING_H
