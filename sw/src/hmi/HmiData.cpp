#include "HmiData.h"
#include "VerbListView.h"
#include "MoonPhaseView.h"
#include "TempView.h"
#include "VarView.h"
#include "ProgListView.h"

// STL
#include <algorithm>
#include <cstring>
#include <memory>
#include <vector>

// Logging
#include "Logger.h"

/**
 * @brief Reset the content of the Right LCD Data
 */
void HmiData::resetRightLcdData()
{
    rLcdData_.nProg = UNASIGNED;
    rLcdData_.nVerb = UNASIGNED;
    rLcdData_.nNoun = UNASIGNED;
    rLcdData_.nR1 = UNASIGNED;
    rLcdData_.nR2 = UNASIGNED;
    rLcdData_.nR3 = UNASIGNED;
    rLcdData_.sR1[0] = '\0';
    rLcdData_.bR1ShowSign = false;
    rLcdData_.sR2[0] = '\0';
    rLcdData_.sR3[0] = '\0';

    rLcdData_.nProgMode = DRAW_OFF;
    rLcdData_.nVerbMode = DRAW_OFF;
    rLcdData_.nNounMode = DRAW_OFF;
    rLcdData_.nR1Mode = DRAW_OFF;
    rLcdData_.nR2Mode = DRAW_OFF;
    rLcdData_.nR3Mode = DRAW_OFF;

    rLcdData_.lCompActyMode = DRAW_OFF;
    rLcdData_.lProgMode = DRAW_OFF;
    rLcdData_.lVerbMode = DRAW_OFF;
    rLcdData_.lNounMode = DRAW_OFF;
    rLcdData_.lR1Mode = DRAW_OFF;
    rLcdData_.lR2Mode = DRAW_OFF;
    rLcdData_.lR3Mode = DRAW_OFF;
}

void HmiData::resetLeftLcdData()
{
    lAlarms_.lUplinkAct = DRAW_OFF;
    lAlarms_.lTemp = DRAW_OFF;
    lAlarms_.lKeyErr = DRAW_OFF;
    // lDoor is intentionally NOT reset here: it is event-driven and owned by
    // UdpDoorProgram — only a negative UDP signal may clear it, so an RSET
    // wipe must not hide an active door condition.
    lAlarms_.lOpErr = DRAW_OFF;
    lAlarms_.lShutdown = DRAW_OFF;
    lAlarms_.lReboot = DRAW_OFF;

    // Hide any active content view
    lView_.reset();
}

HmiData::HmiData()
{
    /* Set Default values */
    resetRightLcdData();
    resetLeftLcdData();

    /* Assume the database is healthy until DbMonitorProgram publishes otherwise */
    dbHealthy_ = true;
}

HmiData::~HmiData() {}

/**
 * @brief Getter for the entire Right LCD Structure
 *
 * @return copy of the RIght LCD Data structure
 */
const RightLcdData &HmiData::getRightLcdData()
{
    mutex_.lock();

    /* Value to return */
    RightLcdData &out = rLcdData_;

    mutex_.unlock();

    return out;
}

/**
 * @brief Update the set of Right LCD Data
 *
 * @param data[in] Reference to
 */
void HmiData::setRightLcdData(const RightLcdData &data)
{
    mutex_.lock();

    std::memcpy(&rLcdData_, &data, sizeof(RightLcdData));

    mutex_.unlock();
}

/**
 * @brief Snapshot copy of the left-LCD alarm flags.
 *
 * @return value copy taken under the lock
 */
LeftAlarmFlags HmiData::getLeftAlarms() const
{
    mutex_.lock();

    /* Value to return */
    LeftAlarmFlags out = lAlarms_;

    mutex_.unlock();

    return out;
}

/**
 * @brief Update the set of left-LCD alarm flags
 *
 * @param data[in] Reference to the flags to store
 */
