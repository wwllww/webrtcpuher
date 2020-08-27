#include "MacRender.h"


namespace xrtc {
    
    IRenderer *CreateMacRenderGL() {
        return new MacRenderGL();
    }
    
    MacRenderGL::MacRenderGL(IMAGE_TYPE type):imageType_(type),renderMode_(RENDER_MODE_NORM),
    bInit_(false),buffer_(nullptr),texture_(0),width_(0),height_(0),bufffer_size_(0),windowWidth(0),
    windowHeight(0),nsview_(nullptr),useMirror_(true){
        
    }
    MacRenderGL::~MacRenderGL() {
        if(!bInit_)
            return;
        bInit_ = false;
        
        delete [] buffer_;
        buffer_ = nullptr;
        
        //glDeleteTextures(1,&texture_);
    }
    
    bool MacRenderGL::init(HWND hwnd) {
        
        if(bInit_)
            return false;
        
        nsview_ = (__bridge NSView *) hwnd;
        
        NSRect Rect = nsview_.frame;//[nsview_ visibleRect];
        windowWidth = Rect.size.width;
        windowHeight = Rect.size.height;
        
        NSRect viewFrame = NSMakeRect(0, 0, Rect.size.width, Rect.size.height);
        NSOpenGLView *view = [[NSOpenGLView alloc] initWithFrame:viewFrame pixelFormat:nil];
        context_ = [view openGLContext];
        
        [nsview_ addSubview:view];
        [context_ makeCurrentContext];
        
        glGenTextures(1, &texture_);
        ResizeViewport(windowWidth,windowHeight);
        
        bInit_ = true;
        return true;
    }
    
