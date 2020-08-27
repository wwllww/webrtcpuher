#include <chrono>
#include "DemoPusher.h"
#include "IRTCLivePusher.h"
#include <QDebug>
#include <QTimer>
#include <QFile>
#include <QToolBar>
#include <QMessageBox>


IRTCLivePusher *livePusher = nullptr;

void LogCallBack(const char* msg, void* object) {
	qDebug() << msg;
}

class eventLivePusher : public IRTCLivePusherEvent {
public:
	virtual void onPusherEvent(RTCLiveEvent event) {}
	/**
	 * @brief key {"Fps","AudioKBitRate","VideoKBitRate","cpuAppUsage","cpuAppUsage"}
	 */
	virtual void onPusherStats(const std::map<const char*, double>& stats) {}
	virtual void onCaptureVideoFrame(const char* data, int width, int height, ColorSpace color) {}
	virtual void onCaptureAudioFrame(const char* data, int len, int channel, int samplesPerSec) {}
};

eventLivePusher eventLive;


void MixYUV420(unsigned char* pDstImg, int realDstW, int realDstH, int posX, int posY,
	int dstW, int dstH, unsigned char* pSrcImg, int srcW, int srcH)
{
	unsigned char *USrcImg = pSrcImg + srcW * srcH;
	unsigned char *UDstImg = pDstImg + realDstW * realDstH;
	unsigned char *VSrcImg = pSrcImg + srcW * srcH * 5 / 4;
	unsigned char *VDstImg = pDstImg + realDstW * realDstH * 5 / 4;
	pDstImg = pDstImg + posY * realDstW + posX;

	int UsrcW = srcW >> 1;
	int UdstW = dstW >> 1;
	int UdstH = dstH >> 1;

	double rateH = (double)srcH / (double)dstH;
	double rateW = (double)srcW / (double)dstW;

	int tSrcH, tSrcW;

	for (int i = 0; i < dstH; i++)
	{
		tSrcH = (int)(rateH * double(i) + 0.5);
		for (int j = 0; j < dstW; j++)
		{
			tSrcW = (int)(rateW * double(j) + 0.5);
			pDstImg[realDstW * i + j] = pSrcImg[tSrcH * srcW + tSrcW];
		}
	}
	int pos = (posX + (posY * realDstW >> 1)) >> 1;
	int halfW = realDstW >> 1;
	for (int i = 0; i < UdstH; i++)
	{
		tSrcH = (int)(rateH * double(i) + 0.5);
		for (int j = 0; j < UdstW; j++)
		{
			tSrcW = (int)(rateW * double(j) + 0.5);
			UDstImg[i * halfW + j + pos] = USrcImg[tSrcH * UsrcW + tSrcW];
			VDstImg[i * halfW + j + pos] = VSrcImg[tSrcH * UsrcW + tSrcW];
		}
	}
}

DemoPusher::DemoPusher(QMainWindow *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

	// add tool bar
	QToolBar *toolbar = new QToolBar(this);

	QAction*  deviceMan = new QAction("Device Manager", this);
	toolbar->addAction(deviceMan);
	deviceMan->setShortcut(Qt::Key_Control | Qt::Key_E);
	deviceMan->setToolTip("Device Manager");
	connect(deviceMan, SIGNAL(triggered()), this, SLOT(manageDevice()));
	addToolBar(toolbar);

	connect(ui.LiveButton_, SIGNAL(clicked(bool)), SLOT(liveBtnClicked()));
	connect(ui.PreviewButton_, SIGNAL(clicked(bool)), SLOT(previewBtnClicked()));

	ui.streamId_->setText("test");

	rtcRegisterLogFunc(LogCallBack, nullptr);
	livePusher = createRTCLivePusher();
	livePusher->registerRTCLivePushEventHandler(&eventLive);
	livePusher->enableVideoCapture(true);
	livePusher->enableAudioCapture(true);
	//livePusher->startPush("x_3_stream");
	//livePusher->enableExternalVideoSource(true);
	livePusher->startPreview((WindowIdType)ui.m_videoShow->winId());

	QString runPath = QCoreApplication::applicationDirPath(); // get app path

	mfilename = runPath + "/foreman_320x240.yuv";
	mAudioFileName = runPath + "/test.pcm";

	int Width = 1280;
	int Height = 720;
	unsigned char *renderTarget = new unsigned char[Width * Height * 1.5];
	memset(renderTarget,0, Width * Height);
	memset(renderTarget + Width * Height,0x80, Width * Height / 2);

	FILE *yuvFile = fopen("D:/test.1280x720_i420.yuv", "rb");

	if (!yuvFile) {
		return;
	}
	unsigned char *src = new unsigned char[1280 * 720 * 1.5];
	fread(src, 1, 1280 * 720 * 1.5, yuvFile);
	fclose(yuvFile);
	
	long long start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	for (int i = 0; i < 100; i++) {
		MixYUV420(renderTarget, Width, Height, 320, 0, 640, 480, src, 1280, 720);
		MixYUV420(renderTarget, Width, Height, 0, 480, 320, 240, src, 1280, 720);
		MixYUV420(renderTarget, Width, Height, 320, 480, 320, 240, src, 1280, 720);
		MixYUV420(renderTarget, Width, Height, 640, 480, 320, 240, src, 1280, 720);
		MixYUV420(renderTarget, Width, Height, 960, 480, 320, 240, src, 1280, 720);
	}
	
	long long end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - start;
	qDebug() << "cost ----------:  " << end << " ms";
	
	FILE *file = fopen("D:/testPinjie.yuv", "wb");
	if (!file) {
		return;
	}

	fwrite(renderTarget, 1, Width * Height * 1.5, file);
	fclose(file);
}

DemoPusher::~DemoPusher()
{
	destroyRTCLivePusher(livePusher);
}

