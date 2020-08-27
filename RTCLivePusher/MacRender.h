#ifndef MacRender_h
#define MacRender_h
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#import <Cocoa/Cocoa.h>
#include "VideoRenderer.h"
namespace xrtc {
 
    class MacRenderGL: public IRenderer
    {
    public:
        enum IMAGE_TYPE {
            IMAGE_TYPE_RGB = 1,
            IMAGE_TYPE_YUV = 2
        };
        explicit MacRenderGL(IMAGE_TYPE type = IMAGE_TYPE_YUV);
        ~MacRenderGL() override;
        
        bool init(HWND hwnd) override;
        
        virtual void loadYuv(const uint8_t* Y, const uint8_t* U, const uint8_t* V, int width, int height) override;
        virtual void loadRGB(const uint8_t *data, int width, int height) override;
        
        virtual void setDisplayMode(RENDER_MODE mode) override;
        virtual void setMirror(bool useMirror) override;
		virtual void setIsLocalRender(bool bLocal) override;
        
        void clean() override;
        
    private:
        IMAGE_TYPE   imageType_;
        RENDER_MODE  renderMode_;
        bool bInit_;
        uint8_t *buffer_;
        GLuint texture_;
        size_t width_,height_,bufffer_size_;
        int windowWidth,windowHeight;
        NSOpenGLContext *context_;
        NSView * nsview_;
        void ResizeViewport(size_t width,size_t height);
        void ResizeVideo(size_t width,size_t height);
        void Yuv420toRgb32(const uint8_t* srcY, const uint8_t* srcU,
                         const uint8_t* srcV,int width,int height,uint8_t* RGB32);
        
        bool useMirror_;
    };

} // namespace xrtc
#endif /* MacRender_h */
