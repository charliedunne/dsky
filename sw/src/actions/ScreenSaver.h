#ifndef _SCREENSAVER_H_
#define _SCREENSAVER_H_

// STL
#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>

/**
 * @brief Default screensaver state at startup (1 = enabled, 0 = disabled).
 */
#define SCREENSAVER_DEFAULT 1

/**
 * @brief Idle time before the screen is turned off [seconds].
 */
#define SCREENSAVER_TIMER 30

/**
 * @brief Directory holding the kernel backlight devices.
 */
#define SCREENSAVER_BACKLIGHT_DIR "/sys/class/backlight"

/**
 * @brief Brightness level used to wake the display when the previous level
 *        was 0 or could not be read, so the user is never stranded in the dark.
 */
#define SCREENSAVER_BACKLIGHT_FALLBACK 128

/**
 * @brief Device node of the DRM card driving the display panel.
 */
#define SCREENSAVER_DRM_CARD "/dev/dri/card0"

/**
 * @brief Minimum interval between repeated backlight failure log messages
 *        [seconds], to keep the journal clean when the device is missing.
 */
#define SCREENSAVER_LOG_INTERVAL 30

/**
 * @brief Forward declaration of libdrm's CRTC configuration structure; the
 *        DRM headers are only needed in the implementation file.
 */
struct _drmModeCrtc;

/**
 * @brief Turns the display off after a period of keypad inactivity.
 *
 * A background thread ticks about once per second and turns the screen off
 * by stopping CRTC scanout through libdrm (a true blank) and dropping the
 * panel backlight to 0 via sysfs; waking restores the saved CRTC
 * configuration and the brightness level captured before sleep. Any keypad
 * interaction resets the idle timer and immediately wakes the screen. If no
 * DRM device or active CRTC is available, only the backlight is controlled.
 */
class ScreenSaver {

    private:

    /**
     * @brief Background thread object
     */
    std::thread thread_;

    /**
     * @brief Flag to determine if the thread must continue its execution
     */
    std::atomic<bool> running_;

    /**
     * @brief Screensaver enable state (toggled by VERB 3 actions)
     */
    std::atomic<bool> enabled_;

    /**
     * @brief Cached screen power state, guarded by screenMutex_
     */
    bool screenOn_;

    /**
     * @brief Resolved path of the backlight brightness sysfs file, empty if
     *        no backlight device was found at startup. Guarded by screenMutex_.
     */
    std::string backlightPath_;

    /**
     * @brief Brightness level to restore when waking the screen: the level
     *        captured just before it was turned off, or the fallback level if
     *        that could not be read. Guarded by screenMutex_.
     */
    int restoreBrightness_;

    /**
     * @brief Open file descriptor of the DRM device, -1 if unavailable.
     *        Guarded by screenMutex_.
     */
    int drmFd_;

    /**
     * @brief File descriptor that holds DRM mastership on the card, used for
     *        CRTC blanking and restore because those ioctls require
     *        mastership; equals drmFd_ when our own fd is (or becomes) the
     *        master, -1 if unknown. Guarded by screenMutex_.
     */
    int masterFd_;

    /**
     * @brief Saved CRTC configuration captured at startup (or lazily on the
     *        first blank if the display was not configured yet), used to
     *        restore scanout when waking the display; nullptr in
     *        backlight-only mode. Guarded by screenMutex_.
     */
    struct _drmModeCrtc *crtcConfig_;

    /**
     * @brief Connector id attached to the saved CRTC, needed because this
     *        libdrm version does not store connectors in drmModeCrtc.
     *        Guarded by screenMutex_.
     */
    uint32_t crtcConnectorId_;

    /**
     * @brief True while the CRTC is blanked (scanout stopped). Guarded by
     *        screenMutex_.
     */
    bool crtcBlanked_;

    /**
     * @brief Timestamp of the last failure log message, used to throttle
     *        repeated errors. Guarded by screenMutex_.
     */
    std::chrono::steady_clock::time_point lastFailLog_;

    /**
     * @brief Guards screenOn_, backlightPath_, restoreBrightness_, drmFd_,
     *        masterFd_, crtcConfig_, crtcConnectorId_, crtcBlanked_ and
     *        lastFailLog_ so
     *        the main thread and the tick thread never issue redundant or
     *        racing sysfs or DRM operations
     */
    std::mutex screenMutex_;

    /**
     * @brief Timestamp of the last keypad interaction, guarded by idleMutex_
     */
    std::chrono::steady_clock::time_point lastInteraction_;

    /**
     * @brief Guards lastInteraction_ (no atomic time_point in C++17)
     */
    std::mutex idleMutex_;

    public:

    /**
     * @brief Constructor. Starts the background monitoring thread.
     */
    ScreenSaver();

    /**
     * @brief Destructor. Stops and joins the background thread.
     */
    ~ScreenSaver();

    /**
     * @brief Notify that the user touched the keypad: resets the idle timer
     *        and wakes the screen if it is currently off.
     */
    void noteInteraction();

    /**
     * @brief Enable or disable the screensaver. Disabling while the screen is
     *        off wakes the screen immediately so the user is never stranded
     *        in the dark.
     *
     * @param[in] enabled true to enable, false to disable
     */
    void setEnabled(bool enabled);

    /**
     * @brief Query whether the screensaver is currently enabled.
     *
     * @return true if enabled, false otherwise
     */
    bool isEnabled() const;

    private:

    /**
     * @brief Background thread body: ticks about once per second and turns
     *        the screen off after SCREENSAVER_TIMER seconds of inactivity.
     */
    void run();

    /**
     * @brief Turn the physical display on or off.
     *
     * Turning off captures the current brightness (or falls back to a safe
     * level), stops CRTC scanout when available for a true blank, and writes
     * 0 to the backlight; turning on re-enables the saved CRTC configuration
     * and restores the captured brightness. The cached state is only updated
     * on success so a failed operation is retried on the next tick or
     * interaction.
     *
     * @param[in] on true to turn the screen on, false to turn it off
     */
    void setScreen(bool on);

    /**
     * @brief Lazily discover the active CRTC configuration on first use.
     *
     * The display may not be configured yet when the constructor runs, so the
     * initial discovery can fail and fall back to backlight-only mode. This
     * reopens the DRM device if it was closed and retries the discovery; it is
     * a no-op once a configuration has been captured. CRTC changes require DRM
     * mastership, which on this system may be held by another fd of this
     * process (e.g. SDL's); the holder is located by probing and cached in
     * masterFd_.
     *
     * @return true if a CRTC configuration is available for blanking
     */
    bool ensureCrtcConfig();

    /**
     * @brief Log a screensaver failure, throttled to one message per
     *        SCREENSAVER_LOG_INTERVAL seconds.
     *
     * @param[in] what full description of the failure
     */
    void logFailure(const std::string &what);

};

#endif /* _SCREENSAVER_H_ */
