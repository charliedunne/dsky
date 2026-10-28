#include "ScreenSaver.h"

// Logging
#include "Logger.h"

// DRM (libdrm)
#include <xf86drm.h>
#include <xf86drmMode.h>

// System
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <unistd.h>

namespace {

/**
 * @brief Read a numeric brightness level from a backlight sysfs file.
 *
 * @param[in]  path full path to the brightness file
 * @param[out] value parsed brightness level
 * @return true on success, false if the file could not be read or parsed
 */
bool readBacklight(const std::string &path, int &value)
{
    std::ifstream in(path);
    if (!in) {
        return false;
    }
    if (!(in >> value)) {
        return false;
    }
    return true;
}

/**
 * @brief Write a brightness level to a backlight sysfs file.
 *
 * @param[in] path  full path to the brightness file
 * @param[in] value brightness level to write
 * @return true on success, false if the write failed
 */
bool writeBacklight(const std::string &path, int value)
{
    std::ofstream out(path);
    if (!out) {
        return false;
    }
    out << value << std::endl;
    return static_cast<bool>(out);
}

/**
 * @brief Find the first backlight brightness sysfs file.
 *
 * The target hardware has exactly one backlight device, so the first match
 * wins.
 *
 * @return full path to the brightness file, or an empty string if none found
 */
std::string findBacklightPath()
{
    namespace fs = std::filesystem;

    const fs::path dir(SCREENSAVER_BACKLIGHT_DIR);
    std::error_code ec;
    if (!fs::is_directory(dir, ec)) {
        return "";
    }

    for (const auto &entry : fs::directory_iterator(dir, ec)) {
        if (!entry.is_directory(ec)) {
            continue;
        }
        const fs::path brightness = entry.path() / "brightness";
        if (fs::is_regular_file(brightness, ec)) {
            return brightness.string();
        }
    }

    return "";
}

/**
 * @brief Find the first CRTC that currently has an active connector attached.
 *
 * Walks all connectors on the given DRM device, finds one bound to an encoder
 * whose CRTC matches a real CRTC, and returns that CRTC's configuration. This
 * libdrm version does not store the attached connectors inside drmModeCrtc, so
 * the active connector id is reported separately for later restore. The caller
 * owns the returned structure and must release it with drmModeFreeCrtc().
 *
 * @param[in]  fd          open DRM device file descriptor
 * @param[out] connectorId connector id attached to the found CRTC (0 if none)
 * @return CRTC configuration for the active display, or nullptr if none found
 */
struct _drmModeCrtc *findActiveCrtc(int fd, uint32_t &connectorId)
{
    drmModeResPtr resources = drmModeGetResources(fd);
    if (resources == nullptr) {
        return nullptr;
    }

    struct _drmModeCrtc *found = nullptr;
    for (int i = 0; i < resources->count_connectors && found == nullptr; ++i) {
        drmModeConnectorPtr conn = drmModeGetConnector(fd, resources->connectors[i]);
        if (conn == nullptr || conn->encoder_id == 0) {
            if (conn != nullptr) {
                drmModeFreeConnector(conn);
            }
            continue;
        }

        // Confirm the encoder is actually bound to one of this device's CRTCs.
        uint32_t crtcId = 0;
        drmModeEncoderPtr enc = drmModeGetEncoder(fd, conn->encoder_id);
        if (enc != nullptr) {
            crtcId = enc->crtc_id;
            drmModeFreeEncoder(enc);
        }
        drmModeFreeConnector(conn);
        if (crtcId == 0) {
            continue;
        }

        drmModeCrtcPtr crtc = drmModeGetCrtc(fd, crtcId);
        if (crtc != nullptr) {
            found = crtc;
            connectorId = conn->connector_id;
        }
    }

