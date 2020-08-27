#include <algorithm>


#if defined(__APPLE__)
#include "../util/MacUtil.h"
#endif
#include "RtcLogWrite.h"
#include "SDLRenderer.h"


#define MIN_(a,b)            (((a) < (b)) ? (a) : (b))
#define MAX_(a,b)            (((a) > (b)) ? (a) : (b))

SDLRenderer::SDLRenderer(IMAGE_TYPE type)
	: m_imageType(type)
	, m_window(nullptr)
	, m_renderer(nullptr)
	, m_texture(nullptr)
	, m_hwnd(nullptr)
{

}

SDLRenderer::~SDLRenderer()
{
	clean();
}


void SDLRenderer::setMirror(bool useMirror)
{
	m_useMirror = useMirror;
}


bool SDLRenderer::init(HWND hwnd)
{
	static int init = SDL_Init(SDL_INIT_VIDEO);
	if (init) {
		LOG_INFO("Could not initialize SDL : %s" << SDL_GetError());
		return false;
	}

	if (hwnd != nullptr && m_hwnd == hwnd) {
		LOG_INFO("init with the same hwnd")
			return true;
	}
	else if (m_hwnd) {
		clean();
	}

	m_hwnd = hwnd;
#if defined(__APPLE__)
    m_window = SDL_CreateWindowFrom(macutil::windowFromView(hwnd));
#else
	m_window = SDL_CreateWindowFrom(hwnd);
#endif
        
	if (!m_window) {
		LOG_INFO("SDL CreateWindowFrom failed : " << SDL_GetError());
		return false;
	}

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_SOFTWARE);//SDL_RENDERER_ACCELERATED);

	if (!m_renderer) {
		LOG_INFO("Use hardware render failed, retry soft render.");
		m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_SOFTWARE);
		if (nullptr == m_renderer) {
			LOG_INFO("Use soft render failed, cannot render on this computer.");
			clean();
			return false;
		}
		else {
			SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
		}
	}
	else {
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	}
	//SDL_SetHint(SDL_HINT_RENDER_LOGICAL_SIZE_MODE, "0");
		

	return true;
}

void SDLRenderer::loadYuv(const uint8_t* Y, const uint8_t* U, const uint8_t* V, int width, int height)
{		
	loadImage(Y, width, height, width);		
	//SDL_UpdateYUVTexture(m_texture, nullptr, Y, width, U, width / 2, V, width /2);		
}

void SDLRenderer::loadImage(const uint8_t *data, int width, int height, int pitch)	
{
	if (!data || width <= 0 || height <= 0) {
		return;
	}

	if (!m_renderer || !m_window) {
		return;
	}

	if (width != m_lastWidth || height != m_lastHeight) {
		if (!initTexture(width, height)) {
			clean();
			return;
		}
	}

    if (!m_texture) {
		return;
	}

	SDL_RenderClear(m_renderer);

	SDL_UpdateTexture(m_texture, nullptr, data, pitch);

	int winWidth, winHeight;

	    
	SDL_GetWindowSize(m_window, &winWidth, &winHeight);

	float wScale = (float)winWidth / (float)m_lastWidth;
	float hScale = (float)winHeight / (float)m_lastHeight;		
	SDL_Rect winRect, imageRect;
	SDL_Rect* ptrWinRect = nullptr;
	SDL_Rect* ptrImageRect = nullptr;

	float r = MIN_(wScale, hScale);
	// clip window to fit image
	winRect.h = r * m_lastHeight;
	winRect.w = r * m_lastWidth;
	winRect.x = (int)(winWidth - winRect.w) / 2;
	winRect.y = (int)(winHeight - winRect.h) / 2;
	ptrWinRect = &winRect;
	
	SDL_RenderCopyEx(m_renderer, m_texture, ptrImageRect, ptrWinRect, 0, nullptr, m_useMirror ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);

	SDL_RenderPresent(m_renderer);
}

void SDLRenderer::clean()
{		
	if (m_texture) {
		SDL_DestroyTexture(m_texture);
		m_texture = nullptr;
	}

	if (m_renderer) {
		SDL_DestroyRenderer(m_renderer);
		m_renderer = nullptr;
	}

	if (m_window) {
		SDL_DestroyWindow(m_window);
		m_window = nullptr;
	}
}

bool SDLRenderer::initTexture(int w, int h)
{
	if (w <= 0 || h <= 0) {
		return false;
	}

	if (m_texture) {
		SDL_DestroyTexture(m_texture);
	}

	Uint32 format = SDL_PIXELFORMAT_IYUV;
	if (m_imageType == IMAGE_TYPE_RGB) {
		format = SDL_PIXELFORMAT_RGB24;
	}
	m_texture = SDL_CreateTexture(m_renderer, format, SDL_TEXTUREACCESS_STREAMING, w, h);
	if (!m_texture) {
		clean();
		return false;
	}

	m_lastWidth = w;
	m_lastHeight = h;
				
	return true;
}		


void SDLRenderer::onRefresh()
{
	if (m_texture && m_renderer && m_window) {
		SDL_RenderPresent(m_renderer);
	}
}



