#ifndef IRTCLIVEPUSHER_H
#define IRTCLIVEPUSHER_H
#include <map>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <windef.h>
typedef HWND WindowIdType;
#ifdef RTCPUSHER_EXPORTS
# define RTCLIVEPUSHER_EXPORT __declspec(dllexport)
#else
# define RTCLIVEPUSHER_EXPORT __declspec(dllimport)
#endif
#elif defined(__APPLE__)
typedef void* WindowIdType;
#define RTCLIVEPUSHER_EXPORT
#endif

enum ColorSpace
{
	Color_YUVI420 = 0,
	Color_RGB24,
	Color_RGB32
};

enum RTCLiveEvent
{
	RTCLIVE_IDLE = 0,
	RTCLIVE_CONNECTING,
	RTCLIVE_CONNECTED,
	RTCLIVE_DISCONNECT,
	RTCLIVE_RECONNECTING,
	RTCLIVE_FAILED
};

struct RTCPushConfig
{
	int width;//推流宽
	int height;//推流高
	int bitrate;//码率 kbps
	int fps;//推流帧率

	RTCPushConfig() {
		width = 1280;
		height = 720;
		bitrate = 500;
		fps = 15;
	}
};

class IRTCLivePusherEvent
{
public:
	virtual ~IRTCLivePusherEvent() {}

	virtual void onPusherEvent(RTCLiveEvent event) {}
	/** 
	 * @brief key {"Fps","AudioKBitRate","VideoKBitRate","cpuAppUsage","cpuAppUsage"}
	 */
	virtual void onPusherStats(const std::map<const char*,double>& stats) {}
	virtual void onCaptureVideoFrame(const char* data,int width,int height,ColorSpace color) {}
	virtual void onCaptureAudioFrame(const char* data, int len,int channel,int samplesPerSec) {}
};

/*！
	@brief 该回调函数用于获取当时日志
	@param msg 日志信息
	@param object 接收回调函数的对象的指针，set时传入，回调时传出
*/
typedef void(*log_callback)(const char* msg, void* object);


class IVideoDeviceManager; //视频采集设备管理
class IPlayoutManager; //音频播放设备管理
class IMicManager; //音频采集设备管理

class IRTCLivePusher
{
public:
	virtual ~IRTCLivePusher(){}
	/////////////////////////////////////////////////////////////////////////////////
	//                      （一）设备管理
	/////////////////////////////////////////////////////////////////////////////////
	/// @name 设备管理
	/// @{
	/**@brief 视频设备管理
	*/
	virtual IVideoDeviceManager* getVideoDeviceManager() = 0;
	/**@brief 播放器设备管理
	*/
	virtual IPlayoutManager* getPlayoutManager() = 0;
	/**@brief 麦克（录音）设备管理
	*/
	virtual IMicManager* getMicManager() = 0;
	virtual void setPushParam(const RTCPushConfig& config) = 0;
	virtual int registerRTCLivePushEventHandler(IRTCLivePusherEvent *eventHandler) = 0;
	virtual int startPreview(WindowIdType winId) = 0;
	virtual void stopPreview() = 0;
	virtual int startPush(const char* posturl,const char* streamId) = 0;
	virtual void stopPush() = 0;
	virtual int setMirror(bool isMirror) = 0;
	virtual int muteAudio(bool mute) = 0;
	virtual int muteVideo(bool mute) = 0;
	virtual int enableAudioVolumeEvaluation(int interval) = 0;
	virtual void sendVideoBuffer(const char* data, int width, int height, ColorSpace color) = 0;
	virtual void sendAudioBuff(const char*data, int len, int channel, int samplesPerSec) = 0;
	virtual int enableVideoCapture(bool enable) = 0;
	virtual int enableAudioCapture(bool enable) = 0;
	virtual bool enableExternalVideoSource(bool enable) = 0;
	virtual bool enableExternalAudioSource(bool enable) = 0;
	virtual int sendMessage(const char* msg, int len) = 0;
};


class IVideoDeviceManager {
public:
	virtual ~IVideoDeviceManager() {}

	/**@brief 获取摄像头数量
	* @return 返回摄像头数量
	*/
	virtual unsigned getDeviceCount() = 0;

	/**@brief 获取当前使用的摄像头设备Id
	* @devceId 当前摄像头的设备ID
	* @return true：找打当前使用的摄像头； false：没有找到当前使用的摄像头
	*/
	virtual bool getCurDeviceID(char* deviceId) = 0;
	/**@brief 设置当前使用的摄像头
	* @param devideIndex 摄像头的编号（从0开始， @see IVideoDeviceManager::getDevice）
	* @return 是否成功设置了
	*/
	virtual bool setCurDevice(unsigned deviceIndex) = 0;
	/**@brief 设置当前使用的摄像头
	* @param devideId 摄像头的Id
	* @return 是否成功设置了
	*/
	virtual bool setCurDeviceID(const char* deviceId) = 0;

