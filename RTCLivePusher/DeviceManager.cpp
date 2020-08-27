#include "DeviceManager.h"
#include "modules/audio_device/include/audio_device_data_observer.h"
#include "RTCLivePusher.h"
namespace webrtcEngine
{
VideoDeviceManager* VideoDeviceManager::s_videoDeviceMan = nullptr;
webrtcEngine::VideoDeviceManager* VideoDeviceManager::instance() 
{
  if (s_videoDeviceMan == nullptr) {
    s_videoDeviceMan = new VideoDeviceManager();
  }
  return s_videoDeviceMan;
}

void VideoDeviceManager::releaseManager() 
{
  if (s_videoDeviceMan) {
    delete s_videoDeviceMan;
  }
  s_videoDeviceMan = nullptr;
}

bool VideoDeviceManager::setDevice(const char deviceId[MAX_DEVICE_ID_LEN]) 
{
  assert(deviceId != nullptr);
  
  if (deviceId == nullptr) {
    return false;
  }
  /// m_curDeviceName will be set at createVideoCaptureModule, if success
  m_curDeviceId = deviceId;

  return true;
}


bool VideoDeviceManager::setDeviceIndex(int deviceIndex) 
{
	return true;
}

rtc::scoped_refptr<webrtc::VideoCaptureModule>
VideoDeviceManager::createVideoCaptureModule(const char* cameraId)
{

  std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(
      webrtc::VideoCaptureFactory::CreateDeviceInfo());
  if (!info) {
    return nullptr;
  }

  int num_devices = info->NumberOfDevices();
  for (int i = 0; i < num_devices; ++i) {
    const uint32_t kSize = 256;
    char name[kSize] = {0};
    char id[kSize] = {0};
    if (info->GetDeviceName(i, name, kSize, id, kSize) != -1) {
      if (strcmp(cameraId, id) == 0) {
        auto obj = webrtc::VideoCaptureFactory::Create(id);
        if (obj) {
          m_curDeviceName = name;
          m_curDeviceId = id;
          return obj;
		}
      }

    }
  }
   
  return nullptr;
}

#if 0
std::unique_ptr<cricket::VideoCapturer>
VideoDeviceManager::openAVideoCaptureDevice() {
  cricket::WebRtcVideoDeviceCapturerFactory factory;
  if (!m_curDeviceName.empty()) {
    return factory.Create(cricket::Device(m_curDeviceName, 0));
  }

  std::map<std::string, std::string> device_infos =
      VideoDeviceManager::instance()->getVideoDeviceInfos();
  std::unique_ptr<cricket::VideoCapturer> capturer;
  for (const auto& i: device_infos) {
    capturer = factory.Create(cricket::Device(i.first, 0));
    if (capturer) {
      m_curDeviceName = i.first;
      m_curDeviceId = i.second;
      break;
    }
  }
  return capturer;
}
#endif

std::map<std::string, std::string> VideoDeviceManager::getVideoDeviceInfos() const 
{
  std::map<std::string, std::string> device_infos;
  std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(
      webrtc::VideoCaptureFactory::CreateDeviceInfo());
  if (!info) {
    return device_infos;
  }
  int num_devices = info->NumberOfDevices();
  for (int i = 0; i < num_devices; ++i) {
    const uint32_t kSize = 256;
    char name[kSize] = {0};
    char id[kSize] = {0};
    if (info->GetDeviceName(i, name, kSize, id, kSize) != -1) {
      device_infos[name] = id;
    }
  }
  return device_infos;
}

AudioDeviceManager* AudioDeviceManager::s_audioDeviceMan = nullptr;
 AudioDeviceManager::AudioDeviceManager() {}

 AudioDeviceManager::~AudioDeviceManager() 
 {
   if (m_workerThread) {
	 m_workerThread->Invoke<void>(RTC_FROM_HERE,
                                  [&] { m_audioDevice = nullptr; });
	 }
 }

webrtcEngine::AudioDeviceManager* AudioDeviceManager::instance() 
{
   if (s_audioDeviceMan == nullptr) {
     s_audioDeviceMan = new AudioDeviceManager();
   }
   return s_audioDeviceMan;
 }

