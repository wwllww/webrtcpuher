
#ifndef CAPTURERTRACKSOURCE_H
#define CAPTURERTRACKSOURCE_H


#include <memory>
#include <string>
#include <vector>

#include "pc/video_track_source.h"
#include "VcmCapturer.h"

namespace webrtcEngine 
{
		
class CapturerTrackSource : public webrtc::VideoTrackSource 
{
 public:
	 static const size_t kWidth = 640;
	 static const size_t kHeight = 480;
	 static const size_t kFps = 30;
	 static const size_t kDeviceIndex = 0;
  static rtc::scoped_refptr<CapturerTrackSource> Create(size_t width = kWidth,
														size_t height = kHeight,
														size_t fps = kFps,
														size_t deviceIndex = kDeviceIndex) 
  {    
    std::unique_ptr<webrtc::VcmCapturer> capturer = absl::WrapUnique(
        webrtc::VcmCapturer::Create(width, height, fps, deviceIndex));
    if (!capturer) {
      return nullptr;
    }
    return new rtc::RefCountedObject<CapturerTrackSource>(std::move(capturer));
  }

  cricket::VideoAdapter& video_adapter() { return m_capturer->videoAdapter(); }

  bool setDevice(const char* deviceName) {
    return m_capturer->setDevice(deviceName);
  }

  bool setVideoFormat(int width, int height,int fps) {
    return m_capturer->setVideoFormat(width,height,fps);
  }

  bool sendSEIMessage(const char* msg, uint32_t len) {
	return m_capturer->SendSEIMessage(msg, len);
  }

 protected:
  explicit CapturerTrackSource(
      std::unique_ptr<webrtc::VcmCapturer> capturer)
      : VideoTrackSource(/*remote=*/false), m_capturer(std::move(capturer)) {}

 private:
  rtc::VideoSourceInterface<webrtc::VideoFrame>* source() override 
  {
    return m_capturer.get();
  }
  std::unique_ptr<webrtc::VcmCapturer> m_capturer;
};


}  // namespace webrtcEngine
#endif
