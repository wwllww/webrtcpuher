
#ifndef WEBRTC_DEVICE_MANAGER_H_
#define WEBRTC_DEVICE_MANAGER_H_

#include <memory>

#include "absl/memory/memory.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "api/video_track_source_proxy.h"
#include "media/engine/internal_decoder_factory.h"
#include "media/engine/internal_encoder_factory.h"
#include "media/engine/multiplex_codec_factory.h"

#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "modules/video_capture/video_capture_factory.h"
#include "modules/audio_device/include/audio_device_factory.h"

#include "include/IDeviceManager.h"
#include "IRTCLivePusher.h"

#define MAX_DEVICE_ID_LENGTH 256

namespace webrtcEngine 
{
class RecordedDataObserver;
    // manage local video device and audio device
class VideoDeviceManager : public IVideoDeviceManager 
{
 protected:
  VideoDeviceManager() {}
  ~VideoDeviceManager() {}

 public:
  static VideoDeviceManager* instance();
  static void releaseManager();
  virtual int getCount() override 
  {
    std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(
        webrtc::VideoCaptureFactory::CreateDeviceInfo());
    if (!info) {
      return 0;
    }
    return info->NumberOfDevices();
  }
  /**
   * active the video device for current using
   * @param [in] deviceId
   *        the deviceId of the device you want to active currently
   * @return return 0 if success or the error code.
   */
  virtual bool setDevice(const char deviceId[MAX_DEVICE_ID_LEN]) override;

  virtual bool setDeviceIndex(int deviceIndex) override;

  rtc::scoped_refptr<webrtc::VideoCaptureModule> createVideoCaptureModule(
      const char* cameraId);

  ///std::unique_ptr<cricket::VideoCapturer> openAVideoCaptureDevice();

  /**
   * get the current active video device
   * @param [in, out] deviceId
   *        the device id of the current active video device
   * @return return 0 if success or an error code
   */
  virtual bool getCurrentDevice(char deviceId[MAX_DEVICE_ID_LEN]) override 
  {
    if (m_curDeviceId.empty()) {
      return false;
    }
    strcpy(deviceId, m_curDeviceId.c_str());
    return true;
  }
  // index: begin from 0
  virtual bool getDevice(int index,
                         char deviceName[MAX_DEVICE_ID_LEN],
                         char deviceId[MAX_DEVICE_ID_LEN]) override 
  {
    std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(
        webrtc::VideoCaptureFactory::CreateDeviceInfo());
    if (!info) {
      return false;
    }

    return info->GetDeviceName(index, deviceName, MAX_DEVICE_ID_LEN, deviceId,
                               MAX_DEVICE_ID_LEN) == 0;
  }

  /**
   * release the resource
   */
  virtual void release() override{}

 private:
  std::map<std::string, std::string> getVideoDeviceInfos() const;
  std::string m_curDeviceId;
  std::string m_curDeviceName;
  static VideoDeviceManager *s_videoDeviceMan;
};

class AudioDeviceManager : public IAudioDeviceManager 
{

protected:
 AudioDeviceManager();
 ~AudioDeviceManager();

 public:
  static AudioDeviceManager* instance();
  static void Release();
  bool init(rtc::Thread *worker_thread);
  rtc::scoped_refptr<webrtc::AudioDeviceModule> getADM() const;

  virtual int32_t sendRecordedBuffer(const uint8_t* audio_data,
                                     uint32_t data_len,
                                     int bits_per_sample,
                                     int sample_rate,
                                     size_t number_of_channels) override;
  virtual void registerObserver(webrtc::AudioDeviceDataObserver*) override;
  virtual void setExternalAudioMode(bool bExternal) override;
  virtual void setAudioRecordedParam(unsigned sampleRate, unsigned nChannel) override;
  /**
   * active the playback device for current using
   * @param [in] deviceId
   *        the deviceId of the playback device you want to active
   * currently
   * @return return 0 if success or the error code.
   */
  virtual bool setPlaybackDevice(
      const char deviceId[MAX_DEVICE_ID_LEN]) override;

  int getPlaybackDeviceIndex(const char* deviceName);
  int getRecordingDeviceIndex(const char* deviceName);
  /**
   * get the current active playback device
   * @param [in, out] deviceId
   *        the device id of the current active video device
   * @return return 0 if success or an error code
   */
  virtual bool getCurPlaybackDevice(char deviceId[MAX_DEVICE_ID_LEN]) override;
  virtual int getPlaybackDeviceCount() override;
  virtual bool getPlaybackDevice(int index,
                                 char deviceName[MAX_DEVICE_ID_LEN],
                                 char deviceId[MAX_DEVICE_ID_LEN]) override;

