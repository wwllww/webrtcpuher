
#ifndef VCMCAPTURER_H
#define VCMCAPTURER_H

#include <memory>
#include <vector>

#include "api/scoped_refptr.h"
#include "modules/video_capture/video_capture.h"

#include "modules/video_capture/video_capture_factory.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"


#include "api/video/video_frame.h"
#include "api/video/video_source_interface.h"
#include "media/base/video_adapter.h"
#include "media/base/video_broadcaster.h"

#include <algorithm>

#include "api/scoped_refptr.h"
#include "api/video/i420_buffer.h"
#include "api/video/video_frame_buffer.h"
#include "api/video/video_rotation.h"

namespace webrtc 
{
// reference from webrtc::TestVideoCapturer
class CustomVideoCapturer : public rtc::VideoSourceInterface<VideoFrame> 
{
 public:
  CustomVideoCapturer() = default;
  ~CustomVideoCapturer() override = default;
  
    
  void AddOrUpdateSink(
      rtc::VideoSinkInterface<VideoFrame>* sink,
      const rtc::VideoSinkWants& wants) override 
  {
    m_broadcaster.AddOrUpdateSink(sink, wants);
    UpdateVideoAdapter();
  }

  void RemoveSink(rtc::VideoSinkInterface<VideoFrame>* sink) override 
  {
    m_broadcaster.RemoveSink(sink);
    UpdateVideoAdapter();
  }
  
  bool SendSEIMessage(const char* msg, uint32_t len) {
	 return m_broadcaster.SendSEIMessage(msg, len);
  }

 protected:
  void OnFrame(const VideoFrame& frame) 
  {
    int cropped_width = 0;
    int cropped_height = 0;
    int out_width = 0;
    int out_height = 0;

    if (!m_videoAdapter.AdaptFrameResolution(
            frame.width(), frame.height(), frame.timestamp_us() * 1000,
            &cropped_width, &cropped_height, &out_width, &out_height)) 
	{
      // Drop frame in order to respect frame rate constraint.
      return;
    }
    out_width = m_target_width;
    out_height = m_target_height;
    if (out_height != frame.height() || out_width != frame.width()) {
      // Video adapter has requested a down-scale. Allocate a new buffer and
      // return scaled version.
      rtc::scoped_refptr<I420Buffer> scaled_buffer =
          I420Buffer::Create(out_width, out_height);
      if(out_height * frame.width() != out_width *frame.height())
          scaled_buffer->CropAndScaleFrom(*frame.video_frame_buffer()->ToI420());
      else
          scaled_buffer->ScaleFrom(*frame.video_frame_buffer()->ToI420());
        
      m_broadcaster.OnFrame(VideoFrame::Builder()
                               .set_video_frame_buffer(scaled_buffer)
                               .set_rotation(kVideoRotation_0)
                               .set_timestamp_us(frame.timestamp_us())
                               .set_id(frame.id())
                               .build());
    } else {
      // No adaptations needed, just return the frame as is.
      m_broadcaster.OnFrame(frame);
    }
  }

  rtc::VideoSinkWants GetSinkWants() 
  {
    return m_broadcaster.wants();
  }

