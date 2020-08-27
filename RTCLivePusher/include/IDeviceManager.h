
#ifndef WEBRTC_IDEVICE_MANAGER_H_
#define WEBRTC_IDEVICE_MANAGER_H_

#include <memory>
namespace webrtc 
{
	class AudioDeviceDataObserver;
}
namespace webrtcEngine 
{	
	static const int MAX_DEVICE_ID_LEN = 256;

	class IVideoDeviceManager 
	{
         public:
          virtual ~IVideoDeviceManager() {}

          virtual int getCount() = 0;
          /**
           * active the video device for current using
           * @param [in] deviceId
           *        the deviceId of the device you want to active currently
           * @return return true if success or false
           */
          virtual bool setDevice(const char deviceId[MAX_DEVICE_ID_LEN]) = 0;

		  /**
           * active the video device index for current using
           * @param [in] device index
           *        the device index of the device you want to active currently
           * @return return true if success or false
           */
          virtual bool setDeviceIndex(int deviceIndex) = 0;

          /**
           * get the current active video device
           * @param [in, out] deviceId
           *        the device id of the current active video device
           * @return return true if success or false
           */
          virtual bool getCurrentDevice(char deviceId[MAX_DEVICE_ID_LEN]) = 0;
          // index: begin from 0
          virtual bool getDevice(int index,
                                 char deviceName[MAX_DEVICE_ID_LEN],
                                 char deviceId[MAX_DEVICE_ID_LEN]) = 0;

          /**
           * release the resource
           */
          virtual void release() = 0;
    };


    class IAudioDeviceManager 
	{
        public:
		virtual ~IAudioDeviceManager() {}

        /**
        * active the playback device for current using
        * @param [in] deviceId
        *        the deviceId of the playback device you want to active
        * currently
        * @return return true if success or false.
        */
        virtual bool setPlaybackDevice(
            const char deviceId[MAX_DEVICE_ID_LEN]) = 0;

        /**
        * get the current active playback device
        * @param [in, out] deviceId
        *        the device id of the current active video device
        * @return return true if success or false
        */
        virtual bool getCurPlaybackDevice(char deviceId[MAX_DEVICE_ID_LEN]) = 0;
        virtual bool getPlaybackDevice(int index,
                                       char deviceName[MAX_DEVICE_ID_LEN],
                                      char deviceId[MAX_DEVICE_ID_LEN]) = 0;
        virtual int getPlaybackDeviceCount() = 0;

        /**
        * set current playback device volume
        * @param [in] volume
        *        the volume you want to set 0-255
        * @return return true if success or false
        */
        virtual bool setPlaybackDeviceVolume(int volume) = 0;

        /**
        * get current playback device volume
        * @param [in, out] volume
        *        the current playback device volume 0-255
        * @return return true if success or false
        */
        virtual bool getPlaybackDeviceVolume(int* volume) = 0;

        /**
        * active the recording audio device for current using
        * @param [in] deviceId
        *        the deviceId of the recording audio device you want to
        * active currently
        * @return return true if success or false
        */
        virtual bool setRecordingDevice(
            const char deviceId[MAX_DEVICE_ID_LEN]) = 0;

        /**
        * get the current active recording device
        * @param [in, out] deviceId
        *        the device id of the current active recording audio device
        * @return return true if success or false
        */
        virtual bool getCurRecordingDevice(
            char deviceId[MAX_DEVICE_ID_LEN]) = 0;        
        virtual int getRecordingDeviceCount() = 0;
        virtual bool getRecordingDevice(int index,
                                        char deviceName[MAX_DEVICE_ID_LEN],
                                       char deviceId[MAX_DEVICE_ID_LEN]) = 0;

        /**
        * set current recording device volume
        * @param [in] volume
        *        the volume you want to set 0-255
        * @return return true if success or false
        */
        virtual bool setRecordingDeviceVolume(int volume) = 0;
        virtual bool getRecordingDeviceVolume(int* volume) = 0;
		        

        virtual bool setPlaybackDeviceMute(bool mute) = 0;
        virtual bool getPlaybackDeviceMute(bool* mute) = 0;
        virtual bool setRecordingDeviceMute(bool mute) = 0;
        virtual bool getRecordingDeviceMute(bool* mute) = 0;
		
		virtual int32_t sendRecordedBuffer(const uint8_t* audio_data,
                                           uint32_t data_len,
                                           int bits_per_sample,
                                           int sample_rate,
                                           size_t number_of_channels) = 0;
        virtual void registerObserver(webrtc::AudioDeviceDataObserver*) = 0;
        virtual void setExternalAudioMode(bool bExternal) = 0;
        virtual void setAudioRecordedParam(unsigned sampleRate,unsigned nChannel) = 0;
        /**
        * release the resource
        */
        virtual void release() = 0;
    };


}  // namespace webrtcEngine
#endif  // RTC_IDEVICE_MANAGER_H_