 void AudioDeviceManager::Release()
 {
   if (s_audioDeviceMan) {
     delete s_audioDeviceMan;
     s_audioDeviceMan = nullptr;
   }
 }

bool AudioDeviceManager::init(rtc::Thread* worker_thread) 
{

  m_workerThread = worker_thread;
  RTC_CHECK(m_workerThread);
  m_audioDevice = m_workerThread->Invoke<rtc::scoped_refptr<webrtc::AudioDeviceModule>>(
          RTC_FROM_HERE, [&] {
            return webrtc::CreateAudioDeviceWithDataObserver(
                webrtc::AudioDeviceModule::AudioLayer::kPlatformDefaultAudio,nullptr);
          });

  //RTC_CHECK(m_audioDevice);
  return true;
 }

rtc::scoped_refptr<webrtc::AudioDeviceModule> AudioDeviceManager::getADM() const 
{
   return m_audioDevice;
}

int32_t AudioDeviceManager::sendRecordedBuffer(const uint8_t* audio_data,
                                                uint32_t data_len,
                                                int bits_per_sample,
                                                int sample_rate,
                                                size_t number_of_channels) 
{
  if (!m_audioDevice)
      return -1;

   return m_audioDevice->SendRecordedBuffer(
       audio_data, data_len, bits_per_sample, sample_rate, number_of_channels);

}

void AudioDeviceManager::registerObserver(webrtc::AudioDeviceDataObserver* recorded_data_observer) 
{
  if (!m_audioDevice)
      return;

  ((webrtc::ADMWrapper*)m_audioDevice.get())->RegisterObserver(recorded_data_observer);
}

void AudioDeviceManager::setExternalAudioMode(bool bExternal) 
{
  if (!m_audioDevice)
      return;

  if (bExternal) {
    stopRecording();
    m_audioDevice->EnableExternalAudioInput(bExternal);
  } else {
    m_audioDevice->EnableExternalAudioInput(bExternal);
    initRecording();
    startRecording();
  }
}

void AudioDeviceManager::setAudioRecordedParam(unsigned sampleRate,unsigned nChannel) 
{
  if (!m_audioDevice)
    return;

  ((webrtc::ADMWrapper*)m_audioDevice.get())->SetRecordedSampleRate(sampleRate);
  ((webrtc::ADMWrapper*)m_audioDevice.get())->SetRecordedChannels(nChannel);
}

bool AudioDeviceManager::setPlaybackDevice(
     const char deviceId[MAX_DEVICE_ID_LEN]) 
{
   if (!m_audioDevice)
     return false;

   int idx = getPlaybackDeviceIndex(deviceId);
   if (idx == -1) {
     return false;
   }

   if (stopPlayOut()) {
     m_curPlaybackId = deviceId;
	 bool ret = m_audioDevice->SetPlayoutDevice(idx) == 0;
     startPlayOut();
     return ret;
   }

   return false;
 }

int AudioDeviceManager::getPlaybackDeviceIndex(const char* deviceName) 
{
   if (deviceName == nullptr) {
     return -1;
   }
   if (!m_audioDevice)
     return false;

   int num_devices = getPlaybackDeviceCount();
   for (int i = 0; i < num_devices; ++i) {
     const uint32_t kSize = 256;
     char name[kSize] = {0};
     char id[kSize] = {0};
     if (m_audioDevice->PlayoutDeviceName(i, name, id) != -1) {
       if (strcmp(id, deviceName) == 0) {
         return i;
       }
     }
   }
   return -1;
 }

int AudioDeviceManager::getRecordingDeviceIndex(const char* deviceName) 
{
   if (deviceName == nullptr) {
     return -1;
   }
   if (!m_audioDevice)
     return false;

   int num_devices = getRecordingDeviceCount();
   for (int i = 0; i < num_devices; ++i) {
     const uint32_t kSize = 256;
     char name[kSize] = {0};
     char id[kSize] = {0};
     if (m_audioDevice->RecordingDeviceName(i, name, id) != -1) {
       if (strcmp(id, deviceName) == 0) {
         return i;
       }
     }
   }
   return -1;
 }

bool AudioDeviceManager::getCurPlaybackDevice(
     char deviceId[MAX_DEVICE_ID_LEN])
{

	if (m_curPlaybackId.empty()) {
		  char deviceName[MAX_DEVICE_ID_LEN] = {0};
          if(getPlaybackDevice(-1, deviceName, deviceId))
				m_curPlaybackId = deviceId;
          else {
            return false;
		  }
    } else {
        strcpy(deviceId, m_curPlaybackId.c_str());
	}

   return true;
 }

int AudioDeviceManager::getPlaybackDeviceCount() 
{
   if (!m_audioDevice)
     return 0;
   return m_audioDevice->PlayoutDevices();
 }

bool AudioDeviceManager::getPlaybackDevice(int index,
                                            char deviceName[MAX_DEVICE_ID_LEN],
                                            char deviceId[MAX_DEVICE_ID_LEN])
{
   if (!m_audioDevice)
     return false;
   return m_audioDevice->PlayoutDeviceName(index, deviceName, deviceId) == 0;
 }

bool AudioDeviceManager::setPlaybackDeviceVolume(int volume) 
{
   if (!m_audioDevice)
     return false;
   return m_audioDevice->SetSpeakerVolume(volume) == 0;
 }

bool AudioDeviceManager::getPlaybackDeviceVolume(int* volume) 
{
  if (!m_audioDevice)
    return false;

  uint32_t v = 0;
  bool ret = m_audioDevice->SpeakerVolume(&v) == 0;
  *volume = v;
  return ret;
}

bool AudioDeviceManager::setRecordingDevice(
    const char deviceId[MAX_DEVICE_ID_LEN]) 
{
  if (!m_audioDevice)
    return false;

  int idx = getRecordingDeviceIndex(deviceId);
  if (idx == -1) {
    return false;
  }
  m_curRecorderId = deviceId;
  int ret = false;
  if (m_audioDevice->Recording()) {
	stopRecording();
    ret = m_audioDevice->SetRecordingDevice(idx) == 0;
    startRecording();
  } else {
    if (m_audioDevice->RecordingIsInitialized()) {
      stopRecording();
      ret = m_audioDevice->SetRecordingDevice(idx) == 0;
      m_audioDevice->InitRecording();
    } else {
      ret = m_audioDevice->SetRecordingDevice(idx) == 0;
	}
  }
  return ret;
}

bool AudioDeviceManager::getCurRecordingDevice(
    char deviceId[MAX_DEVICE_ID_LEN]) 
{

	if (m_curRecorderId.empty()) {
		char deviceName[MAX_DEVICE_ID_LEN] = {0};
          if (getRecordingDevice(-1, deviceName, deviceId))
				m_curRecorderId = deviceId;
          else
				return false;
  
	} else {
       strcpy(deviceId, m_curRecorderId.c_str());
	}

  return true;
}

int AudioDeviceManager::getRecordingDeviceCount() 
{
  if (!m_audioDevice)
    return false;

  return m_audioDevice->RecordingDevices();
}

bool AudioDeviceManager::getRecordingDevice(int index,
                                            char deviceName[MAX_DEVICE_ID_LEN],
                                            char deviceId[MAX_DEVICE_ID_LEN]) 
{
  if (!m_audioDevice)
    return false;

  return m_audioDevice->RecordingDeviceName(index, deviceName, deviceId) == 0;
}

bool AudioDeviceManager::setRecordingDeviceVolume(int volume)
{
  if (!m_audioDevice)
    return false;
  return m_audioDevice->SetMicrophoneVolume(volume) == 0;
}

bool AudioDeviceManager::getRecordingDeviceVolume(int* volume) 
{
  if (!m_audioDevice)
    return false;

  uint32_t v = 0;
  bool ret = m_audioDevice->MicrophoneVolume(&v) == 0;
  *volume = v;
  return ret;
}

bool AudioDeviceManager::setPlaybackDeviceMute(bool mute)
{
  if (!m_audioDevice)
    return false;
  return m_audioDevice->SetSpeakerMute(mute) == 0;
}

bool AudioDeviceManager::getPlaybackDeviceMute(bool* mute)
{
  if (!m_audioDevice)
    return false;
  return m_audioDevice->SpeakerMute(mute) == 0;
}

bool AudioDeviceManager::setRecordingDeviceMute(bool mute)
{
  if (!m_audioDevice)
    return false;
  return m_audioDevice->SetMicrophoneMute(mute) == 0;
}

bool AudioDeviceManager::getRecordingDeviceMute(bool* mute)
{
  if (!m_audioDevice)
    return false;
  return m_audioDevice->MicrophoneMute(mute) == 0;
}

bool AudioDeviceManager::initRecording() 
{
  if (!m_audioDevice)
    return false;
  return m_audioDevice->InitRecording() == 0;
}

bool AudioDeviceManager::startRecording()
{
  if (!m_audioDevice)
		return false;

  m_audioDevice->InitRecording();
  return m_workerThread->Invoke<bool>(
      RTC_FROM_HERE, [&] { return m_audioDevice->StartRecording() == 0; });
}

bool AudioDeviceManager::stopRecording() 
{
  if (!m_audioDevice)
    return false;

  return m_workerThread->Invoke<bool>(
      RTC_FROM_HERE, [&] { return m_audioDevice->StopRecording() == 0; });
}

bool AudioDeviceManager::startPlayOut()
{
  if (!m_audioDevice)
    return false;

   if (!m_audioDevice->Playing()) {
    if (m_audioDevice->InitPlayout() == 0) {
      return m_workerThread->Invoke<bool>(
           RTC_FROM_HERE, [&] { return m_audioDevice->StartPlayout() == 0; });
      }
    } 
   return false;
 
}

bool AudioDeviceManager::stopPlayOut()
{
  if (!m_audioDevice)
    return false;

  if (m_audioDevice->Playing()) {
    return m_workerThread->Invoke<bool>(
        RTC_FROM_HERE, [&] { return m_audioDevice->StopPlayout() == 0; });
  }
  return true;
}

}  // namespace webrtcEngine

