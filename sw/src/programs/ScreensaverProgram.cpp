#include "ScreensaverProgram.h"

// Logging
#include "Logger.h"

// Noun selector for the idle timeout variable (VERB 8/9)
#include "DskyVariables.h"

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
        // Capture the connector id before freeing the connector.
        uint32_t connector = conn->connector_id;
        drmModeFreeConnector(conn);
        if (crtcId == 0) {
            continue;
        }

        drmModeCrtcPtr crtc = drmModeGetCrtc(fd, crtcId);
        if (crtc != nullptr) {
            found = crtc;
            connectorId = connector;
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

ScreensaverProgram::ScreensaverProgram(HmiData &data, VariableStore &store) :
    BgProgram(SCREENSAVER_PROGRAM, data, "SCREENSAVER", SCREENSAVER_TICK_PERIOD_MS),
    store_(store), screenOn_(true), restoreBrightness_(SCREENSAVER_BACKLIGHT_FALLBACK), drmFd_(-1), masterFd_(-1), crtcConfig_(nullptr), crtcConnectorId_(0), crtcBlanked_(false), capturedBufferId_(0), lastFailLog_(), lastInteraction_(std::chrono::steady_clock::now())
{
    backlightPath_ = findBacklightPath();
    if (backlightPath_.empty()) {
        LogError << "ScreensaverProgram: no backlight device found in " << SCREENSAVER_BACKLIGHT_DIR << std::endl;
    } else {
        LogInfo << "ScreensaverProgram: using backlight " << backlightPath_ << std::endl;
    }

    // Capture the active CRTC configuration so we can blank and restore it.
    drmFd_ = ::open(SCREENSAVER_DRM_CARD, O_RDWR | O_CLOEXEC);
    if (drmFd_ < 0) {
        LogWarning << "ScreensaverProgram: cannot open " << SCREENSAVER_DRM_CARD << " (" << std::strerror(errno) << "), falling back to backlight-only" << std::endl;
        drmFd_ = -1;
    } else {
        crtcConfig_ = findActiveCrtc(drmFd_, crtcConnectorId_);
        if (crtcConfig_ == nullptr) {
            LogWarning << "ScreensaverProgram: no active CRTC found on " << SCREENSAVER_DRM_CARD << ", falling back to backlight-only" << std::endl;
            ::close(drmFd_);
            drmFd_ = -1;
        } else {
            masterFd_ = findMasterFd(drmFd_);
            if (masterFd_ != drmFd_) {
                LogInfo << "ScreensaverProgram: DRM mastership held by fd " << masterFd_ << ", using it for CRTC control" << std::endl;
            }
            LogInfo << "ScreensaverProgram: using CRTC " << crtcConfig_->crtc_id << " (connector " << crtcConnectorId_ << ") for blanking" << std::endl;
        }
    }
}

ScreensaverProgram::~ScreensaverProgram()
{
    /* stop() joins the worker thread (no-op if not running) and wakes the
     * screen if it is currently blanked. */
    stop();

    // Best-effort restore so the user is never stranded in the dark.
    {
        std::lock_guard<std::mutex> lock(screenMutex_);
        if (crtcBlanked_) {
            // Best-effort restore with the buffer captured at blank time.
            // Ignore failures: this is best-effort cleanup on shutdown.
            drmModeSetCrtc((masterFd_ >= 0) ? masterFd_ : drmFd_, crtcConfig_->crtc_id,
                           capturedBufferId_ ? capturedBufferId_ : crtcConfig_->buffer_id,
                           crtcConfig_->x, crtcConfig_->y, &crtcConnectorId_, 1,
                           &crtcConfig_->mode);
            writeBacklight(backlightPath_, restoreBrightness_);
        }
    }
    if (drmFd_ >= 0) {
        ::close(drmFd_);
    }
    if (crtcConfig_ != nullptr) {
        drmModeFreeCrtc(crtcConfig_);
    }
}

void ScreensaverProgram::start()
{
    /* Read the idle timeout (seconds) from the persistent variable system and
     * apply it to this run. The read happens on the HMI thread; the worker
     * thread only ever sees the cached timerSeconds_ member, so no store
     * access is needed from the worker. */
    int seconds = SCREENSAVER_TIMER;
    if (!store_.get(DskyVariables::NOUN_SCREENSAVER_TIMER, seconds) || seconds < 1) {
        seconds = SCREENSAVER_TIMER;
    }
    timerSeconds_.store(seconds);

    /* Restart the idle clock so a freshly started screensaver cannot blank
     * the screen immediately. */
    {
        std::lock_guard<std::mutex> lock(screenMutex_);
        lastInteraction_ = std::chrono::steady_clock::now();
    }

    LogInfo << "ScreensaverProgram: starting, idle timer " << seconds << "s" << std::endl;
    BgProgram::start();
}

void ScreensaverProgram::stop()
{
    BgProgram::stop();

    /* Never strand the user in the dark: if the screen is currently blanked,
     * wake it now that the screensaver no longer manages the display. This is
     * also the retry path for a restore that failed on a previous stop(). */
    setScreen(true);

    LogInfo << "ScreensaverProgram: stopped" << std::endl;
}

void ScreensaverProgram::operation()
{
    /* The expiry check and the blanking must be atomic with respect to
     * notifyActivity(): holding screenMutex_ across both guarantees that an
     * activity notification landing in between refreshes lastInteraction_
     * first, so a stale decision can never blank a screen that was just woken. */
    bool transitioned = false;
    std::function<void(bool)> callback;
    {
        std::lock_guard<std::mutex> lock(screenMutex_);

        auto idle = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - lastInteraction_).count();
        if (idle >= timerSeconds_.load()) {
            transitioned = setScreenLocked(false);
        }

        callback = stateCallback_;
    }

    // Fire the callback outside the lock so an IPC call in it cannot stall the
    // tick thread, and from a copy taken under the lock so a concurrent
    // setStateCallback() cannot race with the invocation.
    if (transitioned && callback) {
        callback(false);
    }
}

