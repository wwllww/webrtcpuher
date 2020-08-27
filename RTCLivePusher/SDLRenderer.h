#ifndef SDLRENDERER_H
#define SDLRENDERER_H

#include "VideoRenderer.h"

#if defined(__APPLE__)
#include "SDL2/SDL.h"
#else
#include "SDL.h"
#endif

class SDLRenderer : public IRenderer
{
public:
	enum IMAGE_TYPE {
		IMAGE_TYPE_RGB = 1,
		IMAGE_TYPE_YUV = 2
	};
	explicit SDLRenderer(IMAGE_TYPE type = IMAGE_TYPE_YUV);
	~SDLRenderer() override;

	bool init(HWND hwnd) override;

	virtual void loadYuv(const uint8_t* Y, const uint8_t* U, const uint8_t* V, int width, int height) override;
	virtual void setMirror(bool useMirror) override;
	void clean() override;

private:
	bool initTexture(int w, int h);
	void loadImage(const uint8_t *data, int width, int height, int pitch);

	void onRefresh();

private:
	IMAGE_TYPE		m_imageType;

	SDL_Window*     m_window;
	SDL_Renderer*   m_renderer;
	SDL_Texture*    m_texture;
	HWND            m_hwnd = nullptr;

	int m_lastWidth = 0;
	int m_lastHeight = 0;
	bool m_useMirror = true;
};


#endif // SDLRENDERER_H