    void MacRenderGL::loadYuv(const uint8_t* Y, const uint8_t* U, const uint8_t* V, int width, int height) {
        [context_ makeCurrentContext];
        if(width_ != width || height != height_) {
            ResizeVideo(width,height);
        }
        
        Yuv420toRgb32(Y, U, V, width, height, buffer_);
        
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture_);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, GL_RGBA,GL_UNSIGNED_INT_8_8_8_8,buffer_);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glBegin(GL_QUADS);
        {
            
            //NSRect Rect = [nsview_ visibleRect];
            if(windowWidth != nsview_.frame.size.width || windowHeight != nsview_.frame.size.height) {
                ResizeViewport(nsview_.frame.size.width,nsview_.frame.size.height);
            }
            
            float x = 0,x2 = windowWidth,y = 0,y2 = windowHeight;
            
            if (width > height)
            {
                //等比例缩放
                float aspect = (float)windowWidth/ windowHeight;
                
                if (aspect > 1.0f)
                {
                    y = 0;
                    y2 = y + windowHeight;
                    
                    float NewWidth = (float)width * windowHeight / height;
                    
                    x = (windowWidth - NewWidth) / 2;
                    x2 = x + NewWidth;
                    
                    if (NewWidth > windowWidth)
                    {
                        x = 0;
                        x2 = windowWidth;
                        
                        float NewHeight = (float)height * windowWidth/ width;
                        
                        y = (windowHeight- NewHeight) / 2;
                        y2 = y + NewHeight;
                    }
                }
                else
                {
                    x = 0;
                    x2 = windowWidth;
                    
                    float NewHeight = (float)height * windowWidth / width;
                    
                    y = (windowHeight - NewHeight) / 2;
                    y2 = y + NewHeight;
                    
                    if (NewHeight > windowHeight)
                    {
                        y = 0;
                        y2 = y + windowHeight;
                        
                        float NewWidth = (float)width * windowHeight / height;
                        
                        x = (windowWidth - NewWidth) / 2;
                        x2 = x + NewWidth;
                    }
                }
            }
            else
            {
                int w = (float)windowHeight * width / height;
                
                int RenderSide = 1;
                
                if (w < windowWidth)
                {
                    if (RenderSide == 0) //居左
                    {
                        x = 0;
                        x2 = x + w;
                    }
                    else if (RenderSide == 1) //居中
                    {
                        x = (windowWidth- w) / 2;
                        x2 = x + w;
                    }
                    else  //居右
                    {
                        x = windowWidth - w;
                        x2 = x + w;
                    }
                }
                else
                {
                    x = 0;
                    x2 = windowWidth;
                    
                    float NewHeight = (float)height * windowWidth/ width;
                    
                    y = (windowHeight- NewHeight) / 2;
                    y2 = y + NewHeight;
                    
                    if (NewHeight > windowHeight)
                    {
                        y = 0;
                        y2 = y + windowHeight;
                        
                        float NewWidth = (float)width * windowHeight / height;
                        
                        x = (windowWidth- NewWidth) / 2;
                        x2 = x + NewWidth;
                    }
                }
                
            }
            
            if(renderMode_ == RENDER_MODE_HIDDEN) {
                //clip
                x = y = 0;
                if(x2 == windowWidth) {
                    
                    float newWidth = (float)width * windowHeight / height;
                    
                    x2 = newWidth;
                    y2 = windowWidth;
                    
                } else if(y2 == windowHeight) {
                    
                    float newHeight = (float)height * windowWidth/ width;
                    
                    x2 = windowWidth;
                    y2 = newHeight;
                }
                
            }
            
            if(!useMirror_) {
                
                glTexCoord2f(0.0f, 0.0f);
                glVertex3f(x / windowWidth, y / windowHeight, 0.0f);
                
                glTexCoord2f(0.0f, 1.0f);
                glVertex3f(x / windowWidth, y2 / windowHeight, 0.0f);
                
                glTexCoord2f(1.0f, 1.0f);
                glVertex3f(x2 / windowWidth, y2 / windowHeight, 0.0f);
                
                glTexCoord2f(1.0f, 0.0f);
                glVertex3f(x2 / windowWidth, y / windowHeight, 0.0f);
                
            } else {
                
                glTexCoord2f(0.0f, 0.0f);
                glVertex3f(x2 / windowWidth, y / windowHeight, 0.0f);
                
                glTexCoord2f(0.0f, 1.0f);
                glVertex3f(x2 / windowWidth, y2 / windowHeight, 0.0f);
                
                glTexCoord2f(1.0f, 1.0f);
                glVertex3f(x / windowWidth, y2 / windowHeight, 0.0f);
                
                glTexCoord2f(1.0f, 0.0f);
                glVertex3f(x / windowWidth, y / windowHeight, 0.0f);
            }
            
        }
        glEnd();
        
        glBindTexture(GL_TEXTURE_2D, 0);
        glFlush();
        
    }
    void MacRenderGL::loadRGB(const uint8_t *data, int width, int height) {
        
    }
    
    void MacRenderGL::setDisplayMode(RENDER_MODE mode) {
        renderMode_ = mode;
    }
    
    void MacRenderGL::setMirror(bool useMirror) {
        useMirror_ = useMirror;
    }
    
	void MacRenderGL::setIsLocalRender(bool bLocal)
	{
		if(!bLocal) {
			useMirror_ = false;
		}
	}


    void MacRenderGL::clean() {
        
    }
    
    void MacRenderGL::ResizeViewport(size_t width,size_t height) {
        
        glViewport(0, 0, width, height);
        
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glOrtho(0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f);
        glMatrixMode(GL_MODELVIEW);
        windowHeight = height;
        windowWidth = width;
    }
    
    void MacRenderGL::ResizeVideo(size_t width,size_t height) {
        if(bInit_) {
            
            width_ = width;
            height_ = height;
            
            bufffer_size_ = width_ * height_ * 4;
            
            if(buffer_)
                delete [] buffer_;
            
            buffer_ = new uint8_t[bufffer_size_];
            
            memset(buffer_,0,bufffer_size_);
            
            glBindTexture(GL_TEXTURE_2D, texture_);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL,0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8, buffer_);
 
        }
    }
    
    void MacRenderGL::Yuv420toRgb32(const uint8_t* srcY, const uint8_t* srcU,
                             const uint8_t* srcV,int width,int height,
                                  uint8_t* RGB32) {
        
        int Y, U, V, R, G, B;
        int i, j;
        int cwidth = width >> 1;
        
        for (i = 0; i < height; ++i)
        {
            for (j = 0; j < width; ++j)
            {
                Y = *(srcY + i * width + j);
                U = *(srcU + (i >> 1) * cwidth + (j >> 1));
                V = *(srcV + (i >> 1) * cwidth + (j >> 1));
                R = Y + 1.403 * (V - 128);
                G = Y - 0.344 * (U - 128) - 0.714 * (V - 128);
                B = Y + 1.772 * (U - 128);
                
                if (R <= 0 || R >= 255)
                {
                    R = (R <= 0) ? 0 : 255;
                }
                if (G <= 0 || G >= 255)
                {
                    G = (G <= 0) ? 0 : 255;
                }
                if (B <= 0 || B >= 255)
                {
                    B = (B <= 0) ? 0 : 255;
                }
                
                *(RGB32 + (i* width + j) * 4) = 255;
                *(RGB32 + (i * width + j) * 4 + 1) = B;
                *(RGB32 + (i * width + j) * 4 + 2) = G;
                *(RGB32 + (i* width + j) * 4 + 3) = R;
                
                
            }
        }
    }
    
    
}

