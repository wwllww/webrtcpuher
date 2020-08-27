#include "RTCVideoSink.h"
#include "SDLRenderer.h"
#include "RtcLogWrite.h"

RtcVideoSink::RtcVideoSink()
{
	LOG_INFO("this :" << this);
	m_sdkRenderer = new SDLRenderer();
}

RtcVideoSink::~RtcVideoSink()
{
	LOG_INFO(" ~this :" << this);
	std::lock_guard<std::mutex> locker(m_mutexObserver);
	if (m_sdkRenderer) {
		delete m_sdkRenderer;
		m_sdkRenderer = nullptr;
	}
}

void RtcVideoSink::StartRenderer()
{
	m_startRender = true;
}

void RtcVideoSink::StopRenderer()
{
	m_startRender = false;
}

bool RtcVideoSink::setVideoWindow(WindowIdType wnd)
{
	LOG_INFO("begin,this :" << this << " m_sdkRenderer: " << m_sdkRenderer);
	std::lock_guard<std::mutex> locker(m_mutexObserver);
	if (m_sdkRenderer) {
		if (!m_sdkRenderer->init(wnd)) {
			LOG_ERROR("end,m_sdkRenderer init failed!");
			return false;
		}
	}
	LOG_INFO("end");
	return true;
}

void RtcVideoSink::setMirror(bool useMirror)
{
	LOG_INFO("useMirror: " << useMirror);
	if (m_sdkRenderer) {
		m_sdkRenderer->setMirror(useMirror);
	}
}

bool RtcVideoSink::renderYuv(uint8_t *data, int width, int height)
{
	std::lock_guard<std::mutex> locker(m_mutexObserver);
	if (!m_sdkRenderer) {
		LOG_ERROR("m_sdkRenderer is null");
		return false;
	}
	if(m_startRender)
		m_sdkRenderer->loadYuv(data, data + width * height, data + width * height * 5 / 4, width, height);
	return true;
}
