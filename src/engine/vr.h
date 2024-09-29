#ifndef __VR_H__
#define __VR_H__

extern int virtualreality;

namespace vr
{
    struct vrbuffer
    {
        GLuint resolvefb;   // Framebuffer object
        GLuint resolvetex;  // Texture attached to framebuffer
    };

    extern bool isEnabled();
    extern void update();
    extern void render();
    extern bool init();
    extern void clean();
}

#endif
