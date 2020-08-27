#include "api/create_peerconnection_factory.h"
#include "api/stats/rtc_stats_report.h"
#include "api/stats/rtcstats_objects.h"
#include "system_wrappers/include/field_trial.h"
#include "third_party/libyuv/include/libyuv.h"
#include "modules/rtp_rtcp/source/byte_io.h"
#include "api/video_codecs/video_encoder.h"

#include <mutex>
#include <nlohmann/json.hpp>
#include <cpp-httplib/httplib.h>
#include <util/url.hpp>
#include <util/timer.h>
#include "RTCLivePusher.h"
#include "RtcLogWrite.h"
#include "DeviceManager.h"
#include "MediaStream.h"
#include "CapturerTrackSource.h"
#include "ExternalVideoFrameTrackSource.h"

static const char* PLAY_STREAM_URL = "http://rtcapi.xueersi.com/rtclive/v1/publish";
static const char* STOP_STREAM_URL = "http://rtcapi.xueersi.com/rtclive/v1/unpublish";
static const char* SCHEDULE_URL = "http://rtcapi.xueersi.com/rtclive/v1/services";

const int kWidth = 1280;
const int kHeight = 720;
const int kFps = 15;
const int kBitrate = 500;
const int kSendSEISeconds = 5;
static const uint8_t uuid_live_start_time[16] =
{
	0xa8, 0x5f, 0xe4, 0xe9, 0x1b, 0x69, 0x11, 0xe8,
	0x85, 0x82, 0x00, 0x50, 0xc2, 0x49, 0x00, 0x48
};

std::unique_ptr<rtc::Thread> g_worker_thread;
std::unique_ptr<rtc::Thread> g_signaling_thread;
rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
g_peer_connection_factory;
log_callback g_logCallBack = nullptr;
void *logObject = nullptr;


static const char *s_webrtcRttMult = "WebRTC-RttMult/Enabled-1.0/"
                                     "WebRTC-Video-ResolutionDegradation/Enabled/"
	                                 "WebRTC-Audio-OpusMinPacketLossRate/Enabled-20/";
const char g_kAudioLabel[] = "audio";
const char g_kVideoLabel[] = "video";
const char g_kStreamId[] = "stream_id";

#define SDK_VERSION "1.0.0"

#define BEGIN_ASYN_THREAD_CALL_1(p1)	if (nullptr != m_runLoop) {	m_runLoop->AddRunner(task::Clouser([this, p1]() {
#define BEGIN_ASYN_THREAD_CALL_2(p1,p2)	if (nullptr != m_runLoop) {	m_runLoop->AddRunner(task::Clouser([this, p1, p2]() {
#define BEGIN_ASYN_THREAD_CALL			if (nullptr != m_runLoop) {	m_runLoop->AddRunner(task::Clouser([this]() {
#define BEGIN_SYN_THREAD_CALL			if (nullptr != m_runLoop) {	m_runLoop->AddSynRunner(task::Clouser([&]() {
#define END_THREAD_CALL		}));}


// long SSL_ctrl(SSL *s, int cmd, long larg, void *parg)
// {
// 	return 0;
// }

std::string GetEnvVarOrDefault(const char* env_var_name,
	const char* default_value)
{
	std::string value;
	const char* env_var = getenv(env_var_name);
	if (env_var)
		value = env_var;

	if (value.empty())
		value = default_value;

	return value;
}

void externalVideoFrame2YuvI420(void* buffer,int width,int height,uint64_t timestamp, ColorSpace color, ExternalVideoFrame& videoFrame)
{
	videoFrame.height = height;
	videoFrame.width = width;
	videoFrame.renderTimeMs = timestamp;
	videoFrame.rotation = 0;
	videoFrame.yStride = videoFrame.width;
	videoFrame.uStride = videoFrame.width / 2;
	videoFrame.vStride = videoFrame.width / 2;

	if (color == Color_YUVI420) {
		videoFrame.yBuffer = buffer;
		videoFrame.uBuffer = (char*)videoFrame.yBuffer + videoFrame.height*videoFrame.width;
		int size_uv = ((videoFrame.width + 1) >> 1) * ((videoFrame.height + 1) >> 1);
		videoFrame.vBuffer = (char*)videoFrame.uBuffer + size_uv;
	}
	else if (color == Color_RGB32) {
		libyuv::ABGRToI420((const uint8_t*)buffer, width * 4,
		    (uint8_t*)videoFrame.yBuffer, videoFrame.yStride,
		    (uint8_t*)videoFrame.uBuffer, videoFrame.uStride,
		    (uint8_t*)videoFrame.vBuffer, videoFrame.vStride,
		    videoFrame.width, videoFrame.height);
	}
	else if (color == Color_RGB24) {
		libyuv::RGB24ToI420((const uint8_t*)buffer,width * 3,
			(uint8_t*)videoFrame.yBuffer, videoFrame.yStride,
			(uint8_t*)videoFrame.uBuffer, videoFrame.uStride,
			(uint8_t*)videoFrame.vBuffer, videoFrame.vStride,
			videoFrame.width, videoFrame.height);
     }
}

void videoDataCallback(const uint8_t* data_y,
	const uint8_t* data_u,
	const uint8_t* data_v,
	const uint8_t* data_a,
	int stride_y,
	int stride_u,
	int stride_v,
	int stride_a,
	uint32_t width,
	uint32_t height,
	int64_t  render_time,
	void *context)
{
	RTCLivePusher* pusher = (RTCLivePusher*)context;
	pusher->OnFrame(data_y, data_u, data_v, data_a,
		stride_y, stride_u, stride_v, stride_a,
		width, height, render_time);
}
void audioDataCallback(const void* audio_data,
	int bits_per_sample,
	int sample_rate,
	int number_of_channels,
	int number_of_frames,
	void *context)
{
	RTCLivePusher* pusher = (RTCLivePusher*)context;
	pusher->receiveAudioFrame(audio_data, bits_per_sample, sample_rate, number_of_channels, number_of_frames);
}