void DemoPusher::enableExternalVideoData(bool enable)
{
	livePusher->enableExternalVideoSource(enable);
	if (enable) {
		if (!mediaTimer) {
			mediaTimer = new QTimer(this);
			connect(mediaTimer, SIGNAL(timeout()), SLOT(timerPlayerVideo()));
		}

		mediaTimer->start(40);
	}
	else {
		mediaTimer->stop();
		livePusher->enableVideoCapture(true);
	}
}

void DemoPusher::enableExternalAudioData(bool enable)
{
	livePusher->enableExternalAudioSource(enable);
	if (enable) {

		if (!audioTimer) {
			audioTimer = new QTimer(this);
			connect(audioTimer, SIGNAL(timeout()), SLOT(timerPlayerAudio()));
		}
		audioTimer->start(10);
	}
	else {
		audioTimer->stop();
	}
}

void DemoPusher::timerPlayerVideo()
{
	//获取yuv 数据发送
	if (mpfile == NULL) {
		QFile bfilePath(mfilename);
		if (!bfilePath.exists()) {
			return;
		}
		mpfile = fopen(mfilename.toLocal8Bit().toStdString().c_str(), "rb");
	}

	if (!mpfile)
	{
		QMessageBox::information(this, "info", "open video file failed");
		mediaTimer->stop();
		return;
	}

	if (feof(mpfile)) {
		qDebug() << "timerPlayeData eof";
		fseek(mpfile, 0, SEEK_SET);
	}

	if (myuvBuf == NULL) {
		myuvBuf = new char[320 * 240 * 2];
	}
	if (mpfile != NULL) {
		fread(myuvBuf, 1, 320 * 240 * 3 / 2, mpfile);
		livePusher->sendVideoBuffer(myuvBuf,320,240,Color_YUVI420);
	}
}

void DemoPusher::timerPlayerAudio()
{
	if (mpPCMFile == NULL) {
		QFile bfilePath(mAudioFileName);
		if (!bfilePath.exists())
			return;

		mpPCMFile = fopen(mAudioFileName.toLocal8Bit().toStdString().c_str(), "rb");
	}

	if (!mpPCMFile) {
		QMessageBox::information(this, "info", "open audio file failed");
		audioTimer->stop();
		return;
	}

	if (mpPCMFile && feof(mpPCMFile)) {
		fseek(mpPCMFile, 0, SEEK_SET);
	}
	unsigned len = 480 * 2 * 2;
	if (mPCMBuf == NULL) {
		mPCMBuf = new char[len];
	}

	if (mpPCMFile)
	{
		int nCount = fread(mPCMBuf, 1, len, mpPCMFile);
		livePusher->sendAudioBuff(mPCMBuf, len, 2, 48000);
	}
	else
		qDebug() << "timerPlayeAudio failed";
}

void DemoPusher::manageDevice()
{
	DeviceSetting dialog(this);

	dialog.setSetting(deviceConfig_);

	if (dialog.exec() == QDialog::Accepted) {
		DeviceConfig oldConfig = deviceConfig_;
		deviceConfig_ = dialog.getSetting();

		if (oldConfig.cameralIndex != deviceConfig_.cameralIndex)
			livePusher->getVideoDeviceManager()->setCurDevice(deviceConfig_.cameralIndex);

		if (oldConfig.micIndex != deviceConfig_.micIndex)
			livePusher->getMicManager()->setCurDevice(deviceConfig_.micIndex);

		if (oldConfig.playOutIndex != deviceConfig_.playOutIndex)
			livePusher->getPlayoutManager()->setCurDevice(deviceConfig_.playOutIndex);

		if (oldConfig.width != deviceConfig_.width || oldConfig.height != deviceConfig_.height ||
			oldConfig.bitrate != deviceConfig_.bitrate || oldConfig.fps != deviceConfig_.fps) {

			RTCPushConfig config;
			config.width = deviceConfig_.width;
			config.height = deviceConfig_.height;
			config.bitrate = deviceConfig_.bitrate;
			config.fps = deviceConfig_.fps;
			livePusher->setPushParam(config);
		}

		if (oldConfig.isMirror != deviceConfig_.isMirror) {
			livePusher->setMirror(deviceConfig_.isMirror);
		}

		if (oldConfig.useExternalVideoData != deviceConfig_.useExternalVideoData) {
			enableExternalVideoData(deviceConfig_.useExternalVideoData);
		}

		if (oldConfig.useExternalAudioData != deviceConfig_.useExternalAudioData) {
			enableExternalAudioData(deviceConfig_.useExternalAudioData);
		}
	}
}

void DemoPusher::liveBtnClicked()
{
	if (isStartLive) {
		livePusher->stopPush();
		ui.LiveButton_->setText(QString::fromLocal8Bit("开始直播"));
	}
	else {
		
		QString &streamId = ui.streamId_->text();
		QString &posturl = ui.postUrl->text();

		if (streamId.isEmpty() || posturl.isEmpty()) {
			QMessageBox::information(this, "info", "streamId or posturl is empty");
			return;
		}
		
		livePusher->startPush(posturl.toLocal8Bit(),streamId.toLocal8Bit());
		ui.LiveButton_->setText(QString::fromLocal8Bit("结束直播"));
	}
	isStartLive = !isStartLive;
}

void DemoPusher::previewBtnClicked()
{
	if (isPreview) {
		livePusher->stopPreview();
		ui.PreviewButton_->setText(QString::fromLocal8Bit("开启预览"));
	}
	else {
		livePusher->startPreview((WindowIdType)ui.m_videoShow->winId());
		ui.PreviewButton_->setText(QString::fromLocal8Bit("关闭预览"));
	}
	isPreview = !isPreview;
}
