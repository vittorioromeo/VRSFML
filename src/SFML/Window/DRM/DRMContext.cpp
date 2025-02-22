#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/DRM/DRMContext.hpp"
#include "SFML/Window/DRM/WindowImplDRM.hpp"
#include "SFML/Window/VideoModeUtils.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Sleep.hpp"

#include "SFML/Base/Abort.hpp"
#include "SFML/Base/Builtins/Memcpy.hpp"

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <xf86drm.h>

#include <cerrno>

// We check for this definition in order to avoid multiple definitions of GLAD
// entities during unity builds of SFML.
#ifndef GLAD_EGL_IMPLEMENTATION_INCLUDED
#define GLAD_EGL_IMPLEMENTATION_INCLUDED
#define GLAD_EGL_IMPLEMENTATION
#include <glad/egl.h>
#endif

namespace
{
struct DrmFb
{
    gbm_bo*   bo{};
    base::U32 fbId{};
};

bool            initialized = false;
sf::priv::Drm   drmNode;
drmEventContext drmEventCtx{};
pollfd          pollFD{};
gbm_device*     gbmDevice      = nullptr;
int             contextCount   = 0;
EGLDisplay      display        = EGL_NO_DISPLAY;
int             waitingForFlip = 0;

void pageFlipHandler(int /* fd */, unsigned int /* frame */, unsigned int /* sec */, unsigned int /* usec */, void* data)
{
    int* temp = static_cast<int*>(data);
    *temp     = 0;
}

bool waitForFlip(int timeout)
{
    while (waitingForFlip)
    {
        pollFD.revents = 0;

        if (poll(&pollFD, 1, timeout) < 0)
            return false;

        if (pollFD.revents & (POLLHUP | POLLERR))
            return false;

        if (pollFD.revents & POLLIN)
        {
            drmHandleEvent(drmNode.fileDescriptor, &drmEventCtx);
        }
        else
        {
            return false;
        }
    }
    return true;
}

void cleanup()
{
    if (!initialized)
        return;

    drmModeSetCrtc(drmNode.fileDescriptor,
                   drmNode.originalCrtc->crtc_id,
                   drmNode.originalCrtc->buffer_id,
                   drmNode.originalCrtc->x,
                   drmNode.originalCrtc->y,
                   &drmNode.connectorId,
                   1,
                   &drmNode.originalCrtc->mode);

    drmModeFreeConnector(drmNode.savedConnector);
    drmModeFreeEncoder(drmNode.savedEncoder);
    drmModeFreeCrtc(drmNode.originalCrtc);

    if (display != EGL_NO_DISPLAY)
        eglCheck(eglTerminate(display));

    display = EGL_NO_DISPLAY;

    gbm_device_destroy(gbmDevice);
    gbmDevice = nullptr;

    close(drmNode.fileDescriptor);

    drmNode.fileDescriptor = -1;
    drmNode.mode           = nullptr;

    pollFD      = {};
    drmEventCtx = {};

    waitingForFlip = 0;

    initialized = false;
}

void drmFbDestroyCallback(gbm_bo* bo, void* data)
{
    const int drmFd = gbm_device_get_fd(gbm_bo_get_device(bo));
    auto*     fb    = static_cast<DrmFb*>(data);

    if (fb->fbId)
        drmModeRmFB(drmFd, fb->fbId);

    delete fb;
}

DrmFb* drmFbGetFromBo(gbm_bo& bo)
{
    const int drmFd = gbm_device_get_fd(gbm_bo_get_device(&bo));
    auto*     fb    = static_cast<DrmFb*>(gbm_bo_get_user_data(&bo));
    if (fb)
        return fb;

    fb     = new DrmFb();
    fb->bo = &bo;

    const sf::base::U32 width  = gbm_bo_get_width(&bo);
    const sf::base::U32 height = gbm_bo_get_height(&bo);
    const sf::base::U32 format = gbm_bo_get_format(&bo);

    sf::base::U32 strides[4]   = {0};
    sf::base::U32 handles[4]   = {0};
    sf::base::U32 offsets[4]   = {0};
    sf::base::U64 modifiers[4] = {0};
    modifiers[0]               = gbm_bo_get_modifier(&bo);
    const int numPlanes        = gbm_bo_get_plane_count(&bo);
    for (int i = 0; i < numPlanes; ++i)
    {
        strides[i]   = gbm_bo_get_stride_for_plane(&bo, i);
        handles[i]   = gbm_bo_get_handle(&bo).u32;
        offsets[i]   = gbm_bo_get_offset(&bo, i);
        modifiers[i] = modifiers[0];
    }

    const sf::base::U32 flags = modifiers[0] ? DRM_MODE_FB_MODIFIERS : 0;

    int result = drmModeAddFB2WithModifiers(drmFd, width, height, format, handles, strides, offsets, modifiers, &fb->fbId, flags);

    if (result)
    {
        std::memset(handles, 0, 16);
        handles[0] = gbm_bo_get_handle(&bo).u32;
        std::memset(strides, 0, 16);
        strides[0] = gbm_bo_get_stride(&bo);
        std::memset(offsets, 0, 16);
        result = drmModeAddFB2(drmFd, width, height, format, handles, strides, offsets, &fb->fbId, 0);
    }

    if (result)
    {
        sf::priv::err() << "Failed to create fb: " << std::strerror(errno);
        delete fb;
        return nullptr;
    }

    gbm_bo_set_user_data(&bo, fb, drmFbDestroyCallback);

    return fb;
}

sf::base::U32 findCrtcForEncoder(const drmModeRes& resources, const drmModeEncoder& encoder)
{
    for (int i = 0; i < resources.count_crtcs; ++i)
    {
        // Possible_crtcs is a bitmask as described here:
        // https://dvdhrm.wordpress.com/2012/09/13/linux-drm-mode-setting-api
        const sf::base::U32 crtcMask = 1u << i;
        const sf::base::U32 crtcId   = resources.crtcs[i];
        if (encoder.possible_crtcs & crtcMask)
        {
            return crtcId;
        }
    }

    // No match found
    return 0;
}

sf::base::U32 findCrtcForConnector(const sf::priv::Drm& drm, const drmModeRes& resources, const drmModeConnector& connector)
{
    for (int i = 0; i < connector.count_encoders; ++i)
    {
        const sf::base::U32 encoderId = connector.encoders[i];
        if (auto* encoder = drmModeGetEncoder(drm.fileDescriptor, encoderId))
        {
            const sf::base::U32 crtcId = findCrtcForEncoder(resources, *encoder);

            drmModeFreeEncoder(encoder);
            if (crtcId != 0)
            {
                return crtcId;
            }
        }
    }

    // No match found
    return 0;
}

int getResources(int fd, drmModeResPtr& resources)
{
    resources = drmModeGetResources(fd);
    if (resources == nullptr)
        return -1;
    return 0;
}

int hasMonitorConnected(int fd, drmModeRes& resources)
{
    drmModeConnectorPtr connector = nullptr;
    for (int i = 0; i < resources.count_connectors; ++i)
    {
        connector = drmModeGetConnector(fd, resources.connectors[i]);
        if (connector->connection == DRM_MODE_CONNECTED)
        {
            // There is a monitor connected
            drmModeFreeConnector(connector);
            connector = nullptr;
            return 1;
        }
        drmModeFreeConnector(connector);
        connector = nullptr;
    }
    return 0;
}

int findDrmDevice(drmModeResPtr& resources)
{
    static const int maxDrmDevices = 64;

    drmDevicePtr devices[maxDrmDevices] = {};

    const int numDevices = drmGetDevices2(0, devices, maxDrmDevices);
    if (numDevices < 0)
    {
        sf::priv::err() << "drmGetDevices2 failed: " << std::strerror(-numDevices);
        return -1;
    }

    int fileDescriptor = -1;
    for (int i = 0; i < numDevices; ++i)
    {
        drmDevicePtr device = devices[i];
        int          result = 0;

        if (!(device->available_nodes & (1 << DRM_NODE_PRIMARY)))
            continue;
        // OK, it's a primary device. If we can get the drmModeResources, it means it's also a KMS-capable device.
        fileDescriptor = open(device->nodes[DRM_NODE_PRIMARY], O_RDWR);
        if (fileDescriptor < 0)
            continue;
        result = getResources(fileDescriptor, resources);
#ifdef SFML_DEBUG
        sf::priv::err() << "DRM device used: " << i;
#endif
        if (!result && hasMonitorConnected(fileDescriptor, *resources) != 0)
        {
#ifdef SFML_DEBUG
            sf::err() << "DRM device used: " << device->nodes[DRM_NODE_PRIMARY] << std::endl;
#endif
            break;
        }
        close(fileDescriptor);
        fileDescriptor = -1;
    }
    drmFreeDevices(devices, numDevices);

    if (fileDescriptor < 0)
        sf::priv::err() << "No drm device found!";
    return fileDescriptor;
}

int initDrm()
{
    if (initialized)
        return 0;

    drmModeResPtr resources = nullptr;

    // Use environment variable "SFML_DRM_DEVICE" (or nullptr if not set)
    const char* deviceStr = std::getenv("SFML_DRM_DEVICE");
    if (deviceStr && *deviceStr)
    {
        drmNode.fileDescriptor = open(deviceStr, O_RDWR);
        const int ret          = getResources(drmNode.fileDescriptor, resources);
        if (ret < 0 && errno == EOPNOTSUPP)
            sf::priv::err() << device << " does not look like a modeset device";
    }
    else
    {
        drmNode.fileDescriptor = findDrmDevice(resources);
    }

    if (drmNode.fileDescriptor < 0)
    {
        sf::priv::err() << "Could not open drm device";
        return -1;
    }

    if (!resources)
    {
        sf::priv::err() << "drmModeGetResources failed: " << std::strerror(errno);
        return -1;
    }

    // Find a connected connector:
    drmModeConnectorPtr connector = nullptr;
    for (int i = 0; i < resources->count_connectors; ++i)
    {
        connector = drmModeGetConnector(drmNode.fileDescriptor, resources->connectors[i]);
        if (connector->connection == DRM_MODE_CONNECTED)
        {
            // It's connected, let's use this!
            break;
        }
        drmModeFreeConnector(connector);
        connector = nullptr;
    }

    if (!connector)
    {
        // We could be fancy and listen for hotplug events and wait for a connector..
        sf::priv::err() << "No connected connector!";
        return -1;
    }

    // Find user requested mode:
    if (modeStr && *modeStr)
    {
        for (int i = 0; i < connector->count_modes; ++i)
        {
            drmModeModeInfoPtr currentMode = &connector->modes[i];

            if (std::string_view(currentMode->name) == modeStr)
            {
                if (vrefresh == 0 || currentMode->vrefresh == vrefresh)
                {
                    drm.mode = currentMode;
                    break;
                }
            }
        }
        if (!drm.mode)
            sf::priv::err() << "Requested mode not found, using default mode!";
    }

    // Find encoder:
    drmModeEncoderPtr encoder = nullptr;
    for (int i = 0; i < resources->count_encoders; ++i)
    {
        encoder = drmModeGetEncoder(drmNode.fileDescriptor, resources->encoders[i]);
        if (encoder->encoder_id == connector->encoder_id)
            break;
        drmModeFreeEncoder(encoder);
        encoder = nullptr;
    }

    if (encoder)
    {
        drmNode.crtcId = encoder->crtc_id;
    }
    else
    {
        const base::U32 crtcId = findCrtcForConnector(drm, *resources, *connector);
        if (crtcId == 0)
        {
            sf::priv::err() << "No crtc found!";
            return -1;
        }

        drmNode.crtcId = crtcId;
    }

    drmModeFreeResources(resources);

    drmNode.connectorId = connector->connector_id;

    drmNode.savedConnector = connector;
    drmNode.savedEncoder   = encoder;

    // Get original display mode so we can restore display mode after program exits
    drm.originalCrtc = drmModeGetCrtc(drm.fileDescriptor, drm.crtcId);

    // Use the current mode rather than the preferred one if the user didn't specify a mode with env vars
    if (!drm.mode)
    {
#ifdef SFML_DEBUG
        sf::priv::err() << "DRM using the current mode";
#endif
        drm.mode = &(drm.originalCrtc->mode);
    }

#ifdef SFML_DEBUG
    sf::priv::err() << "DRM Mode used: " << drm.mode->name << "@" << drm.mode->vrefresh;
#endif

    return 0;
}

void checkInit()
{
    if (initialized)
        return;

    // Use environment variable "SFML_DRM_DEVICE" (or nullptr if not set)
    char* deviceString = std::getenv("SFML_DRM_DEVICE");
    if (deviceString && !*deviceString)
        deviceString = nullptr;

    // Use environment variable "SFML_DRM_MODE" (or nullptr if not set)
    char* modeString = std::getenv("SFML_DRM_MODE");

    // Use environment variable "SFML_DRM_REFRESH" (or 0 if not set)
    // Use in combination with mode to request specific refresh rate for the mode
    // if multiple refresh rates for same mode might be supported
    unsigned int refreshRate = 0;
    if (const char* refreshString = std::getenv("SFML_DRM_REFRESH"))
        refreshRate = static_cast<unsigned int>(std::atoi(refreshString));

    if (initDrm(drmNode,
                deviceString,     // device
                modeString,       // requested mode
                refreshRate) < 0) // screen refresh rate
    {
        sf::priv::err() << "Error initializing DRM";
        return;
    }

    gbmDevice = gbm_create_device(drmNode.fileDescriptor);

    std::atexit(cleanup);
    initialized = true;

    pollFD.fd                     = drmNode.fileDescriptor;
    pollFD.events                 = POLLIN;
    drmEventCtx.version           = 2;
    drmEventCtx.page_flip_handler = pageFlipHandler;

    drmNode.mode = nullptr;

    return 1;
}

void setDrmMode(sf::Vector2u size = {})
{
    // don't do anything if supplied width and height are 0 and we already have a drm mode
    if ((size == sf::Vector2u()) && drmNode.mode)
        return;

    drmModeConnectorPtr connector = drmNode.savedConnector;
    if (!connector)
        return;

    // Find user requested mode:
    // Use environment variable "SFML_DRM_MODE" (or nullptr if not set)
    const char* modeString = std::getenv("SFML_DRM_MODE");

    // Use environment variable "SFML_DRM_REFRESH" (or 0 if not set)
    // Use in combination with mode to request specific refresh rate for the mode
    // if multiple refresh rates for same mode might be supported
    unsigned int refreshRate   = 0;
    const char*  refreshString = std::getenv("SFML_DRM_REFRESH");

    if (refreshString)
        refreshRate = static_cast<unsigned int>(std::atoi(refreshString));

    bool matched = false;
    for (int i = 0; i < connector->count_modes; ++i)
    {
        drmModeModeInfoPtr currentMode = &connector->modes[i];

        if (refreshRate == 0 || currentMode->vrefresh == refreshRate)
        {
            // prefer SFML_DRM_MODE setting if matched
            if ((modeString && *modeString) && (std::strcmp(currentMode->name, modeString) == 0))
            {
                drmNode.mode = currentMode;
                break;
            }
            // otherwise match to program supplied width and height
            if (!matched && sf::Vector2u(currentMode->hdisplay, currentMode->vdisplay) == size)
            {
                drmNode.mode = currentMode;
                matched      = true;
            }
        }
    }
    if ((modeString && *modeString) && !drmNode.mode)
        sf::err() << "SFML_DRM_MODE (" << modeString << ") not found, using default mode!" << std::endl;

    // Let's use the current mode rather than the preferred one if the user didn't specify a mode with env vars
    if (!drmNode.mode)
        drmNode.mode = &(drmNode.originalCrtc->mode);

#ifdef SFML_DEBUG
    sf::err() << "DRM Mode used: " << drmNode.mode->name << "@" << drmNode.mode->vrefresh << std::endl;
#endif
}

EGLDisplay getInitializedDisplay()
{
    if (display == EGL_NO_DISPLAY)
    {
        gladLoaderLoadEGL(EGL_NO_DISPLAY);

        display = eglCheck(eglGetDisplay(reinterpret_cast<EGLNativeDisplayType>(gbmDevice)));

        EGLint major = 0;
        EGLint minor = 0;
        eglCheck(eglInitialize(display, &major, &minor));

        gladLoaderLoadEGL(display);

#if defined(SFML_OPENGL_ES)
        if (!eglBindAPI(EGL_OPENGL_ES_API))
        {
            sf::priv::err() << "failed to bind api EGL_OPENGL_ES_API";
        }
#else
        if (!eglBindAPI(EGL_OPENGL_API))
        {
            sf::priv::err() << "failed to bind api EGL_OPENGL_API";
        }
#endif
    }

    return display;
}
} // namespace