  // private:  
  void UpdateVideoAdapter()
  {
    rtc::VideoSinkWants wants = m_broadcaster.wants();
    m_videoAdapter.OnResolutionFramerateRequest(wants.target_pixel_count,
                                                wants.max_pixel_count,
                                                wants.max_framerate_fps);
  }
    void SetTargetWidthandHeight(int target_width,int target_height) {
        m_target_width = target_width;
        m_target_height = target_height;
    }
  rtc::VideoBroadcaster m_broadcaster;
  cricket::VideoAdapter m_videoAdapter;
  int m_target_width = 320;
  int m_target_height = 240;
};



class VcmCapturer : public CustomVideoCapturer,
                    public rtc::VideoSinkInterface<VideoFrame> 
{
 public:
  static VcmCapturer* Create(size_t width,
                                   size_t height,
                                   size_t target_fps,
                                   size_t capture_device_index) 
  {
    std::unique_ptr<VcmCapturer> vcm_capturer(new VcmCapturer());
    if (!vcm_capturer->Init(width, height, target_fps, capture_device_index)) {
      RTC_LOG(LS_WARNING) << "Failed to create VcmCapturer(w = " << width
                          << ", h = " << height << ", fps = " << target_fps
                          << ")";
      return nullptr;
    }
    return vcm_capturer.release();
  }

  
  virtual ~VcmCapturer() { Destroy(); }

  void OnFrame(const VideoFrame& frame) override  
  {
    CustomVideoCapturer::OnFrame(frame);
  }

  cricket::VideoAdapter& videoAdapter() { return m_videoAdapter; }

  bool setDevice(const char* deviceName) 
  { 
	  if (!deviceName || strcmp(deviceName,"") == 0)
			return false;

	  std::unique_ptr<VideoCaptureModule::DeviceInfo> device_info(
              VideoCaptureFactory::CreateDeviceInfo());

	  if (!device_info)
            return false;

      Destroy();
      m_deviceName = deviceName;
      m_vcm = webrtc::VideoCaptureFactory::Create(deviceName);
      if (!m_vcm) {
        return false;
      }
      m_vcm->RegisterCaptureDataCallback(this);

      if (m_vcm->StartCapture(m_capability) != 0) {
        Destroy();
        return false;
       }
      return true;
  }

  bool setVideoFormat(int width, int height, int fps) 
  {
    int suitable_width = width;
    int suitable_height = height;
    SetTargetWidthandHeight(width,height);
      
    if (m_capability.maxFPS != fps || m_capability.width != width || m_capability.height != height) {

      std::unique_ptr<VideoCaptureModule::DeviceInfo> device_info(
          VideoCaptureFactory::CreateDeviceInfo());

      if (!device_info)
		  return false;

      int numOfCap = device_info->NumberOfCapabilities(m_deviceName.c_str());
      
      int pre_width = 0;
      int pre_height = 0;
      for (int i = 0; i < numOfCap; ++i) {
        VideoCaptureCapability capability;
        if (0 == device_info->GetCapability(m_deviceName.c_str(), i, capability)) {
            
          if (width * capability.height == capability.width * height) {
			  m_capability.maxFPS = fps;
              if(width == capability.width) {
                  suitable_width = width;
                  suitable_height = height;
                  break;
              } else {
                  if(!pre_width || !pre_height) {
                      pre_width = capability.width;
                      pre_height = capability.height;
                  } else {
                      if((capability.width * capability.height < pre_width * pre_height &&
                         width * height < capability.width * capability.height) ||
                         (fabs(capability.width - width) <= fabs(pre_width - width)
                          && capability.width > pre_width)) {
                          pre_width = capability.width;
                          pre_height = capability.height;
                      }
                  }
                  
                  suitable_width = pre_width;
                  suitable_height = pre_height;
              }
		  }
		}
      }
	}

	if (m_capability.maxFPS != fps && m_capability.width == width &&
        m_capability.height == height) {
          return true;//do not support fps,so return true
	}
#ifdef WEBRTC_MAC
      if(suitable_width < 960 || suitable_height < 540) {
          suitable_width = 960;
          suitable_height = 540;
      }
#endif
	m_capability.width = static_cast<int32_t>(suitable_width);
	m_capability.height = static_cast<int32_t>(suitable_height);
	m_capability.videoType = VideoType::kI420;
	return setDevice(m_deviceName.c_str());
  }
  
  bool SendSEIMessage(const char* msg, uint32_t len) override {
	  return CustomVideoCapturer::SendSEIMessage(msg, len);
  }

 private:
  VcmCapturer() : m_vcm(nullptr) {}

  bool Init(size_t width,
                         size_t height,
                         size_t target_fps,
                         size_t capture_device_index) 
  {
    SetTargetWidthandHeight(width,height);
    
    std::unique_ptr<VideoCaptureModule::DeviceInfo> device_info(
        VideoCaptureFactory::CreateDeviceInfo());

    if (!device_info) {
        return false;
    }

    if (device_info->NumberOfDevices() == 0)
        return false;

	if (capture_device_index >= device_info->NumberOfDevices()) {
        capture_device_index = 0;
	}

    char device_name[256];
    char unique_name[256];
    if (device_info->GetDeviceName(static_cast<uint32_t>(capture_device_index),
                                   device_name, sizeof(device_name),
                                   unique_name, sizeof(unique_name)) != 0) {
      Destroy();
      return false;
    }
    m_deviceName = unique_name;
    m_vcm = webrtc::VideoCaptureFactory::Create(unique_name);
    if (!m_vcm) {
      return false;
    }
    m_vcm->RegisterCaptureDataCallback(this);

    device_info->GetCapability(m_vcm->CurrentDeviceName(), 0, m_capability);
    
    int numOfCap = device_info->NumberOfCapabilities(m_deviceName.c_str());
    int suitable_width = width;
    int suitable_height = height;
    int pre_width = 0;
    int pre_height = 0;
    for (int i = 0; i < numOfCap; ++i) {
        VideoCaptureCapability capability;
        if (!device_info->GetCapability(m_deviceName.c_str(), i, capability)) {
          if (width * capability.height == capability.width * height) {
              if(width == capability.width) {
                  suitable_width = width;
                  suitable_height = height;
                  break;
              } else {
                  if(!pre_width || !pre_height) {
                      pre_width = capability.width;
                      pre_height = capability.height;
                  } else {
                     if((capability.width * capability.height < pre_width * pre_height &&
                         (int)(width * height) < capability.width * capability.height) ||
                         (fabs(capability.width - width) <= fabs(pre_width - width)
                          && capability.width > pre_width)) {
                          pre_width = capability.width;
                          pre_height = capability.height;
                      }
                  }
                  
                  suitable_width = pre_width;
                  suitable_height = pre_height;
              }
          }
        }
    }
#ifdef WEBRTC_MAC
    if(suitable_width < 960 || suitable_height < 540) {
        suitable_width = 960;
        suitable_height = 540;
    }
#endif
    m_capability.width = static_cast<int32_t>(suitable_width);
    m_capability.height = static_cast<int32_t>(suitable_height);
    m_capability.maxFPS = static_cast<int32_t>(target_fps);
    m_capability.videoType = VideoType::kI420;
    
    if (m_vcm->StartCapture(m_capability) != 0) {
      Destroy();
      return false;
    }

    //assert(m_vcm->CaptureStarted());

    return true;
  }
  
  
  void Destroy() 
  {
    if (!m_vcm)
      return;

    m_vcm->StopCapture();
    m_vcm->DeRegisterCaptureDataCallback();
    // Release reference to VCM.
    m_vcm = nullptr;
  }
 

  rtc::scoped_refptr<VideoCaptureModule> m_vcm;
  VideoCaptureCapability m_capability;
  std::string m_deviceName;
};


}  // namespace webrtcEngine 
#endif  // !VCMCAPTURER_H
