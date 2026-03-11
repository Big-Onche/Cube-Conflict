// godrays.cpp: atmospheric and world-space crepuscular rays

#include "engine.h"

namespace godRays
{
    // Settings vars
    VARFP(godrays, 0, 1, 1, if(!godrays) cleanup());
    VARP(godrayssteps, 1, 24, 64);
    FVARP(godraysscale, 0.125f, 0.25f, 1.0f);
    VARP(godraysatrous, 0, 1, 1);
    VARP(godraysatrousiter, 1, 2, 3);

    // Tunables
    FVAR(godraysatrousalphak, 0.0f, 0.0f, 256.0f);
    FVAR(godraysupscaleedge, 0.0f, 0.02f, 1.0f);

    // Map vars
    FVARR(godraysstrength, 0.0f, 0.5f, 1.0f);
    FVARR(godraysdensity, 0.25f, 3.0f, 16.0f);
    FVARR(godraysmaxaccum, 0.0f, 2.0f, 64.0f);
    FVARR(godraysmaxdist, 0.05f, 0.3f, 2.0f);
    FVARR(godraysnearstart, 0.0f, 0.0f, 4096.0f);
    FVARR(godraysnearend, 1.0f, 128.0f, 8192.0f);
    FVARR(godraysforwardexp, 0.25f, 1.0f, 32.0f);

    // Settings vars
    VARP(godraysgeom, 0, 1, 1);
    VARP(godraysgeomsteps, 1, 32, 64);
    FVARP(godraysgeomshadowbias, 0.0f, 2.0f, 32.0f);
    FVARP(godraysgeomforwardexp, 0.25f, 16.0f, 32.0f);

    // Tunables
    FVAR(godraysgeomdecay, 0.0f, 0.93f, 1.0f);
    FVAR(godraysgeomthreshold, 0.0f, 0.15f, 1.0f);

    // Map vars
    FVARR(godraysgeomstrength, 0.0f, 2.0f, 4.0f);
    FVARR(godraysgeomdensity, 0.25f, 0.85f, 4.0f);
    FVARR(godraysgeommaxdist, 0.01f, 0.14f, 1.0f);

    static int bufferWidth = -1, bufferHeight = -1;
    static GLuint rayFbo = 0, rayTex = 0;
    static GLuint rayFilterFbo = 0, rayFilterTex = 0;
    static GLuint rayGuideFbo = 0, rayGuideTex = 0;
    static GLenum passFormat = GL_RGBA8;
    static GLenum guideFormat = GL_RGBA8;