class DummySetSessionDescriptionObserver
	: public webrtc::SetSessionDescriptionObserver
{
public:
	static DummySetSessionDescriptionObserver* Create()
	{
		return new rtc::RefCountedObject<DummySetSessionDescriptionObserver>();
	}
	virtual void OnSuccess() { LOG_INFO("set remote or local Description sucesss"); }
	virtual void OnFailure(webrtc::RTCError error)
	{
		LOG_ERROR("set remote or local Description failed type: " << ToString(error.type()) << " message: " << error.message());
	}

protected:
	DummySetSessionDescriptionObserver() {}
	~DummySetSessionDescriptionObserver() {}
};

std::mutex g_spcMutex;
std::map<unsigned long long, rtc::scoped_refptr<RTCLivePusher>> g_allSpcs;

IRTCLivePusher* createRTCLivePusher()
{
	LOGER;
	rtc::scoped_refptr<RTCLivePusher> spc(
		new rtc::RefCountedObject<RTCLivePusher>());
	std::lock_guard<std::mutex> locker(g_spcMutex);
	g_allSpcs[(unsigned long long)spc.get()] = spc;
	return spc.get();
}

void destroyRTCLivePusher(IRTCLivePusher* rtcLivePusher) {
	LOG_INFO("rtcLivePusher: " << rtcLivePusher);
	if (rtcLivePusher) {
		std::lock_guard<std::mutex> locker(g_spcMutex);
		g_allSpcs.erase((unsigned long long)rtcLivePusher);
	}
		
}

void rtcRegisterLogFunc(log_callback fn, void *object /*= nullptr*/)
{
	g_logCallBack = fn;
	logObject = object;
}

RTCLivePusher::RTCLivePusher():m_curWidth(kWidth),m_curHeight(kHeight),
                               m_curFps(kFps),m_maxVideoBitrate(kBitrate), m_videoManager(this)
{
	LOGER;
	initPeerConnection();
	openLocalStream();
	m_runLoop = task::Runloop::Create();
	webrtc::VideoEncoder::keyFrameInterval_ = m_curFps * 5;
}

RTCLivePusher::~RTCLivePusher()
{
	LOG_INFO("begin!");
	BEGIN_SYN_THREAD_CALL
		if (m_peerConnection) {
			LOG_INFO("delete peerconnetion");
			if (m_localStream) {
				webrtcEngine::AudioDeviceManager::instance()->registerObserver(nullptr);
				m_localStream->RegisterOnI420Frame(nullptr);
				m_localStream->RegisterOnAudioFrame(nullptr);
			}
			m_localStream.reset();

			m_audioTrack = nullptr;
			m_videoTrack = nullptr;

			closeVideoSource();
			deletePeerConnection();
			LOG_INFO("delete peerconnetion end!");
		}
	END_THREAD_CALL
	LOG_INFO("delete m_runLoop begin!");
	if (m_runLoop) {
		m_runLoop->Stop();
		m_runLoop = nullptr;
	}
	LOG_INFO("delete m_runLoop end!");

	if (m_uBuffer) {
		delete[] m_uBuffer;
		m_uBuffer = nullptr;
	}

	webrtcEngine::AudioDeviceManager::Release();
	webrtcEngine::VideoDeviceManager::releaseManager();

	m_peerConnection = nullptr;
	g_peer_connection_factory = nullptr;
	g_signaling_thread.reset();
	g_worker_thread.reset();
	LOG_INFO("end!");
}

IVideoDeviceManager* RTCLivePusher::getVideoDeviceManager()
{
	return &m_videoManager;
}

IPlayoutManager* RTCLivePusher::getPlayoutManager()
{
	return &m_audioPlayOutManager;
}

IMicManager* RTCLivePusher::getMicManager()
{
	return &m_audioRecoderManager;
}

void RTCLivePusher::setPushParam(const RTCPushConfig& config)
{
	LOG_INFO("begin,width: " << config.width << " height: " << config.height << " fps: " << config.fps << " bitrate: " << config.bitrate);

	int newWidth = config.width;
	int newHeight = config.height;
	int newFps = config.fps;
	int newVideoBitrate = config.bitrate;

	if (newWidth <= 0 || newWidth > 1920) {
		newWidth = kWidth;
	}

	if (newHeight <= 0 || newHeight > 1080) {
		newHeight = kHeight;
	}

	if (newFps <= 0 || newFps > 30) {
		newFps = kFps;
	}

	if (newVideoBitrate <= 0 || newVideoBitrate > 3000) {
		newVideoBitrate = kBitrate;
	}

	bool widthHeightChanged = m_curWidth != newWidth || m_curHeight != newHeight;
	bool fpsChanged = m_curFps != newFps;
	bool videoBitrateChanged = m_maxVideoBitrate != newVideoBitrate;
	m_curWidth = newWidth;
	m_curHeight = newHeight;
	m_curFps = newFps;
	m_maxVideoBitrate = newVideoBitrate;

	if (widthHeightChanged || fpsChanged) {
		LOG_INFO("widthHeightChanged: " << widthHeightChanged << " fpsChanged:" << fpsChanged);
		setVideoFormat();
	}

	if (fpsChanged || videoBitrateChanged) {
		LOG_INFO("videoBitrateChanged: " << videoBitrateChanged << " fpsChanged:" << fpsChanged);
		setSendVideoFpsAndBitrate();
	}

	LOG_INFO("end,width: " << m_curWidth << " height: " << m_curHeight << " fps: " << m_curFps << " bitrate: " << m_maxVideoBitrate);
}

int RTCLivePusher::registerRTCLivePushEventHandler(IRTCLivePusherEvent *eventHandler)
{
	LOG_INFO("eventHandler: " << eventHandler);
	this->m_eventHandler = eventHandler;
	return 0;
}

int RTCLivePusher::startPreview(WindowIdType winId)
{
	if (winId == nullptr) {
		LOG_ERROR("winId is null");
		return -1;
	}
	LOG_INFO("begin! winId: " << winId);
	if (!m_videoSink.setVideoWindow(winId)) {
		LOG_ERROR("setVideoWindow failed");
		return -2;
	}
	m_videoSink.StartRenderer();
	LOG_INFO("end!");
	return 0;
}