void HmiData::setLeftAlarms(const LeftAlarmFlags &data)
{
    mutex_.lock();

    std::memcpy(&lAlarms_, &data, sizeof(LeftAlarmFlags));

    mutex_.unlock();
}

void HmiData::_debugPrintLcdData()
{
    /* Get copy of the data in the current instant */
    RightLcdData data = getRightLcdData();

    std::vector<std::string> modes;
    modes.push_back("OFF");
    modes.push_back("ON");
    modes.push_back("BLINK");

    /* Print out the information */
    LogDebug << "RightLcd Data:" << std::endl;
    LogDebug << "-------------:" << std::endl;
    LogDebug << "nProg: " << data.nProg << ", " << modes[data.nProgMode] << "." << std::endl;
    LogDebug << "nVerb: " << data.nVerb << ", " << modes[data.nVerbMode] << "." << std::endl;
    LogDebug << "nNoun: " << data.nNoun << ", " << modes[data.nNounMode] << "." << std::endl;
    LogDebug << "nR1:   " << data.nR1 << ", " << modes[data.nR1Mode] << "." << std::endl;
    LogDebug << "nR1:   " << data.nR2 << ", " << modes[data.nR2Mode] << "." << std::endl;
    LogDebug << "nR1:   " << data.nR3 << ", " << modes[data.nR3Mode] << "." << std::endl;

    /** @todo Show data about labels... */
}

/* HmiData &HmiData::operator=(const HmiData &input)
{
    setRightLcdData(input.getRightLcdData());

    return *this;
} */

void HmiData::rSetnProg(int nProg, DrawMode mode)
{
    mutex_.lock();

    rLcdData_.nProg = nProg;
    rLcdData_.nProgMode = mode;

    mutex_.unlock();
}

void HmiData::rSetnVerb(int nVerb, DrawMode mode)
{
    mutex_.lock();

    rLcdData_.nVerb = nVerb;
    rLcdData_.nVerbMode = mode;

    mutex_.unlock();
}

void HmiData::rSetnNoun(int nNoun, DrawMode mode)
{
    mutex_.lock();

    rLcdData_.nNoun = nNoun;
    rLcdData_.nNounMode = mode;

    mutex_.unlock();
}

void HmiData::rSetnR1(int nR1, DrawMode mode)
{
    mutex_.lock();

    rLcdData_.nR1 = nR1;
    rLcdData_.nR1Mode = mode;
    rLcdData_.sR1[0] = '\0';

    mutex_.unlock();
}

/**
 * @brief Set the R1 field to a string (letters + digits), max 5 chars.
 *        Longer input is truncated with a warning. Clears the integer value.
 */
void HmiData::rSetStrR1(const std::string &s, DrawMode mode, bool showSign)
{
    mutex_.lock();

    const size_t len = (s.size() > 5) ? 5 : s.size();
    if (s.size() > 5) {
        LogWarning << "HmiData: rSetStrR1 truncated to 5 chars: " << s << std::endl;
    }
    memcpy(rLcdData_.sR1, s.c_str(), len);
    rLcdData_.sR1[len] = '\0';

    rLcdData_.nR1 = UNASIGNED;
    rLcdData_.nR1Mode = mode;
    rLcdData_.bR1ShowSign = showSign;

    mutex_.unlock();
}

void HmiData::rSetnR2(int nR2, DrawMode mode)
{
    mutex_.lock();

    rLcdData_.nR2 = nR2;
    rLcdData_.nR2Mode = mode;
    rLcdData_.sR2[0] = '\0';

    mutex_.unlock();
}

/**
 * @brief Set the R2 field to a string (letters + digits), max 5 chars.
 *        Longer input is truncated with a warning. Clears the integer value.
 */
void HmiData::rSetStrR2(const std::string &s, DrawMode mode)
{
    mutex_.lock();

    const size_t len = (s.size() > 5) ? 5 : s.size();
    if (s.size() > 5) {
        LogWarning << "HmiData: rSetStrR2 truncated to 5 chars: " << s << std::endl;
    }
    memcpy(rLcdData_.sR2, s.c_str(), len);
    rLcdData_.sR2[len] = '\0';

    rLcdData_.nR2 = UNASIGNED;
    rLcdData_.nR2Mode = mode;

    mutex_.unlock();
}