	/**@brief 获取摄像头信息
	* @param nIndex 摄像头的编号（从0开始）
	* @param deviceName 摄像头的名字
	* @param deviceID 摄像头的ID
	* @return 是否成功找到摄像头
	*/
	virtual bool getDevice(unsigned nIndex, char* deviceName, char* deviceID) = 0;
};


class IPlayoutManager
{
public:
	virtual ~IPlayoutManager() {};

	/**@brief 获取当前使用的麦克音量 [0~255]
	* @return 返回当前使用的麦克音量, 如果无法获取，则返回-1
	*/
	virtual int getVolume() = 0;
	/**@brief 设置当前使用的播放器音量
	* @param nVol 音量值  [0~255]
	* @return 是否成功设置了当前使用的播放器音量
	*/
	virtual bool setVolume(unsigned nVol) = 0;

	/**@brief 获取播放器数量
	* @return 返回播放器数量
	*/
	virtual unsigned getDeviceCount() = 0;

	/**@brief 获取当前使用的播放器设备Id
	* @devceId 当前播放器的设备ID
	* @return true：找打当前使用的播放器； false：没有找到当前使用的播放器
	*/
	virtual bool getCurDeviceID(char* deviceId) = 0;
	/**@brief 设置当前使用的播放器
	* @param devideIndex 播放器的编号（从0开始， @see IPlayoutManager::getDevice）
	* @return 是否成功设置了
	*/
	virtual bool setCurDevice(unsigned deviceIndex) = 0;
	/**@brief 设置当前使用的播放器
	* @param devideId 播放器的Id
	* @return 是否成功设置了
	*/
	virtual bool setCurDeviceID(const char* deviceId) = 0;

	/**@brief 获取播放器信息
	* @param nIndex 播放器的编号（从0开始）
	* @param deviceName 播放器的名字
	* @param deviceID 播放器的ID
	* @return 是否成功找到播放器
	*/
	virtual bool getDevice(unsigned nIndex, char* deviceName, char* deviceID) = 0;
};

class IMicManager
{
public:
	virtual ~IMicManager() {};

	/**@brief 获取当前使用的麦克音量 [0~255]
	* @return 返回当前使用的麦克音量, 如果无法获取，则返回-1
	*/
	virtual int getVolume() = 0;
	/**@brief 设置当前使用的麦克音量
	* @param nVol 音量值 [0~255]
	* @return 是否成功设置了当前使用的麦克音量
	*/
	virtual bool setVolume(unsigned nVol) = 0;

	/**@brief 获取麦克数量
	* @return 返回麦克数量
	*/
	virtual unsigned getDeviceCount() = 0;

	/**@brief 获取当前使用的麦克设备Id
	* @devceId 当前麦克的设备ID
	* @return true：找打当前使用的麦克； false：没有找到当前使用的麦克
	*/
	virtual bool getCurDeviceID(char* deviceId) = 0;
	/**@brief 设置当前使用的麦克
	* @param devideIndex 麦克的编号（从0开始， @see IMicManager::getDevice）
	* @return 是否成功设置了
	*/
	virtual bool setCurDevice(unsigned deviceIndex) = 0;
	/**@brief 设置当前使用的麦克
	* @param devideId 麦克的Id
	* @return 是否成功设置了
	*/
	virtual bool setCurDeviceID(const char* deviceId) = 0;

	/**@brief 获取麦克信息
	* @param nIndex 麦克的编号（从0开始）
	* @param deviceName 麦克的名字
	* @param deviceID 麦克的ID
	* @return 是否成功找到麦克
	*/
	virtual bool getDevice(unsigned nIndex, char* deviceName, char* deviceID) = 0;
};

#ifdef  __cplusplus
extern "C" {
#endif //  __cplusplus

	/*!
		@brief 该函数用来获取SDK实例，多次调用会生成多个
		@return 实例指针
	*/
	RTCLIVEPUSHER_EXPORT IRTCLivePusher* createRTCLivePusher();

	/*
		@brief 该函数用来销毁SDK实例，当使用完成通过createRTCMediaPlayer获取的SDK实例
		@param rtcMediaplayer 获取实例指针的地址
		@note  使用该方法销毁，createRTCMediaPlayer应该和destroyRTCMediaPlayer成对出现
	*/
	RTCLIVEPUSHER_EXPORT void destroyRTCLivePusher(IRTCLivePusher* rtcLivePusher);

	/*!
		@brief 该函数用来注册日志回调函数
	*/
	RTCLIVEPUSHER_EXPORT void  rtcRegisterLogFunc(log_callback fn, void *object = nullptr);

#ifdef  __cplusplus
}
#endif //  __cplusplus
#endif