void RTCLivePusher::stopPreview()
{
	LOG_INFO("begin!");
	m_videoSink.StopRenderer();
	LOG_INFO("end!");
}

int RTCLivePusher::startPush(const char* posturl,const char* streamId)
{

	if (streamId == nullptr || posturl == nullptr) {
		LOG_ERROR("streamId is null");
		return -1;
	}
	m_streamId = streamId;
	m_postUrl = posturl;
	LOG_INFO("begin! streamId: " << streamId);
	BEGIN_ASYN_THREAD_CALL
		getPlayListFromSchudle();
		publishStream();
	END_THREAD_CALL
	LOG_INFO("end!");
	return 0;
}

void RTCLivePusher::stopPush()
{
	LOG_INFO("begin!");
	BEGIN_ASYN_THREAD_CALL
		stopStream();
	END_THREAD_CALL
	LOG_INFO("end!");
}


int RTCLivePusher::setMirror(bool isMirror)
{
	LOG_INFO("begin! isMirror: " << isMirror);
	m_videoSink.setMirror(isMirror);
	LOG_INFO("end!");
	return 0;
}

int RTCLivePusher::muteAudio(bool mute)
{
	LOG_INFO("begin! mute: " << mute);
	BEGIN_ASYN_THREAD_CALL_1(mute)
		LOG_INFO("mute: " << mute);
		if (m_audioTrack) {
			m_audioTrack->set_enabled(!mute);
		}
	END_THREAD_CALL
	LOG_INFO("end!");
	return 0;
}

int RTCLivePusher::muteVideo(bool mute)
{
	LOG_INFO("begin! mute: " << mute);
	BEGIN_ASYN_THREAD_CALL_1(mute)
		LOG_INFO("mute: " << mute);
		if (m_videoTrack) {
			m_videoTrack->set_enabled(!mute);
		}
	END_THREAD_CALL
	LOG_INFO("end!");
	return 0;
}

int RTCLivePusher::enableAudioVolumeEvaluation(int interval)
{
	LOG_INFO("begin!");

	LOG_INFO("end!");
	return 0;
}

void RTCLivePusher::sendVideoBuffer(const char* data, int width, int height, ColorSpace color)
{
	if (data == nullptr || width <= 0 || height <= 0){
		LOG_ERROR("invalidate data: " << data << " width: " << width << " height: " << height);
		return;
	}

	if (!m_isExternalVideoEnabled || !m_localVideoTrackSource)
		return;

	ExternalVideoFrameTrackSource* trackSource =
		dynamic_cast<ExternalVideoFrameTrackSource*>(m_localVideoTrackSource.get());
	if (trackSource) {
		ExternalVideoFrame sampleBuffer;
		externalVideoFrame2YuvI420((void*)data, width, height, rtc::TimeMillis(), color, sampleBuffer);
		trackSource->pushExternalVideoFrame(&sampleBuffer);
	}
		
}

void RTCLivePusher::sendAudioBuff(const char* data, int len, int channel, int samplesPerSec)
{
	if (data == nullptr || len <= 0 || channel <= 0 || channel > 2 || samplesPerSec <= 0) {
		LOG_ERROR("invalidate data: " << data << " channel: " << channel << " samplesPerSec: " << samplesPerSec);
		return;
	}

	if (!m_isExternalAudioEnabled)
		return;
	webrtcEngine::AudioDeviceManager::instance()->sendRecordedBuffer((const uint8_t*)data, len, 16, samplesPerSec, channel);
}

int RTCLivePusher::enableVideoCapture(bool enable)
{
	LOG_INFO("begin! enable: " << enable);
	BEGIN_ASYN_THREAD_CALL_1(enable)
		LOG_INFO("enable: " << enable);
		if (m_isLocalVideoEnable == enable) {
			LOG_INFO("repead");
			return;
		}

		if (enable) {
			if (!openVideoSource(true)) {
				LOG_ERROR("openVideoSource failed!");
			}
		}
		else {
			enableVideo(false);
		}
		m_isLocalVideoEnable = enable;
	END_THREAD_CALL
	LOG_INFO("end!");
	return 0;
}

int RTCLivePusher::enableAudioCapture(bool enable)
{
	LOG_INFO("begin! enable: " << enable);
	BEGIN_ASYN_THREAD_CALL_1(enable)
		LOG_INFO("enable: " << enable);
		if (m_isLocalAudioEnable == enable) {
			LOG_INFO("m_isLocalAudioEnable repeat ");
			return;
		}

		if (enable) {
			if (!openAudioSource()) {
				LOG_ERROR("openLocalAudio failed!");
			}
		}
		else {
			enableAudio(false);
		}
		m_isLocalAudioEnable = enable;
	END_THREAD_CALL
	LOG_INFO("end!");
	return 0;
}

bool RTCLivePusher::enableExternalVideoSource(bool enable)
{
	LOG_INFO("begin,enable: " << enable);

	BEGIN_ASYN_THREAD_CALL_1(enable)
		LOG_INFO("enable: " << enable);
		if (m_isExternalVideoEnabled == enable) {
			LOG_INFO("m_isExternalVideoEnabled repeat!");
			return;
		}
		if (enable) {
			if (!openVideoSource(false)) {
				LOG_ERROR("openExternalVideoSource failed");
				return;
			}
			m_isLocalVideoEnable = false;
		}
		else {
			enableVideo(false);
		}
		m_isExternalVideoEnabled = enable;
		
	END_THREAD_CALL

	LOG_INFO("end");
	return true;
}

