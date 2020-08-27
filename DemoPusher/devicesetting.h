#ifndef DEVICESETTING_H
#define DEVICESETTING_H

#include <QDialog>
#include <QList>
struct DeviceConfig {
    int cameralIndex = 0;
    bool useExternalVideoData = false;
	bool useExternalAudioData = false;
    unsigned micVolume = 30;    // total : 255
	int micIndex = 0;
	int playOutIndex = 0;
	unsigned playOutVolume = 30;
	int width = 1280;
	int height = 720;
	int bitrate = 500;
	int fps = 15;
	bool isMirror = true;
};

namespace Ui {
class DeviceSetting;
}

class DeviceSetting : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceSetting(QWidget *parent = nullptr);
    ~DeviceSetting();

public:
    DeviceConfig getSetting();
    void setSetting(const DeviceConfig& config);
	void setPublishEnable(bool published);

private slots:
    void on_buttonBox_accepted();

    void on_micVolume_valueChanged(int value);

    void on_videoSource_clicked(bool checked);
	void on_audioSource_clicked(bool checked);
	void on_playoutVolume_valueChanged(int value);

private:
    Ui::DeviceSetting *ui;
};

#endif // DEVICESETTING_H
