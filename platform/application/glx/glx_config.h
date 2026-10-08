#pragma once

#include <motor/ogl/glx/glx.h>

namespace motor
{
    namespace platform
    {
        namespace glx
        {
            // Use the same visual for the X window and its GLX context.
            inline GLXFBConfig choose_config( Display * display, int const screen,
                VisualID const visual = 0 ) noexcept
            {
                int const attributes[] =
                {
                    GLX_X_RENDERABLE, True,
                    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT | GLX_PBUFFER_BIT,
                    GLX_RENDER_TYPE, GLX_RGBA_BIT,
                    GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
                    GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8,
                    GLX_ALPHA_SIZE, 8, GLX_DEPTH_SIZE, 24, GLX_STENCIL_SIZE, 8,
                    GLX_DOUBLEBUFFER, True, None
                } ;

                int count = 0 ;
                GLXFBConfig * configs = glXChooseFBConfig( display, screen, attributes, &count ) ;
                GLXFBConfig result = nullptr ;
                for( int i = 0 ; configs != nullptr && i < count ; ++i )
                {
                    int id = 0 ;
                    glXGetFBConfigAttrib( display, configs[i], GLX_VISUAL_ID, &id ) ;
                    if( id != 0 && ( visual == 0 || VisualID( id ) == visual ) )
                    {
                        result = configs[i] ;
                        break ;
                    }
                }
                if( configs != nullptr ) XFree( configs ) ;
                return result ;
            }
        }
    }
}
