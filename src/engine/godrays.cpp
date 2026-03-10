// godrays.cpp: cloud-driven atmospheric crepuscular rays

#include "engine.h"

namespace godRays
{
    // Settings vars
    VARFP(godrays, 0, 1, 1, if(!godrays) cleanup());
    VARP(godrayssteps, 1, 24, 64);
    FVARP(godraysscale, 0.25f, 0.5f, 1.0f);

    // Map vars
    FVARR(godraysstrength, 0.0f, 0.5f, 1.0f);
    FVARR(godraysdensity, 0.25f, 3.0f, 16.0f);
    FVARR(godraysmaxdist, 0.05f, 0.35f, 2.0f);
    FVARR(godraysnearstart, 0.0f, 1024.0f, 4096.0f);
    FVARR(godraysnearend, 1.0f, 4096.0f, 8192.0f);
    FVARR(godraysforwardexp, 0.25f, 1.0f, 32.0f);

    static int bufferWidth = -1, bufferHeight = -1;
    static GLuint rayFbo = 0, rayTex = 0;
    static GLenum passFormat = GL_RGBA8;

    static void setupBuffers(int targetWidth, int targetHeight)
    {
        bufferWidth = targetWidth;
        bufferHeight = targetHeight;
        passFormat = hasAFBO && hasTF ? GL_RGBA16F : GL_RGBA8;
        const int passFilter = (bufferWidth < vieww || bufferHeight < viewh) ? 1 : 0;

        if(!rayTex) glGenTextures(1, &rayTex);
        if(!rayFbo) glGenFramebuffers_(1, &rayFbo);

        glBindFramebuffer_(GL_FRAMEBUFFER, rayFbo);
        createtexture(rayTex, bufferWidth, bufferHeight, NULL, 3, passFilter, passFormat, GL_TEXTURE_RECTANGLE);
        glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, rayTex, 0);
        if(glCheckFramebufferStatus_(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            fatal("failed allocating god rays buffers!");

        glBindFramebuffer_(GL_FRAMEBUFFER, 0);
    }

    static bool ensureBuffers()
    {
        if(vieww <= 0 || viewh <= 0) return false;

        const float renderScale = clamp(godraysscale, 0.25f, 1.0f);
        const int targetWidth = max(int(ceilf(vieww*renderScale)), 1),
                  targetHeight = max(int(ceilf(viewh*renderScale)), 1);
        const GLenum targetFormat = hasAFBO && hasTF ? GL_RGBA16F : GL_RGBA8;
        if(rayTex && rayFbo &&
           bufferWidth == targetWidth && bufferHeight == targetHeight &&
           passFormat == targetFormat) return true;

        cleanup();
        setupBuffers(targetWidth, targetHeight);
        return true;
    }

    void cleanup()
    {
        if(rayFbo) { glDeleteFramebuffers_(1, &rayFbo); rayFbo = 0; }
        if(rayTex) { glDeleteTextures(1, &rayTex); rayTex = 0; }
        bufferWidth = bufferHeight = -1;
    }

    void render()
    {
        if(!godrays || drawtex || !hasCloudLayerProjection() || godraysdensity <= 0.0f || godrayssteps <= 0) return;
        if(sunlight.iszero() || sunlightscale <= 0.0f || sunlightdir.z <= 1.0e-4f) return;
        if(!ensureBuffers()) return;

        vec4 cloudShadowParams(0, 0, 0, 0), cloudShadowTransform(0, 0, 1, 0);
        vec cloudShadowColor = getcloudlayershadowcolour().tocolor();
        float cloudShadowStrength = max(getcloudlayershadowstrength(), getCloudLayerOpacity());
        getCloudLayerParams(cloudShadowParams, cloudShadowTransform);
        if(cloudShadowStrength <= 1.0e-4f) return;

        vec sunColor = sunlight.tocolor().mul(max(sunlightscale, 0.0f)).mul(ldrscale * 2.0f);
        if(sunColor.squaredlen() <= 1.0e-8f) return;

        const float nearStart = max(godraysnearstart, 0.0f),
                    nearEnd   = max(godraysnearend, nearStart + 1.0f);

        const float maxDistance = clamp(float(farplane) * max(godraysmaxdist, 0.05f), 1.0f, float(farplane));

        const float stepCount = clamp(float(godrayssteps), 1.0f, 64.0f);

        timer *godRayTimer = begintimer("god rays");

        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);

        glBindFramebuffer_(GL_FRAMEBUFFER, rayFbo);
        glViewport(0, 0, bufferWidth, bufferHeight);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture_(GL_TEXTURE0);
        if(msaalight) glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msdepthtex);
        else glBindTexture(GL_TEXTURE_RECTANGLE, gdepthtex);
        glActiveTexture_(GL_TEXTURE1);
        if(!bindCloudLayer()) glBindTexture(GL_TEXTURE_2D, notexture->id);
        glActiveTexture_(GL_TEXTURE0);

        SETSHADER(cloudCrepuscularRays);
        LOCALPARAM(sunDir, sunlightdir);
        LOCALPARAM(sunColor, sunColor);
        LOCALPARAM(cloudShadowParams, cloudShadowParams);
        LOCALPARAM(cloudShadowTransform, cloudShadowTransform);
        LOCALPARAM(cloudShadowColor, cloudShadowColor);
        LOCALPARAMF(cloudShadowStrength, cloudShadowStrength);
        LOCALPARAMF(godRayMarchParams, max(godraysforwardexp, 0.25f), max(godraysdensity, 0.25f), maxDistance, 0.0f);
        LOCALPARAMF(godRayDistanceParams, nearStart, nearEnd, stepCount, clamp(godraysstrength, 0.0f, 1.0f));
        screenquad(vieww, viewh);

        glBindFramebuffer_(GL_FRAMEBUFFER, msaalight ? mshdrfbo : hdrfbo);
        glViewport(0, 0, vieww, viewh);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glBindTexture(GL_TEXTURE_RECTANGLE, rayTex);
        SETSHADER(scalelinear);
        screenquad(bufferWidth, bufferHeight);
        glDisable(GL_BLEND);

        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);

        glActiveTexture_(GL_TEXTURE0);

        endtimer(godRayTimer);
    }
}