bool RTCLivePusher::enableExternalAudioSource(bool enable)
{
	LOG_INFO("begin,enable: " << enable);

	BEGIN_ASYN_THREAD_CALL_1(enable)
		LOG_INFO("enable: " << enable);
		if (m_isExternalAudioEnabled == enable) {
			LOG_INFO("m_isExternalAudioEnabled repeat!");
			return;
		}
		webrtcEngine::AudioDeviceManager::instance()->setExternalAudioMode(enable);
		m_isExternalAudioEnabled = enable;
		if (enable) {
			m_isLocalAudioEnable = false;
		}
	END_THREAD_CALL

	LOG_INFO("end");
	return true;
}

int RTCLivePusher::sendMessage(const char* msg, int len)
{
	//test
	int64_t currentTime = getServerTime();

	if (currentTime == -1) {
		LOG_ERROR("getServerTime error");
		return -1;
	}

	uint8_t seiInfo[50] = { 0 };
	int index = 0;
	seiInfo[index++] = 0x0;
	seiInfo[index++] = 0x0;
	seiInfo[index++] = 0x0;
	seiInfo[index++] = 0x1;
	seiInfo[index++] = 0x06;
	seiInfo[index++] = 0x05;
	seiInfo[index++] = 0x18;
	memcpy(&seiInfo[index], uuid_live_start_time, sizeof(uuid_live_start_time) / sizeof(uint8_t));
	index += sizeof(uuid_live_start_time) / sizeof(uint8_t);
	webrtc::ByteWriter<int64_t>::WriteBigEndian(&seiInfo[index], currentTime);
	index += sizeof(int64_t);
	seiInfo[index++] = 0x80;

	rtc::CopyOnWriteBuffer buffer;
	buffer.SetData(seiInfo, index);

	LOG_INFO("begin!" << " size: " << buffer.size());

	BEGIN_ASYN_THREAD_CALL_1(buffer)
		webrtcEngine::CapturerTrackSource *capture = dynamic_cast<webrtcEngine::CapturerTrackSource *>(m_localVideoTrackSource.get());
		if (capture) {
			bool ret = capture->sendSEIMessage((const char*)buffer.cdata(), buffer.size());
			LOG_INFO("is captureVideoSource ret: " << ret << " size: " << buffer.size());
			return;
		}

		ExternalVideoFrameTrackSource *external = dynamic_cast<ExternalVideoFrameTrackSource*>(m_localVideoTrackSource.get());
		if (external) {
			bool ret = external->sendSEIMessage((const char*)buffer.cdata(), buffer.size());
			LOG_INFO("is externalVideoSource ret: " << ret << " size: " << buffer.size());
			return;
		}
	END_THREAD_CALL
	LOG_INFO("end!");
	return 0;
}

int64_t RTCLivePusher::getServerTime() const {
	if (m_serverTimeFromSchudle == 0)
		return -1;
	return m_serverTimeFromSchudle + chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count() - m_localTimeUTC;
}

void RTCLivePusher::getPlayListFromSchudle()
{
	LOG_INFO("begin!");

	m_serverTimeFromSchudle = 0;

	nlohmann::json answerReq;
	answerReq["streamid"] = m_streamId;

	std::string reqUrl = SCHEDULE_URL;
	http::url url = http::ParseHttpUrl(reqUrl);
	std::unique_ptr<httplib::Client> cli;
	if (url.protocol == "https") {
		return;
	}
	else {
		cli.reset(new httplib::Client(url.host.c_str(), url.port, 5));
	}

	LOG_INFO("answerReq: " << answerReq.dump().c_str() << " reqUrl: " << reqUrl);
	long long startTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
	auto res = cli->Post(url.path.c_str(), answerReq.dump().c_str(), "application/json; charset=utf-8");

	if (res) {
		LOG_INFO("respond: " << res->body);
	}

	if (res && res->status == 200) {
		nlohmann::json jsonResponse = nlohmann::json::parse(res->body);
		try {

			nlohmann::json &arryList = jsonResponse["d"]["address"];
			if (!arryList.is_array() || arryList.size() == 0) {
				LOG_ERROR("end,address is not array or size = 0!");
				return;
			}

			if (!jsonResponse["d"]["online"].is_null()) {
				if (!jsonResponse["d"]["online"].get<bool>()) {
					LOG_ERROR("stream offline startStreamNotFoundTimer");
				}
			}

			{
				//在这里取服务器时间
				m_localTimeUTC = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
				if (!jsonResponse["d"]["timestamp"].is_null()) {
					m_serverTimeFromSchudle = jsonResponse["d"]["timestamp"].get<int64_t>() / 1000000;
					//m_serverTimeFromSchudle += m_localTimeUTC - startTime;
					m_timeError = m_localTimeUTC - startTime;
					if (m_timeError > 500) {
						LOG_ERROR("time error so big: " << m_timeError << "reset to -1");
						m_timeError = -1;
					}
				}
				else {
					m_timeError = -1;
				}

				LOG_INFO("m_timeError: " << m_timeError);
			}
		}
		catch (std::exception& e) {
			m_timeError = -1;
			LOG_ERROR("josn format is error: " << e.what());
		}

	}
	else {
		//上报调度失败日志
		LOG_ERROR("end!schudle return error!");
		m_timeError = -1;
		return;
	}
	LOG_INFO("end!");
}