void HmiData::rSetnR3(int nR3, DrawMode mode)
{
    mutex_.lock();

    rLcdData_.nR3 = nR3;
    rLcdData_.nR3Mode = mode;
    rLcdData_.sR3[0] = '\0';

    mutex_.unlock();
}

/**
 * @brief Set the R3 field to a string (letters + digits), max 5 chars.
 *        Longer input is truncated with a warning. Clears the integer value.
 */
void HmiData::rSetStrR3(const std::string &s, DrawMode mode)
{
    mutex_.lock();

    const size_t len = (s.size() > 5) ? 5 : s.size();
    if (s.size() > 5) {
        LogWarning << "HmiData: rSetStrR3 truncated to 5 chars: " << s << std::endl;
    }
    memcpy(rLcdData_.sR3, s.c_str(), len);
    rLcdData_.sR3[len] = '\0';

    rLcdData_.nR3 = UNASIGNED;
    rLcdData_.nR3Mode = mode;

    mutex_.unlock();
}

void HmiData::rSetProgLabel(DrawMode mode)
{
    mutex_.lock();

    rLcdData_.lProgMode = mode;

    mutex_.unlock();
}

void HmiData::rSetVerbLabel(DrawMode mode)
{
    mutex_.lock();

    rLcdData_.lVerbMode = mode;

    mutex_.unlock();
}

void HmiData::rSetNounLabel(DrawMode mode)
{
    mutex_.lock();

    rLcdData_.lNounMode = mode;

    mutex_.unlock();
}

void HmiData::rSetCompActLabel(DrawMode mode)
{
    mutex_.lock();

    rLcdData_.lCompActyMode = mode;

    mutex_.unlock();
}

void HmiData::rSetR1Label(DrawMode mode)
{
    mutex_.lock();

    rLcdData_.lR1Mode = mode;

    mutex_.unlock();
}

void HmiData::rSetR2Label(DrawMode mode)
{
    mutex_.lock();

    rLcdData_.lR2Mode = mode;

    mutex_.unlock();
}

void HmiData::rSetR3Label(DrawMode mode)
{
    mutex_.lock();

    rLcdData_.lR3Mode = mode;

    mutex_.unlock();
}

void HmiData::lSetUplinkAct(DrawMode mode)
{
    mutex_.lock();

    lAlarms_.lUplinkAct = mode;

    mutex_.unlock();
}

void HmiData::lSetTemp(DrawMode mode)
{
    mutex_.lock();

    lAlarms_.lTemp = mode;

    mutex_.unlock();
}

void HmiData::lSetKeyErr(DrawMode mode)
{
    mutex_.lock();

    lAlarms_.lKeyErr = mode;

    mutex_.unlock();
}

/**
 * @brief Set the DOOR alarm label draw mode (remote UDP door signal).
 */
void HmiData::lSetDoor(DrawMode mode)
{
    mutex_.lock();

    lAlarms_.lDoor = mode;

    mutex_.unlock();
}

void HmiData::lSetOpErr(DrawMode mode)
{
    mutex_.lock();

    lAlarms_.lOpErr = mode;

    mutex_.unlock();
}

/**
 * @brief Set the SHUTDOWN alarm label draw mode (VERB 99 countdown).
 */
void HmiData::lSetShutdown(DrawMode mode)
{
    mutex_.lock();

    lAlarms_.lShutdown = mode;

    mutex_.unlock();
}

/**
 * @brief Set the REBOOT alarm label draw mode (VERB 98 countdown).
 */
void HmiData::lSetReboot(DrawMode mode)
{
    mutex_.lock();

    lAlarms_.lReboot = mode;

    mutex_.unlock();
}

