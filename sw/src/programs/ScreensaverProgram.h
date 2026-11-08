#ifndef _SCREENSAVERPROGRAM_H_
#define _SCREENSAVERPROGRAM_H_

// STL
#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>

// Core
#include "BgProgram.h"
#include "VariableStore.h"

/**
 * @brief Program ID of the screensaver background program (selected on the keypad).
 */
#define SCREENSAVER_PROGRAM 5

/**
 * @brief Default auto-start state at boot (1 = start, 0 = do not start).
 *
 * This is only the compile-time default. The effective value is stored in the
 * persistent variable system (NOUN DskyVariables::NOUN_SCREENSAVER_AUTOSTART)
 * and read once by main at boot.
 */
#define SCREENSAVER_AUTOSTART_DEFAULT 1

/**
 * @brief Worker loop tick period [milliseconds].
 *
 * The screensaver checks the idle state about once per second; the actual
 * idle timeout is defined separately by SCREENSAVER_TIMER / NOUN 4.
 */
#define SCREENSAVER_TICK_PERIOD_MS 1000

/**
 * @brief Default idle time before the screen is turned off [seconds].
 *
 * This is only the compile-time default. The effective value is stored in the
 * ScreensaverProgram timerSeconds_ member and can be changed at runtime through
 * the persistent variable system (VERB 8/9, NOUN DskyVariables::NOUN_SCREENSAVER_TIMER).
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
 * @brief Background program that turns the display off after a period of keypad inactivity.
 *
 * While running, the worker thread ticks about once per second and turns the
 * screen off by stopping CRTC scanout through libdrm (a true blank) and
 * dropping the panel backlight to 0 via sysfs; waking restores the saved CRTC
 * configuration and the brightness level captured before sleep. Any keypad
 * interaction resets the idle timer and immediately wakes the screen. If no
 * DRM device or active CRTC is available, only the backlight is controlled.
 *
 * The program can be started at boot (NOUN DskyVariables::NOUN_SCREENSAVER_AUTOSTART)
 * or manually from the keypad (PRO + SCREENSAVER_PROGRAM). Stopping it wakes
 * the screen immediately if it is currently blanked, so the user is never
 * stranded in the dark.
 */
class ScreensaverProgram : public BgProgram {

    private:

    /**
     * @brief Reference to the persistent variable store, used by start() to
     *        read the current idle timeout (NOUN 4).
     */
    VariableStore &store_;

    /**
     * @brief Idle time before the screen is turned off [seconds]. Defaults to
     *        SCREENSAVER_TIMER; updated at runtime through the persistent
     *        variable system via setTimer() and re-read from the store on
     *        every start(). Atomic because it is written by the main thread
     *        and read by the tick thread.
     */
    std::atomic<int> timerSeconds_{SCREENSAVER_TIMER};

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
     * @brief Framebuffer id that was scanning out on the CRTC, captured via
     *        drmModeGetCrtc() immediately before blanking. Restoring with
     *        this id is valid because SDL keeps the framebuffer alive while
     *        the CRTC is disabled (rendering is gated by the caller).
     *        Guarded by screenMutex_.
     */
    uint32_t capturedBufferId_;

    /**
     * @brief Timestamp of the last failure log message, used to throttle
     *        repeated errors. Guarded by screenMutex_.
     */
    std::chrono::steady_clock::time_point lastFailLog_;

    /**
     * @brief Timestamp of the last keypad interaction, guarded by screenMutex_.
     *
     * Sharing screenMutex_ with the screen state makes "check idle expiry and
     * blank" (operation) and "wake and refresh the timestamp" (notifyActivity)
     * atomic with respect to each other, so a keypress can never be clobbered
     * by a stale blank decision from the tick thread.
     */
    std::chrono::steady_clock::time_point lastInteraction_;

    /**
     * @brief Guards screenOn_, backlightPath_, restoreBrightness_, drmFd_,
     *        masterFd_, crtcConfig_, crtcConnectorId_, crtcBlanked_,
     *        capturedBufferId_, lastFailLog_ and lastInteraction_ so the main
     *        thread and the tick thread never issue redundant or racing sysfs
     *        or DRM operations. Mutable because the const getter isScreenOn()
     *        also reads the guarded state.
     */
    mutable std::mutex screenMutex_;

