/*
 *  Copyright 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef EXTERNALVIDEOFRAME_H
#define EXTERNALVIDEOFRAME_H


#include <list>
#include <memory>
#include <mutex>
#include <string>

#include "absl/memory/memory.h"
#include "api/video/video_source_interface.h"
#include "media/base/video_broadcaster.h"
#include "rtc_base/task_utils/repeating_task.h"

#include "pc/video_track_source.h"

#include "api/video/video_frame.h"
#include "rtc_base/time_utils.h"

#include <utility>

#include "absl/strings/string_view.h"
#include "api/task_queue/default_task_queue_factory.h"
#include "rtc_base/checks.h"
#include "rtc_base/event.h"
#include "rtc_base/task_queue.h"
#include "rtc_base/task_utils/to_queued_task.h"
#include "rtc_base/thread_annotations.h"

#include "VcmCapturer.h"


struct YuvI420VideoFrame
{
	/** Video pixel width.
	 */
	int width;  // width of video frame
	/** Video pixel height.
	 */
	int height;  // height of video frame
	/** Line span of the Y buffer within the YUV data.
	 */
	int yStride;  // stride of Y data buffer
	/** Line span of the U buffer within the YUV data.
	 */
	int uStride;  // stride of U data buffer
	/** Line span of the V buffer within the YUV data.
	 */
	int vStride;  // stride of V data buffer
	/** Pointer to the Y buffer pointer within the YUV data.
	 */
	void* yBuffer;  // Y data buffer
	/** Pointer to the U buffer pointer within the YUV data.
	 */
	void* uBuffer;  // U data buffer
	/** Pointer to the V buffer pointer within the YUV data.
	 */
	void* vBuffer;  // V data buffer
	/** Set the rotation of this frame before rendering the video. Supports 0, 90,
	 * 180, 270 degrees clockwise.
	 */
	int rotation;  // rotation of this frame (0, 90, 180, 270)
	/** Timestamp (ms) for the video stream render. Use this timestamp to
	synchronize the video stream render while rendering the video streams.

	@note This timestamp is for rendering the video stream, and not for capturing
	the video stream.  just use current time
	*/
	int64_t renderTimeMs;
};

/// only support yuvI420 video format.
typedef YuvI420VideoFrame ExternalVideoFrame;
webrtc::VideoFrame ExternalVideo2VideoFrame(ExternalVideoFrame* vf)
{
	int width = vf->width;
	int height = vf->height;
	int half_width = (width + 1) / 2;
	rtc::scoped_refptr<webrtc::I420Buffer> buffer(
		// Explicit stride, no padding between rows.
		webrtc::I420Buffer::Create(width, height, width, half_width, half_width));
	size_t size_y = static_cast<size_t>(width) * height;
	size_t size_uv = static_cast<size_t>(half_width) * ((height + 1) / 2);

	memcpy(buffer->MutableDataY(), vf->yBuffer, size_y);
	memcpy(buffer->MutableDataU(), vf->uBuffer, size_uv);
	memcpy(buffer->MutableDataV(), vf->vBuffer, size_uv);

	return webrtc::VideoFrame::Builder()
		.set_video_frame_buffer(buffer)
		.set_rotation((webrtc::VideoRotation)vf->rotation)
		.set_timestamp_us(vf->renderTimeMs * 1000)
		.build();
}

// reference webrtc::test::FakePeriodicExteralVideoSource
class PeriodicExteralVideoSource final
    : public webrtc::CustomVideoCapturer
{
 public:
	 PeriodicExteralVideoSource(int width,int height,int fps) {
		 CustomVideoCapturer::SetTargetWidthandHeight(width, height);
	 }
	 void pushExternalVideoFrame(ExternalVideoFrame* vf) 
	 {
		webrtc::VideoFrame frame = ExternalVideo2VideoFrame(vf);
		frame.set_ntp_time_ms(0);
		CustomVideoCapturer::OnFrame(frame);
	 }

	 void setVideoFormat(int width, int height,int fps) {
		 CustomVideoCapturer::SetTargetWidthandHeight(width, height);
	 }
};

// reference webrtc::test::FakePeriodicVideoTrackSource
// A ExternalVideoFrameTrackSource support user push video frame outside.
class ExternalVideoFrameTrackSource : public webrtc::VideoTrackSource
{
public:
	explicit ExternalVideoFrameTrackSource(int width, int height, int fps) :VideoTrackSource(false), m_source(width, height, fps) {}
	~ExternalVideoFrameTrackSource() = default;

	static rtc::scoped_refptr<ExternalVideoFrameTrackSource> Create(int width, int height, int fps) {
		return new rtc::RefCountedObject<ExternalVideoFrameTrackSource>(width, height, fps);
	}

	void pushExternalVideoFrame(ExternalVideoFrame* vf) {
		m_source.pushExternalVideoFrame(vf);
	}

	void setVideoFormat(int width, int height, int fps) {
		m_source.setVideoFormat(width, height, fps);
	}

	bool sendSEIMessage(const char* msg, uint32_t len) {
		return m_source.SendSEIMessage(msg, len);
	}

 protected:
  rtc::VideoSourceInterface<webrtc::VideoFrame>* source() override { return &m_source; }

 private:
  PeriodicExteralVideoSource m_source;
};

#endif