bool RTCLivePusher::publishStream()
{
	LOG_INFO("begin!");

	if (!m_videoTrack && !openVideoSource(m_isLocalVideoEnable)) {
		LOG_ERROR("end! openVideoSource failed");
		return false;
	}

	if (!m_audioTrack && !openAudioSource()) {
		LOG_ERROR("end! openAudioSource failed");
		return false;
	}

	if (!AddTransceiverAndCreateOfferForSend()) {
		LOG_ERROR("end! AddTransceiverAndCreateOfferForSend error");
		return false;
	}

	

	//m_pushStreamUrl = "webrtc://txrtc.wangxiao.eaydu.com/live_xes/x_3_test111";
    m_pushStreamUrl = "webrtc://xrtc.wangxiao.eaydu.com/live_xes/" + m_streamId;
	size_t pos = m_pushStreamUrl.find_last_of('/');
	if (pos == m_pushStreamUrl.size() - 1) {
		m_pushStreamUrl = m_pushStreamUrl.substr(0, pos);
	}
	

	nlohmann::json answerReq;
	answerReq["event"] = "publish";
	answerReq["data"]["reconnect"] = false;
	answerReq["data"]["sdp"] = m_offer.c_str();
	answerReq["data"]["stream"]["publisherId"] = m_streamId;
	answerReq["data"]["stream"]["data"]["bitrate"] = m_maxVideoBitrate;
	answerReq["data"]["stream"]["rtcTyp"] = "artc";

	//std::string reqUrl = PLAY_STREAM_URL;
	//std::string reqUrl = "http://182.254.168.205:1985/rtc/v1/play/";
	std::string reqUrl = "http://121.40.106.102:6000/rtc/v1/publish";
	//std::string reqUrl = "http://182.254.168.205:8000/rtc/v1/publish";
	http::url url = http::ParseHttpUrl(reqUrl);
	std::unique_ptr<httplib::Client> cli;
	if (url.protocol == "https") {
		//cli.reset(new httplib::SSLClient(url.host.c_str(), url.port, 3));
		//((httplib::SSLClient*)cli.get())->enable_server_certificate_verification(false);
		LOG_ERROR("https not supported");
		return false;
	}
	else {
		cli.reset(new httplib::Client(url.host.c_str(), url.port, 3));
	}

	LOG_INFO("answerReq: " << answerReq.dump().c_str() << " reqUrl: " << reqUrl);

	auto res = cli->Post(url.path.c_str(), answerReq.dump().c_str(), "application/json; charset=utf-8");

	if (res)
		LOG_INFO("respond: " << res->body);

	if (res && res->status == 200) {
		if (res->body.empty()) {
			LOG_ERROR("body is empty");
			return false;
		}
		nlohmann::json jsonResponse = nlohmann::json::parse(res->body);
		std::string sdp;
		try {
			//m_sessionId = jsonResponse["d"]["sessionid"].get<string>();
			sdp = jsonResponse["d"]["sdp"].get<string>();
			//sdp = jsonResponse["sdp"].get<string>();
		}
		catch (std::exception &e) {
			LOG_ERROR("json format is error: " << e.what());
			return false;
		}

		if (sdp.empty()) {
			LOG_ERROR("sdp is empty!");
			return false;
		}

		LOG_INFO("remote sdp: " << sdp.c_str());

		int pos =  sdp.find("a=fmtp:111");
		std::string newSdp;
		if (pos != std::string::npos) {
			newSdp = sdp.substr(0, pos);
			int p = sdp.find_first_of("\r\n", pos);
			newSdp += sdp.substr(pos, p - pos);
			newSdp += ";maxplaybackrate=8000;usedtx=1\r\n";
			newSdp += sdp.substr(p + 2);
		}

		webrtc::SdpParseError error;
		webrtc::SessionDescriptionInterface* session_description(
			webrtc::CreateSessionDescription("answer", newSdp, &error));
		if (!session_description) {
			LOG_ERROR("Can't parse received session description message. "
				<< "SdpParseError was: " << error.description);
			return false;
		}
		m_peerConnection->SetRemoteDescription(
			DummySetSessionDescriptionObserver::Create(), session_description);
	}
	else {
		LOG_ERROR("http post error");
		return false;
	}

	LOG_INFO("end!")
	return true;
}

void RTCLivePusher::stopStream()
{
	LOG_INFO("begin!");
	reGenPeerConnection();
	if (m_sessionId.empty()) {
		LOG_INFO("end! m_sessionId is empty!");
		return;
	}

	nlohmann::json req;
	req["streamurl"] = m_pushStreamUrl;
	req["sessionid"] = m_sessionId.c_str();
	std::string reqUrl = STOP_STREAM_URL;
	http::url url = http::ParseHttpUrl(reqUrl);
	std::unique_ptr<httplib::Client> cli;
	if (url.protocol == "https") {
		//cli.reset(new httplib::SSLClient(url.host.c_str(), url.port, 3));
		//((httplib::SSLClient*)cli.get())->enable_server_certificate_verification(false);
		LOG_ERROR("https not supported");
		return;
	}
	else {
		cli.reset(new httplib::Client(url.host.c_str(), url.port, 3));
	}

	LOG_INFO("req: " << req.dump().c_str() << " reqUrl: " << reqUrl);

	auto res = cli->Post(url.path.c_str(), req.dump().c_str(), "application/json; charset=utf-8");

	if (res)
		LOG_INFO("respond: " << res->body);

	if (res && res->status == 200) {
		//sucess
	}

	m_sessionId.clear();

	LOG_INFO("end!");
}

bool RTCLivePusher::AddTransceiverAndCreateOfferForSend()
{
	LOG_INFO("begin!");
	if (!m_localStream) {
		LOG_ERROR("end! m_localStream is null");
		return false;
	}

	::webrtc::RtpTransceiverInit init;
	init.stream_ids.push_back(m_localStream->id());
	init.direction = ::webrtc::RtpTransceiverDirection::kSendOnly;
	::webrtc::RTCErrorOr<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>> ret;
	if (m_audioTrack) {
		LOG_INFO("add m_audioTrack");
		ret = m_peerConnection->AddTransceiver(m_audioTrack, init);
		m_audioTransceiver = ret.value();
	}
	else {
		LOG_ERROR("end! m_audioTrack is null");
		return false;
	}


	init.send_encodings.resize(1);
	init.send_encodings[0].max_bitrate_bps = m_maxVideoBitrate * 1000;
	init.send_encodings[0].max_framerate = m_curFps;
	if (m_videoTrack) {
		LOG_INFO("add m_videoTrack");
		ret = m_peerConnection->AddTransceiver(m_videoTrack, init);
		m_videoTransceiver = ret.value();
	}
	else {
		LOG_ERROR("end! m_videoTrack is null");
		return false;
	}
	

	webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;

	LOG_INFO("create offer");
	{
		std::unique_lock<std::mutex> locker(m_mutexOffer);
		m_peerConnection->CreateOffer(this, options);
		m_conOffer.wait(locker);
		if (m_offer.empty()) {
			LOG_ERROR("m_Offer is empty!!");
			return false;
		}
	}

	LOG_INFO("end!");
	return true;
}