    drmModeFreeResources(resources);
    return found;
}

/**
 * @brief Find the file descriptor that holds DRM mastership for card0.
 *
 * SetCrtc requires the calling fd to be the DRM master, but another component
 * in this process (e.g. SDL) may have become the master by opening the device
 * first. Probing with drmSetMaster() is side-effect-free when a master already
 * exists: exactly one fd returns 0 and all others get -EBUSY/-EACCES, so no
 * live display configuration can be disturbed. When no master exists yet, the
 * first probed fd acquires it, which is safe because this class owns that fd.
 *
 * @param[in] fallbackFd fd opened by this class; probed first and returned if
 *                       no other holder is found
 * @return fd holding (or newly acquiring) mastership, or fallbackFd as last resort
 */
int findMasterFd(int fallbackFd)
{
    namespace fs = std::filesystem;

    // Probe our own fd first: if nobody holds mastership we acquire it on an
    // fd this class owns and closes.
    if (drmSetMaster(fallbackFd) == 0) {
        return fallbackFd;
    }

    // A master exists elsewhere: scan /proc/self/fd for other card0 fds.
    std::error_code ec;
    const fs::path fdDir("/proc/self/fd");
    if (fs::is_directory(fdDir, ec)) {
        for (const auto &entry : fs::directory_iterator(fdDir, ec)) {
            // Only consider symlinks pointing at our DRM card.
            const fs::path target = fs::read_symlink(entry.path(), ec);
            if (ec || target != SCREENSAVER_DRM_CARD) {
                continue;
            }

            int fd = 0;
            try {
                fd = std::stoi(entry.path().filename().string());
            } catch (...) {
                continue;
            }
            if (fd == fallbackFd) {
                continue;
            }

            // Returns 0 only on the fd that is already the master.
            if (drmSetMaster(fd) == 0) {
                return fd;
            }
        }
    }

    return fallbackFd;
}

} // namespace

ScreenSaver::ScreenSaver() : running_(true), enabled_(SCREENSAVER_DEFAULT != 0), screenOn_(true), restoreBrightness_(SCREENSAVER_BACKLIGHT_FALLBACK), drmFd_(-1), masterFd_(-1), crtcConfig_(nullptr), crtcConnectorId_(0), crtcBlanked_(false), lastFailLog_(), lastInteraction_(std::chrono::steady_clock::now())
{
    backlightPath_ = findBacklightPath();
    if (backlightPath_.empty()) {
        LogError << "ScreenSaver: no backlight device found in " << SCREENSAVER_BACKLIGHT_DIR << std::endl;
    } else {
        LogInfo << "ScreenSaver: using backlight " << backlightPath_ << std::endl;
    }

    // Capture the active CRTC configuration so we can blank and restore it.
    drmFd_ = ::open(SCREENSAVER_DRM_CARD, O_RDWR | O_CLOEXEC);
    if (drmFd_ < 0) {
        LogWarning << "ScreenSaver: cannot open " << SCREENSAVER_DRM_CARD << " (" << std::strerror(errno) << "), falling back to backlight-only" << std::endl;
        drmFd_ = -1;
    } else {
        crtcConfig_ = findActiveCrtc(drmFd_, crtcConnectorId_);
        if (crtcConfig_ == nullptr) {
            LogWarning << "ScreenSaver: no active CRTC found on " << SCREENSAVER_DRM_CARD << ", falling back to backlight-only" << std::endl;
            ::close(drmFd_);
            drmFd_ = -1;
        } else {
            masterFd_ = findMasterFd(drmFd_);
            if (masterFd_ != drmFd_) {
                LogInfo << "ScreenSaver: DRM mastership held by fd " << masterFd_ << ", using it for CRTC control" << std::endl;
            }
            LogInfo << "ScreenSaver: using CRTC " << crtcConfig_->crtc_id << " (connector " << crtcConnectorId_ << ") for blanking" << std::endl;
        }
    }

    thread_ = std::thread(&ScreenSaver::run, this);
}

ScreenSaver::~ScreenSaver()
{
    running_ = false;
    if (thread_.joinable()) {
        thread_.join();
    }

    // Best-effort restore so the user is never stranded in the dark.
    if (crtcBlanked_) {
        drmModeSetCrtc((masterFd_ >= 0) ? masterFd_ : drmFd_, crtcConfig_->crtc_id,
                       crtcConfig_->buffer_id, crtcConfig_->x, crtcConfig_->y,
                       &crtcConnectorId_, 1,
                       &crtcConfig_->mode);
    }
    if (drmFd_ >= 0) {
        ::close(drmFd_);
    }
    if (crtcConfig_ != nullptr) {
        drmModeFreeCrtc(crtcConfig_);
    }
}

void ScreenSaver::noteInteraction()
{
    // Refresh the idle timestamp.
    {
        std::lock_guard<std::mutex> lock(idleMutex_);
        lastInteraction_ = std::chrono::steady_clock::now();
    }

    // Wake the screen if it is currently off.
    setScreen(true);
}

void ScreenSaver::setEnabled(bool enabled)
{
    bool wasEnabled = enabled_.exchange(enabled != 0);
    if (wasEnabled == (enabled != 0)) {
        return;
    }

    LogInfo << "ScreenSaver: " << (enabled ? "enabled" : "disabled") << std::endl;

    // Never strand the user in the dark when disabling.
    if (!enabled) {
        setScreen(true);
    }
}

bool ScreenSaver::isEnabled() const
{
    return enabled_;
}

