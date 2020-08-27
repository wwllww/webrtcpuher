#ifndef RTCLIVEPUSHER_H
#define RTCLIVEPUSHER_H

#include "IRTCLivePusher.h"
#include <TaskQueue/RunLoop.h>
#include "api/media_stream_interface.h"
#include "api/peer_connection_interface.h"
#include "pc/video_track_source.h"
#include "RTCVideoSink.h"
#include "DeviceManager.h"

class MediaStreamImpl;
class RTCLivePusher : public IRTCLivePusher,
	                  public webrtc::PeerConnectionObserver,
	                  public webrtc::CreateSessionDescriptionObserver {
public:
	RTCLivePusher();
	~RTCLivePusher();
	virtual IVideoDeviceManager* getVideoDeviceManager() override;
	virtual IPlayoutManager* getPlayoutManager() override;
	virtual IMicManager* getMicManager() override;
	virtual void setPushParam(const RTCPushConfig& config) override;
	virtual int registerRTCLivePushEventHandler(IRTCLivePusherEvent *eventHandler) override;
	virtual int startPreview(WindowIdType winId) override;
	virtual void stopPreview() override;
	virtual int startPush(const char* posturl,const char* streamId) override;
	virtual void stopPush() override;
	virtual int setMirror(bool isMirror) override;
	virtual int muteAudio(bool mute) override;
	virtual int muteVideo(bool mute) override;
	virtual int enableAudioVolumeEvaluation(int interval) override;
	virtual void sendVideoBuffer(const char* data, int width, int height, ColorSpace color) override;
	virtual void sendAudioBuff(const char* data, int len, int channel, int samplesPerSec) override;
	virtual int enableVideoCapture(bool enable) override;
	virtual int enableAudioCapture(bool enable) override;
	virtual bool enableExternalVideoSource(bool enable) override;
	virtual bool enableExternalAudioSource(bool enable) override;
	virtual int sendMessage(const char* msg, int len) override;

	int64_t getServerTime() const;
	void getPlayListFromSchudle();
	bool publishStream();
	void stopStream();
	bool AddTransceiverAndCreateOfferForSend();
	rtc::scoped_refptr<webrtc::AudioTrackInterface> getAudioTrack();
	rtc::scoped_refptr<webrtc::VideoTrackInterface> getVideoTrack();
	bool enableVideo(bool enable);
	bool enableAudio(bool enable);
	void setSendVideoFpsAndBitrate();
	bool setVideoFormat();
	void setVideoDeviceIndex(int videoDeviceIndex);
	bool setLocalDevice(const char* deviceName);

	void OnFrame(const uint8_t* data_y,
		const uint8_t* data_u,
		const uint8_t* data_v,
		const uint8_t* data_a,
		int stride_y,
		int stride_u,
		int stride_v,
		int stride_a,
		uint32_t width,
		uint32_t height,
		int64_t  render_time);

	void receiveAudioFrame(const void* audio_data,
		int bits_per_sample,
		int sample_rate,
		int number_of_channels,
		int number_of_frames);

	void reGenPeerConnection();
	
protected:
	// create a peerconneciton and add the turn servers info to the configuration.
	bool initPeerConnection();
	bool createPeerConnection();
	void deletePeerConnection();
	bool openLocalStream();
	bool openAudioSource();
	bool openVideoSource(bool capture);
	bool createVideoTrack(bool capture);
	void createVideoSource(bool capture);
	void releaseVideoSource();
	void closeVideoSource();
	// PeerConnectionObserver implementation.
	void OnSignalingChange(
		webrtc::PeerConnectionInterface::SignalingState new_state) override {}
	void OnAddStream(
		rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override {}
	void OnRemoveStream(
		rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override {}

	void OnAddTrack(
		rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
		const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>&
		streams) override {}
	void OnRemoveTrack(
		rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override {}
	void OnDataChannel(
		rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override {}
	void OnRenegotiationNeeded() override {}
	// remote
	void OnIceConnectionChange(
		webrtc::PeerConnectionInterface::IceConnectionState new_state) override;
	virtual void OnConnectionChange(
		webrtc::PeerConnectionInterface::PeerConnectionState new_state) override;
	// local ice info
	void OnIceGatheringChange(
		webrtc::PeerConnectionInterface::IceGatheringState new_state) override;
	// local candidate
	void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
	void OnIceConnectionReceivingChange(bool receiving) override {}

	// CreateSessionDescriptionObserver implementation.
	void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;
	void OnFailure(const std::string& error) override;

private:
	IRTCLivePusherEvent *m_eventHandler = nullptr;
	rtc::scoped_refptr<webrtc::PeerConnectionInterface> m_peerConnection;
	task::Runloop *m_runLoop = nullptr;

	std::unique_ptr<MediaStreamImpl> m_localStream;
	// video source
	rtc::scoped_refptr<webrtc::VideoTrackSource> m_localVideoTrackSource = nullptr;
	rtc::scoped_refptr<::webrtc::RtpTransceiverInterface> m_audioTransceiver;
	rtc::scoped_refptr<::webrtc::RtpTransceiverInterface> m_videoTransceiver;
	rtc::scoped_refptr<webrtc::AudioTrackInterface> m_audioTrack;
	rtc::scoped_refptr<webrtc::VideoTrackInterface> m_videoTrack;


	int m_curWidth;
	int m_curHeight;
	int m_curFps;
	int m_maxVideoBitrate;
	int m_curVideoDeviceIndex = 0;
	uint8_t *m_uBuffer = nullptr;
	RtcVideoSink m_videoSink;
	std::string m_streamId;
	std::string m_pushStreamUrl;
	std::string m_sessionId;
	std::string m_postUrl;

	//for create offer
	std::mutex m_mutexOffer;
	std::condition_variable m_conOffer;
	std::string m_offer;
	bool m_isLocalVideoEnable = false;
	bool m_isLocalAudioEnable = false;
	bool m_isExternalVideoEnabled = false;
	bool m_isExternalAudioEnabled = false;

	int64_t  m_currentServerTime = 0;
	int64_t  m_serverTimeFromSchudle = 0;
	int64_t  m_localTimeUTC = 0;
	int64_t  m_timeError = 0;
	uint64_t m_renderCount = 0;

	/*device Manager*/
	VideoManagerImpl m_videoManager;
	AudioRecoderManagerImpl m_audioRecoderManager;
	AudioPlayoutManagerImpl m_audioPlayOutManager;
	
};
#endif