namespace sf::priv
{
////////////////////////////////////////////////////////////
DRMContext::DRMContext(DRMContext* shared)
{
    contextCount++;
    if (initDrm() < 0)
        return;

    setDrmMode();

    // Get the initialized EGL display
    m_display = getInitializedDisplay();

    // Get the best EGL config matching the default video settings
    m_config = getBestConfig(m_display, VideoModeUtils::getDesktopMode().bitsPerPixel, ContextSettings{});
    updateSettings();

    // Create EGL context
    createContext(shared);

    if (shared)
        createSurface(shared->m_size, VideoModeUtils::getDesktopMode().bitsPerPixel, false);
    else // create a surface to force the GL to initialize (seems to be required for glGetString() etc )
        createSurface({1, 1}, VideoModeUtils::getDesktopMode().bitsPerPixel, false);
}


////////////////////////////////////////////////////////////
DRMContext::DRMContext(DRMContext* shared, const ContextSettings& contextSettings, const WindowImpl& owner, unsigned int bitsPerPixel)
{
    contextCount++;
    if (initDrm() < 0)
        return;

    setDrmMode(owner.getSize());

    // Get the initialized EGL display
    m_display = getInitializedDisplay();

    // Get the best EGL config matching the requested video settings
    m_config = getBestConfig(m_display, bitsPerPixel, contextSettings);
    updateSettings();

    // Create EGL context
    createContext(shared);

    createSurface({drmNode.mode->hdisplay, drmNode.mode->vdisplay}, true);
}


////////////////////////////////////////////////////////////
DRMContext::~DRMContext()
{
    // Deactivate the current context
    const EGLContext currentContext = eglCheck(eglGetCurrentContext());

    if (currentContext == m_context)
    {
        eglCheck(eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    }

    // Destroy context
    if (m_context != EGL_NO_CONTEXT)
    {
        eglCheck(eglDestroyContext(m_display, m_context));
        m_context = EGL_NO_CONTEXT;
    }

    // Destroy surface
    if (m_surface != EGL_NO_SURFACE)
    {
        eglCheck(eglDestroySurface(m_display, m_surface));
        m_surface = EGL_NO_SURFACE;
    }

    if (m_currentBO)
        gbm_surface_release_buffer(m_gbmSurface, m_currentBO);

    if (m_nextBO)
        gbm_surface_release_buffer(m_gbmSurface, m_nextBO);

    if (m_gbmSurface)
        gbm_surface_destroy(m_gbmSurface);

    contextCount--;
    if (contextCount == 0)
        cleanup();
}


////////////////////////////////////////////////////////////
bool DRMContext::makeCurrent(bool current)
{
    const EGLSurface surface = current ? m_surface : EGL_NO_SURFACE;
    const EGLContext context = current ? m_context : EGL_NO_CONTEXT;
    return m_surface != EGL_NO_SURFACE && eglCheck(eglMakeCurrent(m_display, surface, surface, context));
}


////////////////////////////////////////////////////////////
void DRMContext::display()
{
    if (m_surface == EGL_NO_SURFACE)
        return;

    if (!m_scanOut)
    {
        eglCheck(eglSwapBuffers(m_display, m_surface));
        return;
    }

    // Handle display of buffer to the screen
    DrmFb* fb = nullptr;

    if (!waitForFlip(-1))
        return;

    if (m_currentBO)
    {
        gbm_surface_release_buffer(m_gbmSurface, m_currentBO);
        m_currentBO = nullptr;
    }

    eglCheck(eglSwapBuffers(m_display, m_surface));

    m_currentBO = m_nextBO;

    // This call must be preceded by a single call to eglSwapBuffers()
    m_nextBO = gbm_surface_lock_front_buffer(m_gbmSurface);

    if (!m_nextBO)
        return;

    fb = drmFbGetFromBo(*m_nextBO);
    if (!fb)
    {
        priv::err() << "Failed to get FB from buffer object";
        return;
    }

    // If first time, need to first call drmModeSetCrtc()
    if (!m_shown)
    {
        if (drmModeSetCrtc(drmNode.fileDescriptor, drmNode.crtcId, fb->fbId, 0, 0, &drmNode.connectorId, 1, drmNode.mode))
        {
            priv::err() << "Failed to set mode: " << std::strerror(errno);
            base::abort();
        }
        m_shown = true;
    }

    // Do page flip
    if (!drmModePageFlip(drmNode.fileDescriptor, drmNode.crtcId, fb->fbId, DRM_MODE_PAGE_FLIP_EVENT, &waitingForFlip))
        waitingForFlip = 1;
}


////////////////////////////////////////////////////////////
void DRMContext::setVerticalSyncEnabled(bool enabled)
{
    eglCheck(eglSwapInterval(m_display, enabled));
}


////////////////////////////////////////////////////////////
void DRMContext::createContext(DRMContext* shared)
{
    constexpr EGLint contextVersion[]{EGL_CONTEXT_CLIENT_VERSION, 1, EGL_NONE};

    const EGLContext toShared = shared ? shared->m_context : EGL_NO_CONTEXT;
    if (toShared != EGL_NO_CONTEXT)
        eglCheck(eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));

    // Create EGL context
    m_context = eglCheck(eglCreateContext(m_display, m_config, toShared, contextVersion));
    if (m_context == EGL_NO_CONTEXT)
        priv::err() << "Failed to create EGL context";
}


////////////////////////////////////////////////////////////
void DRMContext::createSurface(Vector2u size, unsigned int /* bpp */, bool scanout)
{
    base::U32 flags = GBM_BO_USE_RENDERING;

    m_scanOut = scanout;
    if (m_scanOut)
        flags |= GBM_BO_USE_SCANOUT;

    if (!gbm_device_is_format_supported(gbmDevice, fmt, flags))
        err() << "Warning: GBM surface format not supported." << std::endl;

    m_gbmSurface = gbm_surface_create(gbmDevice, size.x, size.y, fmt, flags);

    if (!m_gbmSurface)
    {
        priv::err() << "Failed to create gbm surface.";
        return;
    }

    eglCheck(
        m_surface = eglCreateWindowSurface(m_display, m_config, reinterpret_cast<EGLNativeWindowType>(m_gbmSurface), nullptr));

    if (m_surface == EGL_NO_SURFACE)
    {
        priv::err() << "Failed to create EGL Surface";
    }
}


////////////////////////////////////////////////////////////
void DRMContext::destroySurface()
{
    eglCheck(eglDestroySurface(m_display, m_surface));
    m_surface = EGL_NO_SURFACE;

    gbm_surface_destroy(m_gbmSurface);
    m_gbmSurface = nullptr;

    // Ensure that this context is no longer active since our surface is now destroyed
    setActive(false);
}


////////////////////////////////////////////////////////////
EGLConfig DRMContext::getBestConfig(EGLDisplay display, unsigned int bitsPerPixel, const ContextSettings& contextSettings)
{
    // Set our video settings constraint
    const EGLint attributes[] =
        {EGL_BUFFER_SIZE,
         static_cast<EGLint>(bitsPerPixel),
         EGL_DEPTH_SIZE,
         static_cast<EGLint>(contextSettings.depthBits),
         EGL_STENCIL_SIZE,
         static_cast<EGLint>(contextSettings.stencilBits),
         EGL_SAMPLE_BUFFERS,
         static_cast<EGLint>(contextSettings.antiAliasingLevel),
         EGL_BLUE_SIZE,
         8,
         EGL_GREEN_SIZE,
         8,
         EGL_RED_SIZE,
         8,
         EGL_ALPHA_SIZE,
         8,

         EGL_SURFACE_TYPE,
         EGL_WINDOW_BIT,
#if defined(SFML_OPENGL_ES)
         EGL_RENDERABLE_TYPE,
         EGL_OPENGL_ES_BIT,
#else
         EGL_RENDERABLE_TYPE,
         EGL_OPENGL_BIT,
#endif
         EGL_NONE};

    EGLint    configCount = 0;
    EGLConfig configs[1];

    // Ask EGL for the best config matching our video settings
    eglCheck(eglChooseConfig(display, attributes, configs, 1, &configCount));

    return configs[0];
}


////////////////////////////////////////////////////////////
void DRMContext::updateSettings()
{
    EGLint tmp = 0;

    // Update the internal context contextSettings with the current config
    eglCheck(eglGetConfigAttrib(m_display, m_config, EGL_DEPTH_SIZE, &tmp));
    m_settings.depthBits = static_cast<unsigned int>(tmp);

    eglCheck(eglGetConfigAttrib(m_display, m_config, EGL_STENCIL_SIZE, &tmp));
    m_settings.stencilBits = static_cast<unsigned int>(tmp);

    eglCheck(eglGetConfigAttrib(m_display, m_config, EGL_SAMPLES, &tmp));
    m_settings.antiAliasingLevel = static_cast<unsigned int>(tmp);

    m_settings.majorVersion   = 1;
    m_settings.minorVersion   = 1;
    m_settings.attributeFlags = ContextSettings::Attribute::Default;
}


////////////////////////////////////////////////////////////
GlFunctionPointer DRMContext::getFunction(const char* name)
{
    return reinterpret_cast<GlFunctionPointer>(eglGetProcAddress(name));
}


////////////////////////////////////////////////////////////
Drm& DRMContext::getDRM()
{
    initDrm();
    return drmNode;
}

} // namespace sf::priv