rtc::scoped_refptr<webrtc::AudioTrackInterface> RTCLivePusher::getAudioTrack()
{
	return m_audioTrack;
}

rtc::scoped_refptr<webrtc::VideoTrackInterface> RTCLivePusher::getVideoTrack()
{
	return m_videoTrack;
}

bool RTCLivePusher::enableVideo(bool enable)
{
	LOG_INFO("begin! enable: " << enable);
	if (!m_videoTransceiver || !m_videoTrack) {
		LOG_INFO("end! m_videoTransceiver or m_videoTrack is null");
		return false;
	}
	
	if (enable) {
		m_videoTransceiver->sender()->SetTrack(m_videoTrack);
	}
	else {
		m_videoTransceiver->sender()->SetTrack(nullptr);
		m_videoTrack = nullptr;

		if (m_localVideoTrackSource) {
			closeVideoSource();
		}
	}
	LOG_INFO("end!");
	return true;
}

bool RTCLivePusher::enableAudio(bool enable)
{
	LOG_INFO("begin! enable: " << enable);
	if (!m_audioTransceiver || !m_audioTrack) {
		LOG_INFO("end! m_audioTransceiver or m_audioTrack is null");
		return false;
	}

	if (enable) {
		m_audioTransceiver->sender()->SetTrack(m_audioTrack);
	}
	else {
		m_audioTransceiver->sender()->SetTrack(nullptr);
		m_audioTrack = nullptr;
	}
	LOG_INFO("end!");
	return true;
}

void RTCLivePusher::setSendVideoFpsAndBitrate()
{
	if (!m_localStream) {
		LOG_WARN("m_localStream is null");
		return;
	}
	LOG_INFO("m_curFps: " << m_curFps << " m_maxVideoBitrate: " << m_maxVideoBitrate);
	BEGIN_ASYN_THREAD_CALL
		if (!m_videoTransceiver) {
			LOG_ERROR("m_videoTransceiver is null");
			return;
		}
	    LOG_INFO("m_curFps: " << m_curFps << " m_maxVideoBitrate: " << m_maxVideoBitrate);
		webrtc::RtpParameters parametersToModify = m_videoTransceiver->sender()->GetParameters();
		for (auto &encoding : parametersToModify.encodings) {
			encoding.max_framerate = m_curFps;
			encoding.max_bitrate_bps = m_maxVideoBitrate * 1000;
		}
		m_videoTransceiver->sender()->SetParameters(parametersToModify);
	END_THREAD_CALL
}

bool RTCLivePusher::setVideoFormat()
{
	LOG_INFO("begin,m_curWidth: " << m_curWidth << " m_curHeight:" << m_curHeight << " m_curFps: " << m_curFps);
	if (!m_localStream) {
		LOG_ERROR("end,m_localStream is null");
		return false;
	}
	BEGIN_ASYN_THREAD_CALL
	    bool ret = g_signaling_thread->Invoke<bool>(RTC_FROM_HERE, [&] {
		if (!m_localVideoTrackSource) {
			LOG_ERROR("m_localVideoTrackSource is null");
			return false;
		}
		LOG_INFO("begin,m_curWidth: " << m_curWidth << " m_curHeight:" << m_curHeight << " m_curFps: " << m_curFps);
		webrtcEngine::CapturerTrackSource *capture = dynamic_cast<webrtcEngine::CapturerTrackSource *>(m_localVideoTrackSource.get());

		if (capture) {
			LOG_INFO("is captureVideoSource");
			return capture->setVideoFormat(m_curWidth, m_curHeight, m_curFps);
		}

		ExternalVideoFrameTrackSource *external = dynamic_cast<ExternalVideoFrameTrackSource*>(m_localVideoTrackSource.get());

		if (external) {
			LOG_INFO("is externalVideoSource");
			external->setVideoFormat(m_curWidth, m_curHeight, m_curFps);
			return true;
		}
		LOG_ERROR("return false");
		return false;
		});
	LOG_INFO("ret :" << ret);
	END_THREAD_CALL

	LOG_INFO("end");
	return true;
}

void RTCLivePusher::setVideoDeviceIndex(int videoDeviceIndex)
{
	LOG_INFO("videoDeviceIndex: " << videoDeviceIndex);
	m_curVideoDeviceIndex = videoDeviceIndex;
}

bool RTCLivePusher::setLocalDevice(const char* deviceName)
{
	if (!deviceName) {
		LOG_ERROR("deviceName is null ");
		return false;
	}
	LOG_INFO("deviceName: " << deviceName);
	
	if (m_isExternalVideoEnabled) {
		LOG_ERROR("m_isExternalVideoEnabled: " << m_isExternalVideoEnabled);
		return false;
	}

	if (!m_localVideoTrackSource) {
		if (!createVideoTrack(true)) {
			LOG_ERROR("createVideoTrack error!");
			return false;
		}
	}

	return g_signaling_thread->Invoke<bool>(RTC_FROM_HERE, [&] {
		if (!m_localVideoTrackSource)
			return false;
		webrtcEngine::CapturerTrackSource* capture = dynamic_cast<webrtcEngine::CapturerTrackSource*>(m_localVideoTrackSource.get());
		if (!capture) {
			LOG_ERROR("m_localVideoTrackSource is not a webrtcEngine::CapturerTrackSource");
			return false;
		}
		return capture->setDevice(deviceName);
	});
}