VideoManagerImpl::VideoManagerImpl(RTCLivePusher* livePusher)
{
	m_livePusher = livePusher;
	videoDevice = webrtcEngine::VideoDeviceManager::instance();
}

unsigned VideoManagerImpl::getDeviceCount()
{
	if (!videoDevice) { return 0; }

	return videoDevice->getCount();
}

bool VideoManagerImpl::getCurDeviceID(char* szDeviceID)
{
	assert(szDeviceID);
	if (!szDeviceID) return false;
	if (!videoDevice) { return false; }

	return videoDevice->getCurrentDevice(szDeviceID);
}

bool VideoManagerImpl::setCurDevice(unsigned deviceIndex)
{
	if (!videoDevice) { return false; }

	char deviceName[MAX_DEVICE_ID_LENGTH];
	char deviceID[MAX_DEVICE_ID_LENGTH];
	if (!getDevice(deviceIndex, deviceName, deviceID))
		return false;

	m_livePusher->setVideoDeviceIndex(deviceIndex);

	if (!m_livePusher->setLocalDevice(deviceID)) {
		return false;
	}

	return videoDevice->setDeviceIndex(deviceIndex);
}

bool VideoManagerImpl::setCurDeviceID(const char* deviceId)
{
	if (!deviceId) {
		return false;
	}

	if (!m_livePusher->setLocalDevice(deviceId)) {
		return false;
	}

	if (!videoDevice) { return false; }

	return videoDevice->setDevice(deviceId);
}