    /**
     * @brief Optional callback invoked on every real screen power transition.
     *
     * Called with true when the screen turns on and false when it turns off,
     * only after a transition actually succeeded (a failed blank or wake is
     * not reported). Invoked from whatever thread performed the transition
     * (tick thread on idle blanking, main thread on keypresses, or a
     * background program worker on notifyActivity()) and always with
     * screenMutex_ released. Guarded by screenMutex_; copied under the lock
     * before invocation so it can be replaced safely at runtime.
     */
    std::function<void(bool)> stateCallback_;

    public:

    /**
     * @brief Constructor. Discovers the backlight and DRM/CRTC hardware but
     *        does not start the worker thread; call start() to run.
     */
    ScreensaverProgram(HmiData &data, VariableStore &store);

    /**
     * @brief Destructor. Stops the program (waking the screen if blanked)
     *        and releases the DRM resources.
     */
    virtual ~ScreensaverProgram();

    /**
     * @brief Start the screensaver: read the idle timeout from the variable
     *        store, reset the idle clock, and spawn the worker thread.
     */
    void start() override;

    /**
     * @brief Stop the screensaver and join the worker thread. If the screen
     *        is currently blanked it is woken immediately so the user is
     *        never stranded in the dark.
     */
    void stop() override;

    /**
     * @brief Worker tick: turns the screen off when the idle time has
     *        exceeded the configured timeout.
     */
    void operation() override;

    /**
     * @brief Notify that activity occurred (keypad input or a program event).
     *
     * Wakes the screen if it is currently off and, while running, restarts
     * the idle countdown. Called by main on keypad events and by background
     * programs on events they consider screen-worthy (e.g. a door state
     * change). Note: a periodic source firing more often than the idle
     * timeout will keep the screen on — wire this in deliberately.
     */
    void notifyActivity();

    /**
     * @brief Set the idle time before the screen is turned off [seconds].
     *
     * Called by the persistent variable system when NOUN 4 is written. Values
     * below 1 are clamped to 1 so a zero or negative timeout cannot blank the
     * screen immediately.
     *
     * @param[in] seconds idle time in seconds, minimum 1
     */
    void setTimer(int seconds);

    /**
     * @brief Register a callback invoked on every real screen power transition.
     *
     * The callback receives true when the screen turns on and false when it
     * turns off. It is only called for transitions that actually succeeded,
     * from whatever thread performed them, and always with screenMutex_
     * released — so it may safely perform IPC or block briefly without
     * stalling blanking or waking. Pass an empty callback to detach.
     *
     * @param[in] cb callback invoked as cb(screenOn), may be empty
     */
    void setStateCallback(std::function<void(bool)> cb);

    /**
     * @brief Query whether the physical screen is currently on.
     *
     * The main loop uses this to skip rendering while the display is blanked:
     * presenting with the CRTC disabled leaks Mesa sync_file fence fds, so no
     * GPU work should be submitted until the screen wakes again.
     *
     * @return true if the screen is on, false if it is blanked
     */
    bool isScreenOn() const;

    private:

    /**
     * @brief Turn the physical display on or off.
     *
     * Turning off captures the current brightness (or falls back to a safe
     * level), captures the buffer id currently scanning out, disables the CRTC
     * for a true blank when available, and writes 0 to the backlight. Turning
     * on restores the captured brightness first, then re-enables the CRTC with
     * the buffer captured at blank time (SDL keeps its framebuffer alive while
     * blanked, so that id stays valid). The cached state is only updated on
     * success so a failed operation is retried on the next tick or interaction.
     *
     * @param[in] on true to turn the screen on, false to turn it off
     */
    void setScreen(bool on);

    /**
     * @brief Unlocked implementation of setScreen(); the caller must hold
     *        screenMutex_. Splitting it out lets operation() and
     *        notifyActivity() perform their state check and the resulting
     *        transition atomically under a single lock.
     *
     * @param[in] on true to turn the screen on, false to turn it off
     * @return true if the screen state actually changed (so callers can fire
     *         the state callback), false if nothing happened or the operation
     *         failed and will be retried later
     */
    bool setScreenLocked(bool on);

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

#endif /* _SCREENSAVERPROGRAM_H_ */