void ScreensaverProgram::notifyActivity()
{
    /* Wake and refresh the idle timestamp under one lock so the tick thread
     * cannot blank in between (see operation()). Waking happens regardless of
     * run state: this is also the retry path for a restore that failed at
     * stop() time. May be called from any thread (main on keypad input, or a
     * background program worker on its own events). */
    bool transitioned = false;
    std::function<void(bool)> callback;
    {
        std::lock_guard<std::mutex> lock(screenMutex_);

        if (!screenOn_) {
            transitioned = setScreenLocked(true);
        }

        if (isRunning()) {
            lastInteraction_ = std::chrono::steady_clock::now();
        }

        callback = stateCallback_;
    }

    // Fire the callback outside the lock (see operation()).
    if (transitioned && callback) {
        callback(true);
    }
}

void ScreensaverProgram::setTimer(int seconds)
{
    // Clamp to a sane minimum so a zero/negative value cannot blank the
    // screen immediately.
    int clamped = (seconds < 1) ? 1 : seconds;
    timerSeconds_.store(clamped);
    LogInfo << "ScreensaverProgram: idle timer set to " << clamped << "s" << std::endl;
}

bool ScreensaverProgram::isScreenOn() const
{
    std::lock_guard<std::mutex> lock(screenMutex_);
    return screenOn_;
}

void ScreensaverProgram::setStateCallback(std::function<void(bool)> cb)
{
    std::lock_guard<std::mutex> lock(screenMutex_);
    stateCallback_ = std::move(cb);
}

void ScreensaverProgram::setScreen(bool on)
{
    bool transitioned = false;
    std::function<void(bool)> callback;
    {
        std::lock_guard<std::mutex> lock(screenMutex_);
        transitioned = setScreenLocked(on);
        callback = stateCallback_;
    }

    // Fire the callback outside the lock (see operation()).
    if (transitioned && callback) {
        callback(on);
    }
}