void RTCLivePusher::OnFrame(const uint8_t* data_y, const uint8_t* data_u, const uint8_t* data_v, const uint8_t* data_a, int stride_y, int stride_u, int stride_v, int stride_a, uint32_t width, uint32_t height, int64_t render_time)
{
	//planar yuv
	if (!m_uBuffer || m_curWidth != width || m_curHeight != height) {
		if (m_uBuffer)
			delete[] m_uBuffer;
		m_curWidth = width;  //width of video frame
		m_curHeight = height;  //height of video frame
		m_uBuffer = new uint8_t[width * height * 3 / 2];
	}
	int nYUVBufsize = 0;
	int nVOffset = 0;
	for (int i = 0; i < height; ++i) {
		memcpy(m_uBuffer + nYUVBufsize, data_y + i * stride_y, width);
		nYUVBufsize += width;
	}
	for (int i = 0; i < (height >> 1); ++i) {
		memcpy(m_uBuffer + nYUVBufsize, data_u + i * stride_u, width >> 1);
		nYUVBufsize += (width >> 1);
		memcpy(m_uBuffer + width * height * 5 / 4 + nVOffset, data_v + i * stride_v, width >> 1);
		nVOffset += (width >> 1);
	}

	if (m_eventHandler) {
		m_eventHandler->onCaptureVideoFrame((const char*)m_uBuffer, width, height, Color_YUVI420);
	}

	m_videoSink.renderYuv(m_uBuffer, width, height);

	if ((m_renderCount++) % (m_curFps * kSendSEISeconds) == 0) {
		sendMessage("11", 2);
	}

}

void RTCLivePusher::receiveAudioFrame(const void* audio_data, int bits_per_sample, int sample_rate, int number_of_channels, int number_of_frames)
{
	if (m_eventHandler) {
		m_eventHandler->onCaptureAudioFrame((const char*)audio_data,number_of_frames * bits_per_sample * number_of_channels ,number_of_channels,sample_rate);
	}
}

void RTCLivePusher::reGenPeerConnection()
{
	//要放在worker里调用
	LOG_INFO("begin!");
	deletePeerConnection();
	createPeerConnection();
	LOG_INFO("end!");
}

bool RTCLivePusher::initPeerConnection()
{
	LOG_INFO("begin!");
	if (g_peer_connection_factory == nullptr) {

		//rtc::LogMessage::LogToDebug(rtc::LoggingSeverity::LS_VERBOSE);
		rtc::LogMessage::SetLogToStderr(false);
		webrtc::field_trial::InitFieldTrialsFromString(s_webrtcRttMult);

		g_worker_thread = rtc::Thread::CreateWithSocketServer();
		g_worker_thread->Start();
		g_signaling_thread = rtc::Thread::Create();
		g_signaling_thread->Start();

		if (!webrtcEngine::AudioDeviceManager::instance()->init(
			g_worker_thread.get())) {
			return false;
		}

		g_peer_connection_factory = webrtc::CreatePeerConnectionFactory(
			g_worker_thread.get(), g_worker_thread.get(), g_signaling_thread.get(),
			webrtcEngine::AudioDeviceManager::instance()->getADM(),
			webrtc::CreateBuiltinAudioEncoderFactory(),
			webrtc::CreateBuiltinAudioDecoderFactory(),
			webrtc::CreateBuiltinVideoEncoderFactory(),
			webrtc::CreateBuiltinVideoDecoderFactory(), nullptr, nullptr);
	}

	if (!createPeerConnection()) {
		LOG_ERROR("createPeerConnection failed!");
	}

	LOG_INFO("end!");
	return true;
}

bool RTCLivePusher::createPeerConnection()
{
	LOG_INFO("begin!");
	RTC_DCHECK(g_peer_connection_factory.get() != nullptr);
	RTC_DCHECK(m_peerConnection.get() == nullptr);

	webrtc::PeerConnectionInterface::RTCConfiguration config_;

	// Add the stun server.
	webrtc::PeerConnectionInterface::IceServer stun_server;
	stun_server.uri = GetEnvVarOrDefault("WEBRTC_CONNECT", "stun:stun.l.google.com:19302");
	config_.servers.push_back(stun_server);
	config_.sdp_semantics = ::webrtc::SdpSemantics::kUnifiedPlan;
	config_.bundle_policy = ::webrtc::PeerConnectionInterface::kBundlePolicyMaxBundle;
	config_.rtcp_mux_policy = webrtc::PeerConnectionInterface::kRtcpMuxPolicyRequire;
	config_.type = webrtc::PeerConnectionInterface::kAll;
	config_.tcp_candidate_policy = webrtc::PeerConnectionInterface::TcpCandidatePolicy::kTcpCandidatePolicyEnabled;
	config_.enable_dtls_srtp = true;

	m_peerConnection = g_peer_connection_factory->CreatePeerConnection(config_, nullptr, nullptr, this);
	LOG_INFO("end!");
	return m_peerConnection.get() != nullptr;
}

void RTCLivePusher::deletePeerConnection()
{
	LOG_INFO("begin!");
	if (m_peerConnection) {
		LOG_INFO("come here");
		enableVideo(false);
		enableAudio(false);

		if (m_audioTransceiver) {
			m_audioTransceiver->Stop();
			m_audioTransceiver = nullptr;
		}
		if (m_videoTransceiver) {
			m_videoTransceiver->Stop();
			m_videoTransceiver = nullptr;
		}
		m_peerConnection->Close();
		m_peerConnection = nullptr;
	}
	LOG_INFO("end!");
}

bool RTCLivePusher::openLocalStream()
{
	LOG_INFO("begin!");
	if (m_localStream) {
		LOG_INFO("end!");
		return true;
	}

	m_localStream.reset(new MediaStreamImpl(rtc::CreateRandomUuid(), this));
	webrtcEngine::AudioDeviceManager::instance()->registerObserver(m_localStream.get());
	webrtcEngine::AudioDeviceManager::instance()->startPlayOut();

	m_localStream->RegisterOnI420Frame(I420FRAMEREADY_CALLBACK(videoDataCallback, this));
	m_localStream->RegisterOnAudioFrame(AUDIOBUSREADY_CALLBACK(audioDataCallback, this));

	LOG_INFO("end!");
	return true;
}