    static void setupBuffers(int targetWidth, int targetHeight)
    {
        bufferWidth = targetWidth;
        bufferHeight = targetHeight;
        passFormat = hasAFBO && hasTF ? GL_RGBA16F : GL_RGBA8;
        const int passFilter = (bufferWidth < vieww || bufferHeight < viewh) ? 1 : 0;

        if(!rayTex) glGenTextures(1, &rayTex);
        if(!rayFbo) glGenFramebuffers_(1, &rayFbo);
        if(!rayFilterTex) glGenTextures(1, &rayFilterTex);
        if(!rayFilterFbo) glGenFramebuffers_(1, &rayFilterFbo);
        if(!rayGuideTex) glGenTextures(1, &rayGuideTex);
        if(!rayGuideFbo) glGenFramebuffers_(1, &rayGuideFbo);

        glBindFramebuffer_(GL_FRAMEBUFFER, rayFbo);
        createtexture(rayTex, bufferWidth, bufferHeight, NULL, 3, passFilter, passFormat, GL_TEXTURE_RECTANGLE);
        glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, rayTex, 0);
        if(glCheckFramebufferStatus_(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            fatal("failed allocating god rays buffers!");

        glBindFramebuffer_(GL_FRAMEBUFFER, rayFilterFbo);
        createtexture(rayFilterTex, bufferWidth, bufferHeight, NULL, 3, passFilter, passFormat, GL_TEXTURE_RECTANGLE);
        glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, rayFilterTex, 0);
        if(glCheckFramebufferStatus_(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            fatal("failed allocating god rays filter buffers!");

        glBindFramebuffer_(GL_FRAMEBUFFER, rayGuideFbo);
        guideFormat = hasAFBO && hasTF ? GL_RGBA16F : GL_RGBA8;
        createtexture(rayGuideTex, bufferWidth, bufferHeight, NULL, 3, 0, guideFormat, GL_TEXTURE_RECTANGLE);
        glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, rayGuideTex, 0);
        if(glCheckFramebufferStatus_(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            fatal("failed allocating god rays depth guide buffer!");

        glBindFramebuffer_(GL_FRAMEBUFFER, 0);
    }

    static bool ensureBuffers()
    {
        if(vieww <= 0 || viewh <= 0) return false;

        const float renderScale = clamp(godraysscale, 0.25f, 1.0f);
        const int targetWidth = max(int(ceilf(vieww*renderScale)), 1),
                  targetHeight = max(int(ceilf(viewh*renderScale)), 1);
        const GLenum targetPassFormat = hasAFBO && hasTF ? GL_RGBA16F : GL_RGBA8;
        const GLenum targetGuideFormat = hasAFBO && hasTF ? GL_RGBA16F : GL_RGBA8;
        if(rayTex && rayFbo && rayFilterTex && rayFilterFbo && rayGuideTex && rayGuideFbo &&
           bufferWidth == targetWidth && bufferHeight == targetHeight &&
           passFormat == targetPassFormat &&
           guideFormat == targetGuideFormat) return true;

        cleanup();
        setupBuffers(targetWidth, targetHeight);
        return true;
    }

    void cleanup()
    {
        if(rayFbo) { glDeleteFramebuffers_(1, &rayFbo); rayFbo = 0; }
        if(rayTex) { glDeleteTextures(1, &rayTex); rayTex = 0; }
        if(rayFilterFbo) { glDeleteFramebuffers_(1, &rayFilterFbo); rayFilterFbo = 0; }
        if(rayFilterTex) { glDeleteTextures(1, &rayFilterTex); rayFilterTex = 0; }
        if(rayGuideFbo) { glDeleteFramebuffers_(1, &rayGuideFbo); rayGuideFbo = 0; }
        if(rayGuideTex) { glDeleteTextures(1, &rayGuideTex); rayGuideTex = 0; }
        passFormat = guideFormat = GL_RGBA8;
        bufferWidth = bufferHeight = -1;
    }

    void render()
    {
        const bool wantCloudPass = godrays && hasCloudLayerProjection() && godraysdensity > 0.0f && godrayssteps > 0;
        const bool wantGeomPass = godraysgeom && csmshadowmap && shadowatlastex && csmsplits > 0 &&
                                  godraysgeomstrength > 0.0f && godraysgeomdensity > 0.0f &&
                                  godraysgeomsteps > 0 && godraysgeommaxdist > 0.0f;
        if(drawtex || (!wantCloudPass && !wantGeomPass)) return;
        if(sunlight.iszero() || sunlightscale <= 0.0f || sunlightdir.z <= 1.0e-4f) return;
        if(!ensureBuffers()) return;

        vec4 cloudShadowParams(0, 0, 0, 0), cloudShadowTransform(0, 0, 1, 0);
        vec cloudShadowColor = getcloudlayershadowcolour().tocolor();
        float cloudShadowStrength = 0.0f;
        bool renderCloudPass = false;
        if(wantCloudPass)
        {
            cloudShadowStrength = max(getcloudlayershadowstrength(), getCloudLayerOpacity());
            getCloudLayerParams(cloudShadowParams, cloudShadowTransform);
            renderCloudPass = cloudShadowStrength > 1.0e-4f;
        }
        const bool renderGeomPass = wantGeomPass;
        if(!renderCloudPass && !renderGeomPass) return;

        vec sunColor = sunlight.tocolor().mul(max(sunlightscale, 0.0f)).mul(ldrscale * 2.0f);
        if(sunColor.squaredlen() <= 1.0e-8f) return;

        const float nearStart = max(godraysnearstart, 0.0f),
                    nearEnd   = max(godraysnearend, nearStart + 1.0f);

        const float maxDistance = clamp(float(farplane) * max(godraysmaxdist, 0.05f), 1.0f, float(farplane));
        const float geomMaxDistance = clamp(float(farplane) * max(godraysgeommaxdist, 0.01f), 1.0f, float(farplane));

        const float stepCount = clamp(float(godrayssteps), 1.0f, 64.0f);
        const float geomStepCount = clamp(float(godraysgeomsteps), 1.0f, 64.0f);
        const float atrousDepthStrength = clamp(godraysatrousalphak, 0.0f, 8192.0f);

        timer *godRayTimer = begintimer("god rays");

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glDepthMask(GL_FALSE);

        glBindFramebuffer_(GL_FRAMEBUFFER, rayFbo);
        glViewport(0, 0, bufferWidth, bufferHeight);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        if(renderCloudPass)
        {
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
            LOCALPARAMF(godRayMarchParams, max(godraysforwardexp, 0.25f), max(godraysdensity, 0.25f), maxDistance, max(godraysmaxaccum, 0.0f));
            LOCALPARAMF(godRayDistanceParams, nearStart, nearEnd, stepCount, clamp(godraysstrength, 0.0f, 1.0f));
            screenquad(vieww, viewh);
        }

        if(renderGeomPass)
        {
            if(renderCloudPass)
            {
                glEnable(GL_BLEND);
                glBlendFunc(GL_ONE, GL_ONE);
            }

            glActiveTexture_(GL_TEXTURE0);
            if(msaalight) glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msdepthtex);
            else glBindTexture(GL_TEXTURE_RECTANGLE, gdepthtex);
            glActiveTexture_(GL_TEXTURE2);
            glBindTexture(shadowatlastarget, shadowatlastex);
            glTexParameteri(shadowatlastarget, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            glTexParameteri(shadowatlastarget, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
            glTexParameteri(shadowatlastarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(shadowatlastarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glActiveTexture_(GL_TEXTURE0);

            if(shadowatlastarget == GL_TEXTURE_2D) SETSHADER(geometryCrepuscularRays2D);
            else SETSHADER(geometryCrepuscularRaysRect);
            LOCALPARAM(sunDir, sunlightdir);
            LOCALPARAM(sunColor, sunColor);
            LOCALPARAMF(godRayGeomParams, max(godraysgeomdensity, 0.25f), clamp(godraysgeomdecay, 0.0f, 1.0f), geomMaxDistance, max(godraysgeomforwardexp, 0.25f));
            LOCALPARAMI(godRayGeomSteps, int(geomStepCount));
            LOCALPARAMF(godRayGeomDistanceParams, clamp(godraysgeomstrength, 0.0f, 2.0f), 0.0f, 0.0f, 0.0f);
            LOCALPARAMF(godRayGeomShapeParams, max(godraysgeomshadowbias, 0.0f), clamp(godraysgeomthreshold, 0.0f, 1.0f), 0.0f, 0.0f);
            LOCALPARAMI(csmcount, csmsplits);
            screenquad(vieww, viewh);

            if(renderCloudPass) glDisable(GL_BLEND);
        }

        glBindFramebuffer_(GL_FRAMEBUFFER, rayGuideFbo);
        glViewport(0, 0, bufferWidth, bufferHeight);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glActiveTexture_(GL_TEXTURE0);
        if(msaalight) glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msdepthtex);
        else glBindTexture(GL_TEXTURE_RECTANGLE, gdepthtex);
        SETSHADER(godRayDepthGuide);
        screenquad(vieww, viewh);

        GLuint compositeTex = rayTex;
        if(godraysatrous)
        {
            GLuint filterTex[2] = { rayTex, rayFilterTex };
            GLuint filterFbo[2] = { rayFbo, rayFilterFbo };
            int sourceIndex = 0, targetIndex = 1;
            const int filterIterations = clamp(godraysatrousiter, 1, 3);
            loopi(filterIterations)
            {
                glBindFramebuffer_(GL_FRAMEBUFFER, filterFbo[targetIndex]);
                glViewport(0, 0, bufferWidth, bufferHeight);

                glActiveTexture_(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_RECTANGLE, filterTex[sourceIndex]);
                glActiveTexture_(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_RECTANGLE, rayGuideTex);
                glActiveTexture_(GL_TEXTURE0);
                SETSHADER(aTrousFilter);
                LOCALPARAMF(aTrousSize, float(bufferWidth), float(bufferHeight));
                LOCALPARAMF(aTrousParams, float(1<<i), atrousDepthStrength, 0.0f, 0.0f);
                screenquad(bufferWidth, bufferHeight);

                swap(sourceIndex, targetIndex);
            }
            compositeTex = filterTex[sourceIndex];
        }

        glBindFramebuffer_(GL_FRAMEBUFFER, msaalight ? mshdrfbo : hdrfbo);
        glViewport(0, 0, vieww, viewh);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
        glActiveTexture_(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_RECTANGLE, compositeTex);
        glActiveTexture_(GL_TEXTURE1);
        if(msaalight) glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msdepthtex);
        else glBindTexture(GL_TEXTURE_RECTANGLE, gdepthtex);
        glActiveTexture_(GL_TEXTURE0);
        SETSHADER(godRayUpsample);
        LOCALPARAMF(godrayScale,
                    float(vieww)/float(bufferWidth), float(viewh)/float(bufferHeight),
                    float(bufferWidth)/float(vieww), float(bufferHeight)/float(viewh));
        LOCALPARAMF(bilateralDepthScale, 1.0f / max(float(farplane) * godraysupscaleedge, 1.0e-4f));
        screenquad(bufferWidth, bufferHeight, vieww, viewh);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDisable(GL_BLEND);

        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);

        glActiveTexture_(GL_TEXTURE0);

        endtimer(godRayTimer);
    }
}
