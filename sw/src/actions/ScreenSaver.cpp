#include "ScreenSaver.h"

// Logging
#include "Logger.h"

// System
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

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

} // namespace

ScreenSaver::ScreenSaver() : running_(true), enabled_(SCREENSAVER_DEFAULT != 0), screenOn_(true), restoreBrightness_(SCREENSAVER_BACKLIGHT_FALLBACK), lastFailLog_(), lastInteraction_(std::chrono::steady_clock::now())
{
    backlightPath_ = findBacklightPath();
    if (backlightPath_.empty()) {
        LogError << "ScreenSaver: no backlight device found in " << SCREENSAVER_BACKLIGHT_DIR << std::endl;
    } else {
        LogInfo << "ScreenSaver: using backlight " << backlightPath_ << std::endl;
    }

    thread_ = std::thread(&ScreenSaver::run, this);
}

ScreenSaver::~ScreenSaver()
{
    running_ = false;
    if (thread_.joinable()) {
        thread_.join();
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
        logBacklightFailure("no backlight device found");
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

        if (!writeBacklight(backlightPath_, 0)) {
            logBacklightFailure("failed to turn screen off");
            return; // do not update the cached state, retry next tick
        }

        screenOn_ = false;
        LogInfo << "ScreenSaver: screen off (will restore brightness " << restoreBrightness_ << ")" << std::endl;
    } else {
        if (!writeBacklight(backlightPath_, restoreBrightness_)) {
            logBacklightFailure("failed to turn screen on");
            return; // do not update the cached state, retry next tick
        }

        screenOn_ = true;
        LogInfo << "ScreenSaver: screen on (brightness " << restoreBrightness_ << ")" << std::endl;
    }
}

void ScreenSaver::logBacklightFailure(const char *what)
{
    // Caller holds screenMutex_.
    const auto now = std::chrono::steady_clock::now();
    if (now - lastFailLog_ < std::chrono::seconds(SCREENSAVER_LOG_INTERVAL)) {
        return;
    }

    LogError << "ScreenSaver: " << what << " (" << backlightPath_ << ")" << std::endl;
    lastFailLog_ = now;
}
