
#include <QComboBox.h>
#include <QDebug>

#include "devicesetting.h"
#include "ui_devicesetting.h"
#include "IRTCLivePusher.h"

#define MAX_DEVICE_ID_LENGTH 256

extern  IRTCLivePusher *livePusher;

DeviceSetting::DeviceSetting(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeviceSetting)
{
    ui->setupUi(this);
	
	ui->micVolume->setMinimum(0);
	ui->micVolume->setMaximum(255);

	ui->playOutVolume_->setMinimum(0);
	ui->playOutVolume_->setMaximum(255);

	connect(ui->micVolume, SIGNAL(valueChanged(int)), this, SLOT(on_micVolume_valueChanged(int)));
	connect(ui->videoSource, SIGNAL(clicked(bool)), this, SLOT(on_videoSource_clicked(bool)));

	connect(ui->playOutVolume_, SIGNAL(valueChanged(int)), this, SLOT(on_playoutVolume_valueChanged(int)));
	connect(ui->AudioSourceCheckBox_, SIGNAL(clicked(bool)), this, SLOT(on_audioSource_clicked(bool)));
	ui->Reslution_->setText("1280x720");
	ui->Bitrate_->setText("500");
	ui->FrameRate_->setText("15");
}

DeviceSetting::~DeviceSetting()
{
    delete ui;
}

void DeviceSetting::on_buttonBox_accepted()
{
}

DeviceConfig DeviceSetting::getSetting()
{
    DeviceConfig  config;
    config.cameralIndex = ui->cameralIndex->currentIndex();
    if (config.cameralIndex < 0)
        config.cameralIndex = 0;
    config.useExternalVideoData = ui->videoSource->checkState() == Qt::Checked;
	config.useExternalAudioData = ui->AudioSourceCheckBox_->checkState() == Qt::Checked;
	config.micVolume = ui->micVolume->value();
	config.micIndex = ui->micList_->currentIndex();
	config.playOutIndex = ui->playOutList_->currentIndex();
	config.playOutVolume = ui->playOutVolume_->value();
	std::string Res = ui->Reslution_->text().toLocal8Bit().data();
	size_t pos = Res.find('x');
	if (pos != std::string::npos) {
		config.width = atoi(Res.substr(0, pos).c_str());
		config.height = atoi(Res.substr(pos + 1).c_str());
	}
	else {
		config.width = 1280;
		config.height = 720;
	}

	config.bitrate = atoi(ui->Bitrate_->text().toLocal8Bit().data());
	config.fps = atoi(ui->FrameRate_->text().toLocal8Bit().data());
	config.isMirror = ui->IsMirror_->isChecked();
	return config;
}

void DeviceSetting::setPublishEnable(bool published)
{
	if (published) {
		ui->videoSource->setDisabled(published);
	}
}

void DeviceSetting::setSetting(const DeviceConfig& config)
{

	IMicManager* micMan = livePusher->getMicManager();
	IPlayoutManager* playoutMan = livePusher->getPlayoutManager();
	
	int micCount = micMan->getDeviceCount();
	
	for (int i = 0; i < micCount; ++i) {
		char name[MAX_DEVICE_ID_LENGTH] = { 0 };
		char id[MAX_DEVICE_ID_LENGTH] = { 0 };
		if (!micMan->getDevice(i, name, id))
			continue;
		ui->micList_->addItem(QString(name));
	}

	micCount = playoutMan->getDeviceCount();

	for (int i = 0; i < micCount; ++i) {
		char name[MAX_DEVICE_ID_LENGTH] = { 0 };
		char id[MAX_DEVICE_ID_LENGTH] = { 0 };

		if (!playoutMan->getDevice(i, name, id))
			continue;
		ui->playOutList_->addItem(QString(name));
	}
		
	IVideoDeviceManager* videoMan = livePusher->getVideoDeviceManager();
	int cameraCount = videoMan->getDeviceCount();
	for (int i = 0; i < cameraCount; ++i) {

		char name[MAX_DEVICE_ID_LENGTH] = { 0 };
		char id[MAX_DEVICE_ID_LENGTH] = { 0 };
		if (!videoMan->getDevice(i, name, id))
			continue;
		ui->cameralIndex->addItem(name);// strUtil::Utf8ToUnicode(name));
	}

	if (config.useExternalVideoData)
		ui->cameralIndex->setDisabled(true);

	if (config.useExternalAudioData) {
		ui->micList_->setDisabled(true);
	}
	ui->AudioSourceCheckBox_->setChecked(config.useExternalAudioData);
	ui->videoSource->setChecked(config.useExternalVideoData);

	std::string Res = std::to_string(config.width) + "x" + std::to_string(config.height);
	ui->Reslution_->setText(Res.c_str());
	ui->cameralIndex->setCurrentIndex(config.cameralIndex);
	ui->playOutList_->setCurrentIndex(config.playOutIndex);
	ui->micList_->setCurrentIndex(config.micIndex);

	ui->playOutVolume_->setValue(playoutMan->getVolume());
	ui->micVolume->setValue(micMan->getVolume());

	ui->Bitrate_->setText(std::to_string(config.bitrate).c_str());
	ui->FrameRate_->setText(std::to_string(config.fps).c_str());

	ui->IsMirror_->setChecked(config.isMirror);
}

void DeviceSetting::on_micVolume_valueChanged(int value)
{
	livePusher->getMicManager()->setVolume(value);
}

void DeviceSetting::on_audioSource_clicked(bool checked)
{
	ui->micList_->setDisabled(checked);
}

void DeviceSetting::on_videoSource_clicked(bool checked)
{
	ui->cameralIndex->setDisabled(checked);
}

void DeviceSetting::on_playoutVolume_valueChanged(int value)
{
	livePusher->getPlayoutManager()->setVolume(value);
}
