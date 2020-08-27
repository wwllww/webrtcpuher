#ifndef WEBRTC_MEDIASTREAM_H_
#define WEBRTC_MEDIASTREAM_H_

#include <memory>
#include <string>
#include <vector>

#include "modules/audio_device/include/audio_device_data_observer.h"
#include "VideoObserver.h"

class RTCLivePusher;
/// AudioTrackSinkInterface for record
class MediaStreamImpl:public webrtc::AudioTrackSinkInterface,
                      public webrtc::AudioDeviceDataObserver
{
public:
	MediaStreamImpl(const std::string& id,RTCLivePusher *peerconnect);
	~MediaStreamImpl();

	const char* id() const;
	rtc::VideoSinkInterface<webrtc::VideoFrame>* videoObserver() { return &m_dataObserver; }
    rtc::SEISinkInterface* seiObserver() { return &m_seiDataObserver; }
	AudioTrackSinkInterface* audioObserver() { return this; }

	void RegisterOnI420Frame(I420FRAMEREADY_CALLBACK callback) 
	{
		m_dataObserver.SetVideoCallback(callback);
	}
    void RegisterSEICallback(SEI_CALLBACK callback) 
	{
        m_seiDataObserver.SetSEICallback(callback);
    }		
	virtual void RegisterOnAudioFrame(AUDIOBUSREADY_CALLBACK callback)
	{
        std::lock_guard<std::mutex> lock(m_audioMutex);
		m_onAudioReady = callback;
	}
	virtual void RegisterOnAudioFrameWithType(AUDIOBUSREADY_CALLBACK_WITHTYPE callback) 
	{
        std::lock_guard<std::mutex> lock(m_audioMutex);
        m_onAudioReadyWithType = callback;
	}
    virtual void OnCaptureData(const void* audioSamples,
                                   const size_t nSamples,
                                   const size_t nBytesPerSample,
                                   const size_t nChannels,
                                   const uint32_t samplesRate) override 
	{
          std::lock_guard<std::mutex> lock(m_audioMutex);
          if (m_onAudioReadyWithType) 
		  {
				m_onAudioReadyWithType(audioSamples, nBytesPerSample * 8, samplesRate,
                          static_cast<int>(nChannels),
                          static_cast<int>(nSamples),true);
          }
        }

        virtual void OnRenderData(const void* audio_samples,
                            const size_t num_samples,
                            const size_t bytes_per_sample,
                            const size_t num_channels,
                            const uint32_t samples_per_sec) override 
		{
          std::lock_guard<std::mutex> lock(m_audioMutex);
          if (m_onAudioReadyWithType) {
				m_onAudioReadyWithType(audio_samples, bytes_per_sample * 8,
                                      samples_per_sec,
						              static_cast<int>(num_channels),
                                      static_cast<int>(num_samples),false);
          }
        }

       private:
	// AudioTrackSinkInterface implementation.
	void OnData(const void* audio_data,
				int bits_per_sample,
				int sample_rate,
				size_t number_of_channels,
				size_t number_of_frames) override 
	{
        std::lock_guard<std::mutex> lock(m_audioMutex);
		if (m_onAudioReady) {
			m_onAudioReady(audio_data, bits_per_sample, sample_rate,
						  static_cast<int>(number_of_channels),
						  static_cast<int>(number_of_frames));
		}
	}

protected:
	std::string				m_id;
	RTCLivePusher          *m_peerconnect = nullptr;
	AUDIOBUSREADY_CALLBACK	m_onAudioReady = nullptr;
    AUDIOBUSREADY_CALLBACK_WITHTYPE m_onAudioReadyWithType = nullptr;

	VideoObserver			m_dataObserver; 
	SEIObserver             m_seiDataObserver;
	std::mutex              m_audioMutex;
};
#endif  // WEBRTC_MEDIASTREAM_H_
