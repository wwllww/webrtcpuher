#pragma once

#include <QMainWindow>
#include "ui_DemoPusher.h"
#include "devicesetting.h"
class QTimer;
class DemoPusher : public QMainWindow
{
    Q_OBJECT

public:
    DemoPusher(QMainWindow *parent = Q_NULLPTR);
	~DemoPusher();

	void enableExternalVideoData(bool enable);
	void enableExternalAudioData(bool enable);
private slots:
	void timerPlayerVideo();
	void timerPlayerAudio();
	void manageDevice();
	void liveBtnClicked();
	void previewBtnClicked();

private:
    Ui::DemoPusherClass ui;
	QTimer *mediaTimer = nullptr;
	QTimer *audioTimer = nullptr;
	FILE *mpfile = nullptr;
	FILE *mpPCMFile = nullptr;
	char *myuvBuf = nullptr;
	char *mPCMBuf = nullptr;
	QString mfilename;
	QString mAudioFileName;
	DeviceConfig deviceConfig_;
	bool isPreview = true;
	bool isStartLive = false;
};