bool VideoManagerImpl::getDevice(unsigned nIndex, char* deviceName, char* deviceID)
{
	if (!videoDevice) { return false; }
	return videoDevice->getDevice(nIndex, deviceName, deviceID);
}

AudioRecoderManagerImpl::AudioRecoderManagerImpl()
{
	AudioDeviceMager = webrtcEngine::AudioDeviceManager::instance();
}

int AudioRecoderManagerImpl::getVolume()
{
	if (!AudioDeviceMager) { return false; }

	int v = 0;
	bool ret = AudioDeviceMager->getRecordingDeviceVolume(&v);
	return ret ? v : -1;
}

bool AudioRecoderManagerImpl::setVolume(unsigned nVol)
{
	if (!AudioDeviceMager) { return false; }

	return AudioDeviceMager->setRecordingDeviceVolume(nVol);
}

unsigned AudioRecoderManagerImpl::getDeviceCount()
{
	if (!AudioDeviceMager) { return 0; }

	return AudioDeviceMager->getRecordingDeviceCount();
}

bool AudioRecoderManagerImpl::getCurDeviceID(char* szDeviceID)
{
	assert(szDeviceID);
	if (!szDeviceID) return false;
	if (!AudioDeviceMager) { return false; }

	return AudioDeviceMager->getCurRecordingDevice(szDeviceID);
}

