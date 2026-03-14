// heathaze.cpp: heat haze effects (/doc/Heat Haze.md)

#include "engine.h"

extern GLuint hdrfbo, refractfbo, refracttex;
extern GLuint mshdrfbo, msrefractfbo, msrefracttex;
extern float getfovscale(float referenceFov);

namespace heatHaze
{
    static Texture *worldhazenormaltex = NULL;

    VARR(worldheathaze, 0, 0, 1);
    FVARR(worldheathazestrength, 0.0f, 3.0f, 100.0f);
    FVARR(worldheathazetexsize, 0.125f, 0.125f, 16.0f);
    FVARR(worldheathazemindist, 0.0f, 64.0f, 1e4f);
    FVARR(worldheathazemargin, 1.0f, 1024.0f, 1e5f);
    FVARR(worldheathazescrollx, -2.0f, 0.02f, 1.0f);
    FVARR(worldheathazescrolly, -2.0f, -0.1f, 1.0f);
    FVARR(worldheathazeupdot, -1.0f, 0.15f, 1.0f);
    FVARR(worldheathazeupfade, 1e-3f, 0.20f, 4.0f);

    VARP(heathaze, 0, 1, 1);
    FVARP(heathazestrength, 0, 32.0f, 64.0f);
    FVARP(heathazenearstrength, 0, 1.0f, 16.0f);
    FVARP(heathazefarstrength, 0, 1.0f, 16.0f);
    FVARP(heathazenear, 0, 64.0f, 1e4f);
    FVARP(heathazefar, 1, 512.0f, 2e4f);
    FVARP(heathazedepthscale, 1e-3f, 0.1f, 1e3f);
    FVARP(heathazescrollx, -10.0f, 0.1f, 10.0f);
    FVARP(heathazescrolly, -10.0f, 0.5f, 10.0f);


    bool shouldRenderHazeParticles()
    {
        return heathaze && heathazestrength > 0 && (heathazenearstrength > 0 || heathazefarstrength > 0);
    }

    bool bindSceneTexture()
    {
        if(msaalight)
        {
            if(!mshdrfbo || !msrefractfbo || !msrefracttex) return false;
        }
        else
        {
            if(!hdrfbo || !refractfbo || !refracttex) return false;
        }

        // The transparent pass can leave scissor enabled for particle layers.
        // Ensure we always copy the full resolved scene into the haze source.
        bool hadScissor = glIsEnabled(GL_SCISSOR_TEST) != 0;
        if(hadScissor) glDisable(GL_SCISSOR_TEST);

        if(msaalight)
        {
            glBindFramebuffer_(GL_READ_FRAMEBUFFER, mshdrfbo);
            glBindFramebuffer_(GL_DRAW_FRAMEBUFFER, msrefractfbo);
            glBlitFramebuffer_(0, 0, vieww, viewh, 0, 0, vieww, viewh, GL_COLOR_BUFFER_BIT, GL_NEAREST);
            glBindFramebuffer_(GL_FRAMEBUFFER, mshdrfbo);
            glActiveTexture_(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msrefracttex);
        }
        else
        {
            glBindFramebuffer_(GL_READ_FRAMEBUFFER, hdrfbo);
            glBindFramebuffer_(GL_DRAW_FRAMEBUFFER, refractfbo);
            glBlitFramebuffer_(0, 0, vieww, viewh, 0, 0, vieww, viewh, GL_COLOR_BUFFER_BIT, GL_NEAREST);
            glBindFramebuffer_(GL_FRAMEBUFFER, hdrfbo);
            glActiveTexture_(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_RECTANGLE, refracttex);
        }

        glActiveTexture_(GL_TEXTURE0);
        if(hadScissor) glEnable(GL_SCISSOR_TEST);
        return true;
    }

    bool shouldRenderWorldHaze()
    {
        return worldheathaze && worldheathazestrength > 0;
    }

    void setShaderParams(bool scroll, bool fade)
    {
        float distSpan = max(heathazefar - heathazenear, 1e-3f);
        LOCALPARAMF(heatHazeParams, max(heathazestrength, 0.0f), max(heathazedepthscale, 1e-3f), heathazenear, 1.0f/distSpan);
        LOCALPARAMF(heatHazeStrengths, max(heathazenearstrength, 0.0f), max(heathazefarstrength, 0.0f));
        float scrollTime = lastmillis*0.001f;
        float scrollX = scroll ? fmodf(heathazescrollx*scrollTime, 1.0f) : 0.0f;
        float scrollY = scroll ? fmodf(heathazescrolly*scrollTime, 1.0f) : 0.0f;
        LOCALPARAMF(heatHazeScroll, scrollX, scrollY);
        LOCALPARAMF(heatHazeOptions, fade ? 1.0f : 0.0f, 0, 0, 0);
    }

    static inline void setWorldShaderParams()
    {
        float startDist = max(max(worldheathazemindist, 0.0f), 24.0f);
        float invMargin = 1.0f/max(worldheathazemargin, 1.0f);
        float scrollTime = lastmillis*0.001f;
        float texScale = 0.30f/max(worldheathazetexsize, 1e-3f);
        float upDot = clamp(worldheathazeupdot, -1.0f, 1.0f);
        float invUpFade = 1.0f/max(worldheathazeupfade, 1e-3f);

        LOCALPARAMF(strenght, max(worldheathazestrength * getfovscale(100.0f), 0.0f));
        LOCALPARAMF(params, startDist, invMargin, invMargin, 1.0f);
        LOCALPARAMF(color, 1.0f, 1.0f, 1.0f, 0.0f);
        LOCALPARAMF(upmask, upDot, invUpFade, 0.0f, 0.0f);
        LOCALPARAMF(texgen, texScale, texScale, scrollTime*worldheathazescrollx, scrollTime*worldheathazescrolly);
    }

    void renderWorld()
    {
        if(!shouldRenderWorldHaze() || !bindSceneTexture()) return;

        if(msaalight) return; // This shader variant expects rectangle color fetches.

        if(!worldhazenormaltex) worldhazenormaltex = textureload("media/texture/mat_air/refraction.png", 0, true, false);

        glActiveTexture_(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, (worldhazenormaltex ? worldhazenormaltex : notexture)->id);
        glActiveTexture_(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_RECTANGLE, gnormaltex);
        glActiveTexture_(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_RECTANGLE, refracttex);
        glActiveTexture_(GL_TEXTURE9);
        glBindTexture(GL_TEXTURE_RECTANGLE, gdepthtex);
        glActiveTexture_(GL_TEXTURE0);

        bool hadblend = glIsEnabled(GL_BLEND) != 0;
        bool haddepthtest = glIsEnabled(GL_DEPTH_TEST) != 0;
        bool hadscissor = glIsEnabled(GL_SCISSOR_TEST) != 0;
        GLboolean olddepthmask = GL_TRUE;
        glGetBooleanv(GL_DEPTH_WRITEMASK, &olddepthmask);

        if(hadblend) glDisable(GL_BLEND);
        if(haddepthtest) glDisable(GL_DEPTH_TEST);
        if(hadscissor) glDisable(GL_SCISSOR_TEST);
        glDepthMask(GL_FALSE);

        SETSHADER(worldheathazetex);
        setWorldShaderParams();
        screenquad(vieww, viewh);

        glDepthMask(olddepthmask);
        if(haddepthtest) glEnable(GL_DEPTH_TEST);
        if(hadblend) glEnable(GL_BLEND);
        if(hadscissor) glEnable(GL_SCISSOR_TEST);
    }
}
