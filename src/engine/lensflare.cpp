// flares.cpp: procedural sun flares and shafts

#include "engine.h"

extern GLuint hdrfbo, mshdrfbo;
extern float getfovscale(float referenceFov);

namespace lensFlares
{
    static const float occlusionRadius = 10.0f;

    struct queuedFlare
    {
        vec o;
        bvec color;
        float size;
        int maxDistance;
        bool unlimitedDistance;
        bool lensGhosts;
    };

    static vector<queuedFlare> queuedFlares;

    // Settings
    VARP(flares, 0, 1, 1);
    VARP(flareghosts, 0, 1, 1);
    VARP(sunflares, 0, 1, 1);
    VARP(flarestrength, 0, 0, 3);  // subtle, normal, intense or cinematic
    FVAR(sunflarehalo, 0.0f, 0.2f, 4.0f);
    FVAR(flarebranchthickness, 0.25f, 8.0f, 16.0f);

    // Map vars
    VARR(sunflareshaftsize, 1, 50, 400);
    VARR(sunflarestrength, 0, 50, 200);
    CVARR(sunflarecolour, 0x000000);

    static bool shouldRender(bool sun = false)
    {
        if(!flares || (sun && sunflarestrength <= 0)) return false;
        return !sun || (sunflares && !sunlight.iszero() && sunlightscale > 1.0e-4f);
    }

    void addFlares(const vec &o, int color, float size, bool unlimitedDistance, bool lensGhosts, int maxDistance)
    {
        if(!shouldRender()) return;
        queuedFlare &f = queuedFlares.add();
        f.o = o;
        f.color = bvec::hexcolor(color);
        f.size = max(size, 0.0f);
        f.maxDistance = maxDistance;
        f.unlimitedDistance = unlimitedDistance;
        f.lensGhosts = lensGhosts;
    }

    void cleanup()
    {
        queuedFlares.setsize(0);
    }

    static void drawFlare(Shader *flareShader, const vec4 &screen, const vec4 &params, const vec &color, float ghostStrength, const vec4 &layerWeights, const vec4 &visibilityOverride, float sizeScale)
    {
        GLOBALPARAMF(sunFlareScreen, screen.x, screen.y, screen.z, screen.w);
        GLOBALPARAMF(sunFlareParams, params.x, params.y, params.z, params.w);
        GLOBALPARAMF(sunFlareGhostStrength, ghostStrength);
        GLOBALPARAMF(sunFlareSizeScale, sizeScale);
        GLOBALPARAMF(sunFlareLayerWeights, layerWeights.x, layerWeights.y, layerWeights.z, layerWeights.w);
        GLOBALPARAMF(sunFlareVisibilityOverride, visibilityOverride.x, visibilityOverride.y, visibilityOverride.z, visibilityOverride.w);
        GLOBALPARAM(sunFlareColor, color);
        flareShader->set();
        screenquad(vieww, viewh);
    }

    static float projectedRadiusPixels(const vec &center, const vec2 &centerNdc, float worldRadius)
    {
        float radiusPixels = 0.0f;

        vec sample(center);
        sample.madd(camright, worldRadius);
        vec4 sampleClip;
        camprojmatrix.transform(sample, sampleClip);
        if(sampleClip.w > 1.0e-4f)
        {
            vec2 sampleNdc(sampleClip.x / sampleClip.w, sampleClip.y / sampleClip.w);
            vec2 delta(sampleNdc.x - centerNdc.x, sampleNdc.y - centerNdc.y);
            radiusPixels = max(radiusPixels, 0.5f * sqrtf(delta.x*delta.x*vieww*vieww + delta.y*delta.y*viewh*viewh));
        }

        sample = center;
        sample.madd(camup, worldRadius);
        camprojmatrix.transform(sample, sampleClip);
        if(sampleClip.w > 1.0e-4f)
        {
            vec2 sampleNdc(sampleClip.x / sampleClip.w, sampleClip.y / sampleClip.w);
            vec2 delta(sampleNdc.x - centerNdc.x, sampleNdc.y - centerNdc.y);
            radiusPixels = max(radiusPixels, 0.5f * sqrtf(delta.x*delta.x*vieww*vieww + delta.y*delta.y*viewh*viewh));
        }

        return max(radiusPixels, 1.0f);
    }

    static float getFlareStrength(float sourceBoost)
    {
        return ((sunflarestrength / 50.0f) * sourceBoost) * (0.15f + (0.15f * flarestrength));
    }