  /**
   * set current playback device volume
   * @param [in] volume
   *        the volume you want to set 0-255
   * @return return true if success or false
   */
  virtual bool setPlaybackDeviceVolume(int volume) override;

  /**
   * get current playback device volume
   * @param [in, out] volume
   *        the current playback device volume 0-255
   * @return return true if success or false
   */
  virtual bool getPlaybackDeviceVolume(int* volume) override;

  /**
   * active the recording audio device for current using
   * @param [in] deviceId
   *        the deviceId of the recording audio device you want to
   * active currently
   * @return return true if success or false
   */
  virtual bool setRecordingDevice(
      const char deviceId[MAX_DEVICE_ID_LEN]) override;

  /**
   * get the current active recording device
   * @param [in, out] deviceId
   *        the device id of the current active recording audio device
   * @return return true if success or false
   */
  virtual bool getCurRecordingDevice(
      char deviceId[MAX_DEVICE_ID_LEN]) override;

  virtual int getRecordingDeviceCount() override;
  virtual bool getRecordingDevice(int index,
                                  char deviceName[MAX_DEVICE_ID_LEN],
                                  char deviceId[MAX_DEVICE_ID_LEN]) override;

  /**
   * set current recording device volume
   * @param [in] volume
   *        the volume you want to set 0-255
   * @return return 0 if success or false
   */
  virtual bool setRecordingDeviceVolume(int volume) override;

  /**
   * get current recording device volume
   * @param [in, out] volume
   *        the current recording device volume 0-255
   * @return return true if success or false
   */
  virtual bool getRecordingDeviceVolume(int* volume) override;

  virtual bool setPlaybackDeviceMute(bool mute) override;
  virtual bool getPlaybackDeviceMute(bool* mute) override;
  virtual bool setRecordingDeviceMute(bool mute) override;
  virtual bool getRecordingDeviceMute(bool* mute) override;

  bool initRecording();
  bool startRecording();
  bool stopRecording();
  bool startPlayOut();
  bool stopPlayOut();


  /**
   * release the resource
   */
  virtual void release() override{}

 private:
 	  rtc::scoped_refptr<webrtc::AudioDeviceModule> m_audioDevice = nullptr;
	  static AudioDeviceManager* s_audioDeviceMan;
      rtc::Thread* m_workerThread = nullptr;
      std::string m_curPlaybackId;
      std::string m_curRecorderId;
};

}  // namespace webrtcEngine

class RTCLivePusher;
class VideoManagerImpl : public IVideoDeviceManager
{
public:
	VideoManagerImpl(RTCLivePusher* livePusher);
	~VideoManagerImpl() = default;
	unsigned getDeviceCount();

	bool getCurDeviceID(char* szDeviceID);
	bool setCurDevice(unsigned deviceIndex);
	bool setCurDeviceID(const char* deviceId);
	bool getDevice(unsigned nIndex, char* deviceName, char* deviceID);

private:
	webrtcEngine::IVideoDeviceManager* videoDevice = nullptr;
	RTCLivePusher *m_livePusher = nullptr;
};


class AudioRecoderManagerImpl : public IMicManager
{
public:
	AudioRecoderManagerImpl();
	~AudioRecoderManagerImpl() = default;
	int getVolume();
	bool setVolume(unsigned nVol);

	unsigned getDeviceCount();

	bool getCurDeviceID(char* szDeviceID);
	bool setCurDevice(unsigned deviceIndex);
	bool setCurDeviceID(const char* deviceId);
	bool getDevice(unsigned nIndex, char* deviceName, char* deviceID);

private:
	webrtcEngine::IAudioDeviceManager* AudioDeviceMager = nullptr;
};


class AudioPlayoutManagerImpl : public IPlayoutManager
{
public:
	AudioPlayoutManagerImpl();
	~AudioPlayoutManagerImpl() = default;
	int getVolume() override;
	bool setVolume(unsigned nVol) override;

	unsigned getDeviceCount() override;

	bool getCurDeviceID(char* szDeviceID) override;
	bool setCurDevice(unsigned deviceIndex) override;
	bool setCurDeviceID(const char* deviceId) override;
	bool getDevice(unsigned nIndex, char* deviceName, char* deviceID) override;
private:
	webrtcEngine::IAudioDeviceManager* AudioDeviceMager = nullptr;
};

#endif  // RTC_DEVICE_MANAGER_H_
