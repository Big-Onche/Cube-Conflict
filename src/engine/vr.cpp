// vr.cpp: where wierd vr implementation occurs
// thanks to q009 for the hints ( https://github.com/q009/TesseractVR/ )

#include <openvr.h>
#include "engine.h"
#include "vr.h"

VARFP(virtualreality, 0, 0, 0, vr::init() );

namespace vr
{
    vr::IVRSystem* vrSystem = nullptr;

    enum {EYE_LEFT = 0, EYE_RIGHT, NUMEYES};

    vrbuffer eyeBuffer[NUMEYES];

    bool isEnabled() { return virtualreality && vrSystem; }

    bool initBuffers(uint32_t width, uint32_t height)
    {
        loopi(NUMEYES)
        {
            glGenFramebuffers_(1, &eyeBuffer[i].resolvefb);
            glBindFramebuffer_(GL_FRAMEBUFFER, eyeBuffer[i].resolvefb);

            glGenTextures(1, &eyeBuffer[i].resolvetex);
            glBindTexture(GL_TEXTURE_2D, eyeBuffer[i].resolvetex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, eyeBuffer[i].resolvetex, 0);

            GLenum res = glCheckFramebufferStatus_(GL_FRAMEBUFFER);
            if(res != GL_FRAMEBUFFER_COMPLETE)
            {
                conoutf(CON_ERROR, "Failed to create VR buffer for eye %d: 0x%X", i, res);
                return false;
            }
        }
        glBindFramebuffer_(GL_FRAMEBUFFER, 0);
        return true;
    }

    void freeBuffers()
    {
        loopi(NUMEYES)
        {
            if(eyeBuffer[i].resolvetex)
            {
                glDeleteTextures(1, &eyeBuffer[i].resolvetex);
                eyeBuffer[i].resolvetex = 0;
            }
            if(eyeBuffer[i].resolvefb)
            {
                glDeleteFramebuffers_(1, &eyeBuffer[i].resolvefb);
                eyeBuffer[i].resolvefb = 0;
            }
        }
    }

    bool init()
    {
        if(!virtualreality) return false;

        vr::EVRInitError eError = vr::VRInitError_None;
        vrSystem = vr::VR_Init(&eError, vr::VRApplication_Scene);
        if(eError != vr::VRInitError_None)
        {
            vrSystem = nullptr;
            conoutf(CON_ERROR,"Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
            return false;
        }
        uint32_t renderWidth, renderHeight;
        vrSystem->GetRecommendedRenderTargetSize(&renderWidth, &renderHeight);
        conoutf("%d; %d", renderWidth, renderHeight);

        if(!initBuffers(screenw, screenh))
        {
            conoutf(CON_ERROR, "Failed to initialize VR eye buffers.");
            return false;
        }
        return true;
    }

    void getYawPitchRoll(const matrix4& matrix, float& yaw, float& pitch, float& roll)
    {
        pitch = asin(-matrix.c.y);
        if (cos(pitch) != 0.0f)
        {
            roll = -atan2(matrix.c.x, matrix.c.z);
            yaw = -atan2(matrix.b.y, matrix.a.y);
        }
        else
        {
            roll = 0.0f; // Gimbal lock scenario
            yaw = -atan2(-matrix.a.x, matrix.b.x); // Adjusted for gimbal lock
        }
    }

    matrix4 matrixrh2lh(matrix4 m)
    {
        matrix4 result;

        result.muld(invviewmatrix, m);
        result.muld(matrix4(vec(-1, 0, 0), vec(0, -1, 0), vec(0, 0, -1)));
        result.rotate_around_x(90 * RAD);

        return result;
    }

    static matrix4 convertSteamMatrix(vr::HmdMatrix34_t m)
    {
        return matrixrh2lh(matrix4x3(vec(m.m[0][0], m.m[1][0], m.m[2][0]),
                                         vec(m.m[0][1], m.m[1][1], m.m[2][1]),
                                         vec(m.m[0][2], m.m[1][2], m.m[2][2]),
                                         vec(m.m[0][3], m.m[1][3], m.m[2][3])));
    }

    void updatePoses()
    {
        vr::TrackedDevicePose_t trackedDevicePose[vr::k_unMaxTrackedDeviceCount];
        vr::VRCompositor()->WaitGetPoses(trackedDevicePose, vr::k_unMaxTrackedDeviceCount, nullptr, 0);

        if(trackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
        {
            vr::TrackedDevicePose_t& hmdPose = trackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd];

            matrix4 hmdMatrix = convertSteamMatrix(hmdPose.mDeviceToAbsoluteTracking);  // Convert the pose to a matrix

            float yaw, pitch, roll; // Extract yaw, pitch, roll from the matrix
            getYawPitchRoll(hmdMatrix, yaw, pitch, roll);

            camera1->yaw = yaw * 100.f;
            camera1->pitch = pitch * 100.f;
            camera1->roll = roll * 100.f;

            player->yaw = camera1->yaw;
            player->pitch = camera1->pitch;
            player->roll = camera1->roll;
        }
    }

VARP(ipd, -500, 0, 500); // 300

void blitToResolveBuffers() {
    glBindFramebuffer_(GL_FRAMEBUFFER, 0); // Use the default framebuffer as the source.

    for (int i = 0; i < NUMEYES; i++) {
        glBindFramebuffer_(GL_DRAW_FRAMEBUFFER, eyeBuffer[i].resolvefb); // Bind the framebuffer for each eye.

        // Calculate offset based on eye index.
        int offsetX = (i == EYE_LEFT ? -ipd : ipd);

        // Ensure offsets are clamped to prevent out-of-bounds rendering.
        offsetX = std::max(0, std::min(screenw, offsetX));

        // Blit the screen to each eye's framebuffer with horizontal offset.
        glBlitFramebuffer_(0, 0, screenw, screenh,  // source coordinates
                           offsetX + ipd/2, 0, screenw + offsetX, screenh,  // adjusted destination coordinates
                           GL_COLOR_BUFFER_BIT, GL_LINEAR);
    }

    glBindFramebuffer_(GL_FRAMEBUFFER, 0); // Unbind any FBO.
}

    void update()
    {
        if(!isEnabled()) return;
        updatePoses();
    }

    void render()
    {
        if(!isEnabled()) return;

        blitToResolveBuffers();

        loopi(NUMEYES)
        {
            vr::Texture_t eyeTexture = { (void*)(uintptr_t)eyeBuffer[i].resolvetex, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };

            vr::EVRCompositorError error = vr::VRCompositor()->Submit(!i ? vr::Eye_Left : vr::Eye_Right, &eyeTexture);
            if(error != vr::VRCompositorError_None) conoutf(CON_ERROR, "Error submitting texture for eye %d: %d", i, error);
        }

        vr::VRCompositor()->PostPresentHandoff();
    }

    void clean()
    {
        freeBuffers();
        if(vrSystem)
        {
            vr::VR_Shutdown();
            vrSystem = nullptr;
        }
    }
}