    static bool initSun(vec4 &sunScreen, vec4 &sunParams, vec &sunColor, float &ghostStrength, vec4 &layerWeights, vec4 &visibilityOverride, float &sizeScale)
    {
        if(!shouldRender(true)) return false;

        vec sunPoint(camera1->o);
        sunPoint.madd(sunlightdir, max(nearplane()*4.0f, 1.0f));

        vec4 sunClip;
        camprojmatrix.transform(sunPoint, sunClip);
        if(sunClip.w <= 1.0e-4f || sunClip.z < -sunClip.w) return false;

        vec2 sunNdc(sunClip.x / sunClip.w, sunClip.y / sunClip.w);
        if(fabsf(sunNdc.x) > 1.35f || fabsf(sunNdc.y) > 1.35f) return false;

        float screenEdge = max(fabsf(sunNdc.x), fabsf(sunNdc.y));
        float edgeFade = clamp(1.0f - max(screenEdge - 0.90f, 0.0f) / 0.40f, 0.0f, 1.0f);
        float horizonFade = clamp((sunlightdir.z - 0.02f) / 0.10f, 0.0f, 1.0f);
        float screenFade = edgeFade * horizonFade;
        if(screenFade <= 1.0e-4f) return false;

        float shaftScale = max(sunflareshaftsize / 100.0f, 0.01f);
        sunScreen = vec4(sunNdc.x * 0.5f + 0.5f, sunNdc.y * 0.5f + 0.5f, screenFade, shaftScale);

        vec baseColor = sunflarecolour.iszero() ? sunlight.tocolor() : sunflarecolour.tocolor();
        float colorMax = max(max(baseColor.x, baseColor.y), baseColor.z);
        if(colorMax <= 1.0e-4f) return false;

        float sunScale = max(sunlightscale, 0.0f);
        float sunLuma = (0.2126f * baseColor.x + 0.7152f * baseColor.y + 0.0722f * baseColor.z) * sunScale;
        if(sunLuma <= 1.0e-4f) return false;

        sunColor = vec(baseColor).mul(1.0f / colorMax);

        float sourceBoost = clamp(0.5f + sqrtf(sunLuma), 0.5f, 4.0f);
        float strength = getFlareStrength(sourceBoost);
        sunParams = vec4(strength, colorMax, lastmillis / 1000.0f, sourceBoost);
        ghostStrength = flareghosts ? 0.5f : 0.0f;
        layerWeights = vec4(1.0f, sunflarehalo, 1.0f, flareghosts ? 1.0f : 0.0f);
        visibilityOverride = vec4(-1.0f, -1.0f, 0.0f, 0.0f);
        sizeScale = 1.0f;
        return true;
    }

    static bool initFlare(const queuedFlare &source, vec4 &flareScreen, vec4 &flareParams, vec &flareColor, float &ghostStrength, vec4 &layerWeights, vec4 &visibilityOverride, float &sizeScale)
    {
        // Note: shouldRender() is intentionally not rechecked here.
        // addFlares() already gates on it, so queuedFlares is never populated when rendering is disabled.
        if(isvisiblesphere(0.0f, source.o) > (source.unlimitedDistance ? VFC_FOGGED : VFC_FULL_VISIBLE)) return false;

        vec flaredir(source.o);
        flaredir.sub(camera1->o);
        float flareDistance = flaredir.magnitude();
        if(flareDistance <= 1.0e-4f) return false;

        vec4 flareClip;
        camprojmatrix.transform(source.o, flareClip);
        if(flareClip.w <= 1.0e-4f || flareClip.z < -flareClip.w) return false;

        vec2 flareNdc(flareClip.x / flareClip.w, flareClip.y / flareClip.w);
        if(fabsf(flareNdc.x) > 1.35f || fabsf(flareNdc.y) > 1.35f) return false;

        float screenEdge = max(fabsf(flareNdc.x), fabsf(flareNdc.y));
        float edgeFade = clamp(1.0f - max(screenEdge - 0.90f, 0.0f) / 0.40f, 0.0f, 1.0f);
        float distanceFade = 1.0f;
        if(!source.unlimitedDistance)
        {
            float maxDistance = max(float(source.maxDistance), 1.0f);
            distanceFade = clamp(1.0f - flareDistance / maxDistance, 0.0f, 1.0f);
            if(distanceFade <= 1.0e-4f) return false;
        }

        float screenFade = edgeFade * distanceFade;
        if(screenFade <= 1.0e-4f) return false;

        float referenceDistance = max(float(source.maxDistance), 1.0f);
        float effectiveDistance = max(referenceDistance + (flareDistance - referenceDistance) / 3.0f, 1.0f);
        sizeScale = clamp((source.size / 100.0f) * (referenceDistance / effectiveDistance), 0.01f, 16.0f);
        flareScreen = vec4(flareNdc.x * 0.5f + 0.5f, flareNdc.y * 0.5f + 0.5f, screenFade, sizeScale);

        vec baseColor(source.color.r / 255.0f, source.color.g / 255.0f, source.color.b / 255.0f);
        float colorMax = max(max(baseColor.x, baseColor.y), baseColor.z);
        if(colorMax <= 1.0e-4f) return false;

        flareColor = vec(baseColor).mul(1.0f / colorMax);

        float flareLuma = 0.2126f * baseColor.x + 0.7152f * baseColor.y + 0.0722f * baseColor.z;
        float sourceBoost = clamp(0.5f + sqrtf(flareLuma), 0.5f, 2.5f);
        float strength = getFlareStrength(sourceBoost);
        vec2 linearDepthScale = projmatrix.lineardepthscale();
        float sourceDepth = linearDepthScale.x*flareClip.z + linearDepthScale.y*flareClip.w;
        float occlusionRadiusPixels = projectedRadiusPixels(source.o, flareNdc, occlusionRadius);
        float ghostLayer = flareghosts && source.lensGhosts ? 1.0f : 0.0f;
        flareParams = vec4(strength, colorMax, lastmillis / 1000.0f, sourceBoost);
        ghostStrength = ghostLayer ? 0.5f : 0.0f;
        layerWeights = vec4(0.0f, 0.0f, 1.0f, ghostLayer);
        visibilityOverride = vec4(-1.0f, -1.0f, sourceDepth, occlusionRadiusPixels);
        return true;
    }