bool RTCLivePusher::openAudioSource()
{
	LOG_INFO("begin!");
	if (!m_localStream) {
		LOG_INFO("end! m_localStream is null");
		return false;
	}

	if (m_audioTrack) {
		LOG_INFO("end! m_audioTrack is not null");
		return true;
	}

	m_audioTrack = g_peer_connection_factory->CreateAudioTrack(
		g_kAudioLabel,
		g_peer_connection_factory->CreateAudioSource(cricket::AudioOptions()));

	if (m_audioTrack) {
		m_audioTrack->AddSink(m_localStream->audioObserver());
	}
	LOG_INFO("end! m_audioTrack: " << m_audioTrack);
	return m_audioTrack != nullptr;
}

bool RTCLivePusher::openVideoSource(bool capture)
{
	LOG_INFO("begin!");

	if (!m_localStream) {
		LOG_INFO("end! m_localStream is null");
		return false;
	}

	if (m_videoTrack && m_videoTransceiver) {
		enableVideo(false);
	}
	else {

		if (m_videoTrack)
			LOG_INFO("close videoTrack!");
		m_videoTrack = nullptr;

		if (m_localVideoTrackSource) {
			LOG_INFO("close localVideoTrackSource!");
			closeVideoSource();
		}
	}

	if (!createVideoTrack(capture)) {
		LOG_ERROR("createVideoTrack error!");
		return false;
	}

	if(m_videoTransceiver)
		enableVideo(true);
	LOG_INFO("end!");
	return true;
}

bool RTCLivePusher::createVideoTrack(bool capture)
{
	LOG_INFO("begin!");
	if (!m_localStream) {
		LOG_INFO("end! m_localStream is null");
		return false;
	}

	if (m_localVideoTrackSource) {
		LOG_INFO("end! m_localVideoTrackSource has create!");
		return true;
	}
		
	if (capture) {
		LOG_INFO("start capture: " << m_curWidth << "x" << m_curHeight << "  " << m_curFps);
	}

	webrtc::MethodCall1<RTCLivePusher, void, bool> callCreate(
		this, &RTCLivePusher::createVideoSource, capture);
	callCreate.Marshal(RTC_FROM_HERE, g_signaling_thread.get());
	
	if (!m_localVideoTrackSource) {
		LOG_INFO("end! createVcmVideoSource failed!");
		return false;
	}

	m_videoTrack = g_peer_connection_factory->CreateVideoTrack(
		rtc::CreateRandomUuid(), m_localVideoTrackSource);

	if (!m_videoTrack) {
		LOG_INFO("end! CreateVideoTrack failed!");
		return false;
	}
	m_videoTrack->AddOrUpdateSink(m_localStream->videoObserver(),
		rtc::VideoSinkWants());
	LOG_INFO("end!");
	return true;
}

void RTCLivePusher::createVideoSource(bool capture)
{
	LOG_INFO("begin! capture: " << capture);
	if (capture) {
		m_localVideoTrackSource = webrtcEngine::CapturerTrackSource::Create(
			m_curWidth, m_curHeight, m_curFps, m_curVideoDeviceIndex);
	}
	else {
		m_localVideoTrackSource = ExternalVideoFrameTrackSource::Create(m_curWidth, m_curHeight, m_curFps);
	}
	LOG_INFO("end!");
}

void RTCLivePusher::releaseVideoSource()
{
	LOG_INFO("begin!");
	m_localVideoTrackSource = nullptr;
	LOG_INFO("end!");
}

void RTCLivePusher::closeVideoSource()
{
	LOG_INFO("begin!");
	webrtc::MethodCall0<RTCLivePusher, void> callStop(
		this, &RTCLivePusher::releaseVideoSource);
	callStop.Marshal(RTC_FROM_HERE, g_signaling_thread.get());
	LOG_INFO("end!");
}

void RTCLivePusher::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state)
{

}

void RTCLivePusher::OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState new_state)
{
	LOG_INFO("new_state :" << (PEERCONNECTIONSTATE)new_state);
	switch (new_state)
	{
	case webrtc::PeerConnectionInterface::PeerConnectionState::kNew:
		break;
	case webrtc::PeerConnectionInterface::PeerConnectionState::kConnecting:
		break;
	case webrtc::PeerConnectionInterface::PeerConnectionState::kConnected:
		if (m_eventHandler) {
			m_eventHandler->onPusherEvent(RTCLIVE_CONNECTED);
		}
		break;
	case webrtc::PeerConnectionInterface::PeerConnectionState::kDisconnected:
		if (m_eventHandler) {
			m_eventHandler->onPusherEvent(RTCLIVE_DISCONNECT);
		}
		break;
	case webrtc::PeerConnectionInterface::PeerConnectionState::kFailed:
		break;
	case webrtc::PeerConnectionInterface::PeerConnectionState::kClosed:
		break;
	default:
		break;
	}

}

void RTCLivePusher::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state)
{
	LOG_INFO("new_state :" << new_state);
}

void RTCLivePusher::OnIceCandidate(const webrtc::IceCandidateInterface* candidate)
{
	std::string candidateStr;
	if (!candidate->ToString(&candidateStr)) {
		LOG_ERROR("Failed to serialize candidate");
		return;
	}
	LOG_INFO("candidateStr: " << candidateStr.c_str());
}

void RTCLivePusher::OnSuccess(webrtc::SessionDescriptionInterface* desc)
{
	{
		std::unique_lock<std::mutex> locker(m_mutexOffer);
		m_conOffer.notify_one();
		if (!desc->ToString(&m_offer)) {
			LOG_ERROR("error ToString");
			m_offer.clear();
			return;
		}
	}
	LOG_INFO("SetLocalDescription")
	m_peerConnection->SetLocalDescription(
		DummySetSessionDescriptionObserver::Create(), desc);

	LOG_INFO("success for " << desc->type() << " , sdp: " << m_offer);
}

void RTCLivePusher::OnFailure(const std::string& error)
{
	{
		std::unique_lock<std::mutex> locker(m_mutexOffer);
		m_conOffer.notify_one();
		m_offer.clear();
	}
	LOG_ERROR("error :" << error);
}