/**
 * @brief Build and show a MoonPhaseView, replacing any active view.
 */
void HmiData::showMoon(int pct, float phase, bool editBlink)
{
    auto view = std::make_shared<MoonPhaseView>();
    view->pct = pct;
    view->phase = phase;
    view->editBlink = editBlink;

    showLeftView(view);
}

/**
 * @brief Build and show a TempView (TEMP MONITOR screen), replacing any active view.
 */
void HmiData::showTemp(const std::vector<std::string> &lines, int selected, const std::string &header)
{
    auto view = std::make_shared<TempView>();
    view->lines = lines;
    view->selected = selected;
    view->header = header;

    showLeftView(view);
}

/**
 * @brief Build and show a VarView (READ/WRITE VAR screen), replacing any active view.
 */
void HmiData::showVars(const std::vector<std::string> &lines, int selected, const std::string &header)
{
    auto view = std::make_shared<VarView>();
    view->lines = lines;
    view->selected = selected;
    view->header = header;

    showLeftView(view);
}

/**
 * @brief Build and show a ProgListView (running-programs list screen), replacing any active view.
 */
void HmiData::showProgList(const std::vector<std::string> &lines, int selected, const std::string &header)
{
    auto view = std::make_shared<ProgListView>();
    view->lines = lines;
    view->selected = selected;
    view->header = header;

    showLeftView(view);
}

/**
 * @brief Publish the database health condition (source of the TEMP alarm).
 *        The displayed lTemp flag is derived from this each frame.
 */
void HmiData::setDbHealthy(bool healthy)
{
    mutex_.lock();

    dbHealthy_ = healthy;

    mutex_.unlock();
}

/**
 * @brief Read the published database health condition.
 *
 * @return true while the database is reachable, false otherwise
 */
bool HmiData::isDbHealthy()
{
    mutex_.lock();

    bool healthy = dbHealthy_;

    mutex_.unlock();

    return healthy;
}

/**
 * @brief Show a left-LCD content view, replacing any previous one.
 */
void HmiData::showLeftView(std::shared_ptr<const LeftLcdView> v)
{
    mutex_.lock();

    lView_ = std::move(v);

    mutex_.unlock();
}

/**
 * @brief Hide the active content view (does not touch alarm flags).
 */
void HmiData::clearLeftView()
{
    mutex_.lock();

    lView_.reset();

    mutex_.unlock();
}

/**
 * @brief Snapshot of the active content view (nullptr when none).
 */
std::shared_ptr<const LeftLcdView> HmiData::getLeftView() const
{
    mutex_.lock();

    std::shared_ptr<const LeftLcdView> out = lView_;

    mutex_.unlock();

    return out;
}

namespace {
/**
 * @brief Cap a list page at 20 lines and truncate each line to 27 chars.
 */
std::vector<std::string> capVerbListPage(const std::vector<std::string> &lines)
{
    const size_t maxLines = 20;
    const size_t lineLen  = 27;

    std::vector<std::string> out;
    out.reserve(std::min(lines.size(), maxLines));

    for (size_t i = 0; i < lines.size() && i < maxLines; ++i) {
        out.push_back(lines[i].substr(0, lineLen));
    }

    return out;
}
} // namespace

void HmiData::showVerbList(const std::vector<std::string> &p0, const std::vector<std::string> &p1,
                           const std::vector<std::string> &p2, const std::array<std::string, 3> &headers)
{
    auto view = std::make_shared<VerbListView>();

    // Page 0: first list. Page 1: second list. Page 2: third list.
    view->pages[0] = capVerbListPage(p0);
    view->pages[1] = capVerbListPage(p1);
    view->pages[2] = capVerbListPage(p2);

    // Per-page header override; empty entry keeps the per-page default.
    for (int i = 0; i < 3; ++i) {
        view->headers[i] = headers[i].substr(0, 27);
    }

    view->page = 0;

    showLeftView(view);
}