void ScreenSaver::run()
{
    while (running_) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (!running_ || !enabled_) {
            continue;
        }

        // Snapshot the last interaction under the mutex.
        std::chrono::steady_clock::time_point last;
        {
            std::lock_guard<std::mutex> lock(idleMutex_);
            last = lastInteraction_;
        }

        auto idle = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - last).count();
        if (idle >= SCREENSAVER_TIMER) {
            setScreen(false);
        }
    }
}

void ScreenSaver::setScreen(bool on)
{
    std::lock_guard<std::mutex> lock(screenMutex_);

    // No change needed.
    if (screenOn_ == on) {
        return;
    }

    if (backlightPath_.empty()) {
        logFailure("no backlight device found");
        return;
    }

    if (!on) {
        // Capture the current brightness so we can restore it on wake.
        int current = 0;
        if (!readBacklight(backlightPath_, current) || current <= 0) {
            // Already off or unreadable: use a safe fallback level so the
            // user is never stranded in the dark.
            current = SCREENSAVER_BACKLIGHT_FALLBACK;
        }
        restoreBrightness_ = current;

        // Blank the display by disabling the CRTC (true screen-off, like X DPMS).
        if (!crtcBlanked_ && ensureCrtcConfig()) {
            if (drmModeSetCrtc(masterFd_, crtcConfig_->crtc_id, 0, 0, 0, nullptr, 0, nullptr) != 0) {
                logFailure("failed to blank CRTC " + std::to_string(crtcConfig_->crtc_id) + " (" + std::strerror(errno) + ")");
            } else {
                crtcBlanked_ = true;
            }
        }

        if (!writeBacklight(backlightPath_, 0)) {
            logFailure("failed to turn screen off (" + backlightPath_ + ": " + std::strerror(errno) + ")");
            return; // do not update the cached state, retry next tick
        }

        screenOn_ = false;
        LogInfo << "ScreenSaver: screen off (will restore brightness " << restoreBrightness_ << ")" << std::endl;
    } else {
        // Restore the CRTC with its saved configuration.
        if (crtcBlanked_) {
            if (drmModeSetCrtc(masterFd_, crtcConfig_->crtc_id, crtcConfig_->buffer_id,
                               crtcConfig_->x, crtcConfig_->y,
                               &crtcConnectorId_, 1,
                               &crtcConfig_->mode) != 0) {
                logFailure("failed to restore CRTC " + std::to_string(crtcConfig_->crtc_id) + " (" + std::strerror(errno) + ")");
                return; // keep the screen off, retried on the next keypress
            }
            crtcBlanked_ = false;
        }

        if (!writeBacklight(backlightPath_, restoreBrightness_)) {
            logFailure("failed to turn screen on (" + backlightPath_ + ": " + std::strerror(errno) + ")");
            return; // do not update the cached state, retry next tick
        }

        screenOn_ = true;
        LogInfo << "ScreenSaver: screen on (brightness " << restoreBrightness_ << ")" << std::endl;
    }
}

bool ScreenSaver::ensureCrtcConfig()
{
    // Caller holds screenMutex_.
    if (crtcConfig_ != nullptr) {
        return true;
    }

    // The constructor may have closed the DRM device after a failed initial
    // discovery; reopen it so we can retry now that the display is configured.
    if (drmFd_ < 0) {
        drmFd_ = ::open(SCREENSAVER_DRM_CARD, O_RDWR | O_CLOEXEC);
        if (drmFd_ < 0) {
            logFailure("cannot open " + std::string(SCREENSAVER_DRM_CARD) + " (" + std::strerror(errno) + ")");
            return false;
        }
    }

    crtcConfig_ = findActiveCrtc(drmFd_, crtcConnectorId_);
    if (crtcConfig_ == nullptr) {
        logFailure("no active CRTC found on " + std::string(SCREENSAVER_DRM_CARD));
        return false;
    }

    masterFd_ = findMasterFd(drmFd_);
    if (masterFd_ != drmFd_) {
        LogInfo << "ScreenSaver: DRM mastership held by fd " << masterFd_ << ", using it for CRTC control" << std::endl;
    }

    LogInfo << "ScreenSaver: discovered CRTC " << crtcConfig_->crtc_id << " (connector " << crtcConnectorId_ << ") for blanking" << std::endl;
    return true;
}

void ScreenSaver::logFailure(const std::string &what)
{
    // Caller holds screenMutex_.
    const auto now = std::chrono::steady_clock::now();
    if (now - lastFailLog_ < std::chrono::seconds(SCREENSAVER_LOG_INTERVAL)) {
        return;
    }

    LogError << "ScreenSaver: " << what << std::endl;
    lastFailLog_ = now;
}
