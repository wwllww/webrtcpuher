
#include "VideoRenderer.h"

#if defined(__APPLE__)
namespace xrtc 
{
    class IRenderer;
    extern IRenderer *CreateMacRenderGL();
}
#else
#include "SDLRenderer.h"
#endif

IRenderer* RendererFactory::create(RENDERER_TYPE type)
{
	switch (type)
	{
	case RENDERER_OPENGL:
#if defined(__APPLE__)
		return CreateMacRenderGL();
#endif
		return nullptr;
	case RENDERER_SDL:
#ifndef __APPLE__
		return new SDLRenderer();
#endif
	default:
		return nullptr;
		break;
	}
}