    void render()
    {
        vec4 sunScreen, sunParams, sunLayerWeights, sunVisibilityOverride;
        vec sunColor;
        float sunGhostStrength = 0.0f;
        float sunSizeScale = 1.0f;
        bool renderSun = initSun(sunScreen, sunParams, sunColor, sunGhostStrength, sunLayerWeights, sunVisibilityOverride, sunSizeScale);

        if(!renderSun && queuedFlares.empty())
        {
            queuedFlares.setsize(0);
            return;
        }

        Shader *flareShader = useshaderbyname("lensflare");
        if(!flareShader)
        {
            queuedFlares.setsize(0);
            return;
        }

        bool hadScissor = glIsEnabled(GL_SCISSOR_TEST) != 0;
        bool hadDepth = glIsEnabled(GL_DEPTH_TEST) != 0;
        bool hadBlend = glIsEnabled(GL_BLEND) != 0;
        GLint oldBlendSrcRGB, oldBlendDstRGB, oldBlendSrcAlpha, oldBlendDstAlpha;
        glGetIntegerv(GL_BLEND_SRC_RGB, &oldBlendSrcRGB);
        glGetIntegerv(GL_BLEND_DST_RGB, &oldBlendDstRGB);
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &oldBlendSrcAlpha);
        glGetIntegerv(GL_BLEND_DST_ALPHA, &oldBlendDstAlpha);

        if(hadScissor) glDisable(GL_SCISSOR_TEST);
        if(hadDepth) glDisable(GL_DEPTH_TEST);

        glActiveTexture_(GL_TEXTURE0);
        if(msaalight) glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msdepthtex);
        else glBindTexture(GL_TEXTURE_RECTANGLE, gdepthtex);
        glActiveTexture_(GL_TEXTURE1);
        if(msaalight) glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msnormaltex);
        else glBindTexture(GL_TEXTURE_RECTANGLE, gnormaltex);
        glActiveTexture_(GL_TEXTURE0);

        glBindFramebuffer_(GL_FRAMEBUFFER, msaalight ? mshdrfbo : hdrfbo);
        glViewport(0, 0, vieww, viewh);
        glEnable(GL_BLEND);
        if(glBlendFuncSeparate_) glBlendFuncSeparate_(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE);
        else glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        GLOBALPARAMF(sunFlareFovScale, getfovscale(100.0f));
        GLOBALPARAMF(sunFlareBranchThickness, max(flarebranchthickness, 0.05f));
        if(renderSun) drawFlare(flareShader, sunScreen, sunParams, sunColor, sunGhostStrength, sunLayerWeights, sunVisibilityOverride, sunSizeScale);
        loopv(queuedFlares)
        {
            vec4 flareScreen, flareParams, layerWeights, visibilityOverride;
            vec flareColor;
            float ghostStrength = 0.0f;
            float flareSizeScale = 1.0f;
            if(initFlare(queuedFlares[i], flareScreen, flareParams, flareColor, ghostStrength, layerWeights, visibilityOverride, flareSizeScale))
                drawFlare(flareShader, flareScreen, flareParams, flareColor, ghostStrength, layerWeights, visibilityOverride, flareSizeScale);
        }

        if(glBlendFuncSeparate_) glBlendFuncSeparate_(oldBlendSrcRGB, oldBlendDstRGB, oldBlendSrcAlpha, oldBlendDstAlpha);
        else glBlendFunc(oldBlendSrcRGB, oldBlendDstRGB);
        if(!hadBlend) glDisable(GL_BLEND);
        if(hadDepth) glEnable(GL_DEPTH_TEST);
        if(hadScissor) glEnable(GL_SCISSOR_TEST);
        queuedFlares.setsize(0);
    }
}
