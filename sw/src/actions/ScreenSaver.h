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
 * @brief Minimum interval between repeated backlight failure log messages
 *        [seconds], to keep the journal clean when the device is missing.
 */
#define SCREENSAVER_LOG_INTERVAL 30

/**
 * @brief Turns the display off after a period of keypad inactivity.
 *
 * A background thread ticks about once per second and controls the panel
 * backlight through sysfs: turning the screen off writes 0 to the brightness
 * file, waking it restores the level captured before sleep. Any keypad
 * interaction resets the idle timer and immediately wakes the screen.
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
     * @brief Timestamp of the last backlight failure log message, used to
     *        throttle repeated errors. Guarded by screenMutex_.
     */
    std::chrono::steady_clock::time_point lastFailLog_;

    /**
     * @brief Guards screenOn_, backlightPath_, restoreBrightness_ and
     *        lastFailLog_ so the main thread and the tick thread never issue
     *        redundant or racing sysfs writes
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
     * @brief Turn the physical display on or off via the backlight sysfs.
     *
     * Turning off captures the current brightness (or falls back to a safe
     * level) and writes 0; turning on restores the captured level. The cached
     * state is only updated on success so a failed write is retried on the
     * next tick.
     *
     * @param[in] on true to turn the screen on, false to turn it off
     */
    void setScreen(bool on);

    /**
     * @brief Log a backlight failure, throttled to one message per
     *        SCREENSAVER_LOG_INTERVAL seconds.
     *
     * @param[in] what short description of the failure
     */
    void logBacklightFailure(const char *what);

};

#endif /* _SCREENSAVER_H_ */