bool AudioRecoderManagerImpl::setCurDevice(unsigned deviceIndex)
{
	if (!AudioDeviceMager) { return false; }

	char deviceName[MAX_DEVICE_ID_LENGTH];
	char deviceID[MAX_DEVICE_ID_LENGTH];
	if (!getDevice(deviceIndex, deviceName, deviceID))
		return false;

	return AudioDeviceMager->setRecordingDevice(deviceID);
}

bool AudioRecoderManagerImpl::setCurDeviceID(const char* deviceId)
{
	if (!AudioDeviceMager) { return false; }

	return AudioDeviceMager->setRecordingDevice(deviceId);
}

bool AudioRecoderManagerImpl::getDevice(unsigned nIndex, char* deviceName, char* deviceID)
{
	if (!AudioDeviceMager) { return false; }

	return AudioDeviceMager->getRecordingDevice(nIndex, deviceName, deviceID);
}

AudioPlayoutManagerImpl::AudioPlayoutManagerImpl()
{
	AudioDeviceMager = webrtcEngine::AudioDeviceManager::instance();
}

int AudioPlayoutManagerImpl::getVolume()
{
	if (!AudioDeviceMager) { return false; }

	int v = 0;
	bool ret = AudioDeviceMager->getPlaybackDeviceVolume(&v);
	return ret ? v : -1;
}

bool AudioPlayoutManagerImpl::setVolume(unsigned nVol)
{
	if (!AudioDeviceMager) { return false; }

	return AudioDeviceMager->setPlaybackDeviceVolume(nVol);
}

unsigned AudioPlayoutManagerImpl::getDeviceCount()
{
	if (!AudioDeviceMager) { return false; }

	return AudioDeviceMager->getPlaybackDeviceCount();
}

bool AudioPlayoutManagerImpl::getCurDeviceID(char* szDeviceID)
{
	assert(szDeviceID);
	if (!szDeviceID) return false;
	if (!AudioDeviceMager) { return false; }

	return AudioDeviceMager->getCurPlaybackDevice(szDeviceID);
}

bool AudioPlayoutManagerImpl::setCurDevice(unsigned deviceIndex)
{
	if (!AudioDeviceMager) { return false; }

	char deviceName[MAX_DEVICE_ID_LENGTH];
	char deviceID[MAX_DEVICE_ID_LENGTH];
	if (!getDevice(deviceIndex, deviceName, deviceID))
		return false;

	return AudioDeviceMager->setPlaybackDevice(deviceID);
}

bool AudioPlayoutManagerImpl::setCurDeviceID(const char* deviceId)
{
	if (!AudioDeviceMager) { return false; }

	return AudioDeviceMager->setPlaybackDevice(deviceId);
}

bool AudioPlayoutManagerImpl::getDevice(unsigned nIndex, char* deviceName, char* deviceID)
{
	if (!AudioDeviceMager) { return false; }

	return AudioDeviceMager->getPlaybackDevice(nIndex, deviceName, deviceID);
}
