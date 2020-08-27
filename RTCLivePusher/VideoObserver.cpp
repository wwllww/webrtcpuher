/*
 *  Copyright (c) 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include "api/video/i420_buffer.h"
#include "VideoObserver.h"

void VideoObserver::SetVideoCallback(I420FRAMEREADY_CALLBACK callback) 
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_onI420FrameReady = callback;
}


void VideoObserver::OnFrame(const webrtc::VideoFrame& frame) 
{
  std::unique_lock<std::mutex> lock(m_mutex);
  if (!m_onI420FrameReady)
    return;

  rtc::scoped_refptr<webrtc::VideoFrameBuffer> buffer(frame.video_frame_buffer());
  /// referece from peeConnection
  if (frame.rotation() != webrtc::kVideoRotation_0) {
    buffer = webrtc::I420Buffer::Rotate(*buffer, frame.rotation());
  }

  if (buffer->type() != webrtc::VideoFrameBuffer::Type::kI420A) {
    rtc::scoped_refptr<webrtc::I420BufferInterface> i420_buffer =
        buffer->ToI420();
    m_onI420FrameReady(i420_buffer->DataY(), i420_buffer->DataU(),
                     i420_buffer->DataV(), nullptr, i420_buffer->StrideY(),
                     i420_buffer->StrideU(), i420_buffer->StrideV(), 0,
                     i420_buffer->width(), i420_buffer->height(), frame.timestamp());

  } else {
    // The buffer has alpha channel.
    webrtc::I420ABufferInterface* i420a_buffer = const_cast<webrtc::I420ABufferInterface*>(buffer->GetI420A());

    m_onI420FrameReady(i420a_buffer->DataY(), i420a_buffer->DataU(),
                     i420a_buffer->DataV(), i420a_buffer->DataA(),
                     i420a_buffer->StrideY(), i420a_buffer->StrideU(),
                     i420a_buffer->StrideV(), i420a_buffer->StrideA(), 
		             i420a_buffer->width(), i420a_buffer->height(), frame.timestamp());
  }
}

void SEIObserver::SetSEICallback(SEI_CALLBACK callback) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_onSEIDataCallback = callback;
}

void SEIObserver::onSEIData(const char* data, uint32_t len) {
  std::unique_lock<std::mutex> lock(m_mutex);
  if (!m_onSEIDataCallback)
    return;
  m_onSEIDataCallback(data, len);
}