bool ScreensaverProgram::setScreenLocked(bool on)
{
    // Caller holds screenMutex_.

    // No change needed.
    if (screenOn_ == on) {
        return false;
    }

    if (backlightPath_.empty()) {
        logFailure("no backlight device found");
        return false;
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
            // Capture the buffer currently scanning out so we can restore it on
            // wake. SDL keeps its framebuffer alive while blanked, so the id
            // stays valid until the next successful present after restore.
            drmModeCrtcPtr live = drmModeGetCrtc(masterFd_, crtcConfig_->crtc_id);
            if (live != nullptr) {
                capturedBufferId_ = live->buffer_id;
                drmModeFreeCrtc(live);
            }

            if (drmModeSetCrtc(masterFd_, crtcConfig_->crtc_id, 0, 0, 0, nullptr, 0, nullptr) != 0) {
                logFailure("failed to blank CRTC " + std::to_string(crtcConfig_->crtc_id) + " (" + std::strerror(errno) + ")");
            } else {
                crtcBlanked_ = true;
            }
        }

        if (!writeBacklight(backlightPath_, 0)) {
            logFailure("failed to turn screen off (" + backlightPath_ + ": " + std::strerror(errno) + ")");
            // The CRTC is already disabled: restore it so the panel does not
            // stay dark, and leave the screen on so blanking is retried next tick.
            if (crtcBlanked_) {
                drmModeSetCrtc(masterFd_, crtcConfig_->crtc_id,
                               capturedBufferId_ ? capturedBufferId_ : crtcConfig_->buffer_id,
                               crtcConfig_->x, crtcConfig_->y, &crtcConnectorId_, 1,
                               &crtcConfig_->mode);
                crtcBlanked_ = false;
            }
            return false; // do not update the cached state, retry next tick
        }

        screenOn_ = false;
        LogInfo << "ScreensaverProgram: screen off (will restore brightness " << restoreBrightness_ << ")" << std::endl;
    } else {
        // Bring the backlight up first so the screen is never left dark even
        // if the CRTC restore below cannot be completed this pass.
        if (!writeBacklight(backlightPath_, restoreBrightness_)) {
            logFailure("failed to turn screen on (" + backlightPath_ + ": " + std::strerror(errno) + ")");
            return false; // do not update the cached state, retry next tick
        }

        // Restore the CRTC with the buffer we captured at blank time. SDL
        // keeps its framebuffer alive while blanked, so that id is still valid.
        if (crtcBlanked_) {
            if (drmModeSetCrtc(masterFd_, crtcConfig_->crtc_id,
                               capturedBufferId_ ? capturedBufferId_ : crtcConfig_->buffer_id,
                               crtcConfig_->x, crtcConfig_->y, &crtcConnectorId_, 1,
                               &crtcConfig_->mode) != 0) {
                logFailure("failed to restore CRTC " + std::to_string(crtcConfig_->crtc_id) + " (" + std::strerror(errno) + ")");
                return false; // keep the screen off and blanked, retried on the next keypress
            }
            crtcBlanked_ = false;
        }

        screenOn_ = true;
        LogInfo << "ScreensaverProgram: screen on (brightness " << restoreBrightness_ << ")" << std::endl;
    }

    return true;
}

bool ScreensaverProgram::ensureCrtcConfig()
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
        LogInfo << "ScreensaverProgram: DRM mastership held by fd " << masterFd_ << ", using it for CRTC control" << std::endl;
    }

    LogInfo << "ScreensaverProgram: discovered CRTC " << crtcConfig_->crtc_id << " (connector " << crtcConnectorId_ << ") for blanking" << std::endl;
    return true;
}

void ScreensaverProgram::logFailure(const std::string &what)
{
    // Caller holds screenMutex_.
    const auto now = std::chrono::steady_clock::now();
    if (now - lastFailLog_ < std::chrono::seconds(SCREENSAVER_LOG_INTERVAL)) {
        return;
    }

    LogError << "ScreensaverProgram: " << what << std::endl;
    lastFailLog_ = now;
}
