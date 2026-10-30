#include "HmiData.h"

// STL
#include <cstring>
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
    lLcdData_.lUplinkAct = DRAW_OFF;
    lLcdData_.lTemp = DRAW_OFF;
    lLcdData_.lKeyErr = DRAW_OFF;
    lLcdData_.lOpErr = DRAW_OFF;
    lLcdData_.lShutdown = DRAW_OFF;
    lLcdData_.lReboot = DRAW_OFF;

    // Clear the VERB 0 menu
    lLcdData_.lVerbList = DRAW_OFF;
    lLcdData_.lVerbListCount[0] = 0;
    lLcdData_.lVerbListCount[1] = 0;
    lLcdData_.lVerbListPage = 0;
}

HmiData::HmiData()
{
    /* Set Default values */
    resetRightLcdData();
    resetLeftLcdData();
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
 * @brief Getter for the entire Left LCD Structure
 *
 * @return copy of the Left LCD Data structure
 */
const LeftLcdData &HmiData::getLeftLcdData()
{
    mutex_.lock();

    /* Value to return */
    LeftLcdData &out = lLcdData_;

    mutex_.unlock();

    return out;
}

/**
 * @brief Update the set of Left LCD Data
 *
 * @param data[in] Reference to
 */
void HmiData::setLeftLcdData(const LeftLcdData &data)
{
    mutex_.lock();

    std::memcpy(&lLcdData_, &data, sizeof(LeftLcdData));

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
void HmiData::rSetStrR1(const std::string &s, DrawMode mode)
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

    mutex_.unlock();
}

void HmiData::rSetnR2(int nR2, DrawMode mode)
{
    mutex_.lock();

    rLcdData_.nR2 = nR2;
    rLcdData_.nR2Mode = mode;

    mutex_.unlock();
}

void HmiData::rSetnR3(int nR3, DrawMode mode)
{
    mutex_.lock();

    rLcdData_.nR3 = nR3;
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

    lLcdData_.lUplinkAct = mode;

    mutex_.unlock();
}

void HmiData::lSetTemp(DrawMode mode)
{
    mutex_.lock();

    lLcdData_.lTemp = mode;

    mutex_.unlock();
}

void HmiData::lSetKeyErr(DrawMode mode)
{
    mutex_.lock();

    lLcdData_.lKeyErr = mode;

    mutex_.unlock();
}

void HmiData::lSetOpErr(DrawMode mode)
{
    mutex_.lock();

    lLcdData_.lOpErr = mode;

    mutex_.unlock();
}

/**
 * @brief Set the SHUTDOWN alarm label draw mode (VERB 99 countdown).
 */
void HmiData::lSetShutdown(DrawMode mode)
{
    mutex_.lock();

    lLcdData_.lShutdown = mode;

    mutex_.unlock();
}

/**
 * @brief Set the REBOOT alarm label draw mode (VERB 98 countdown).
 */
void HmiData::lSetReboot(DrawMode mode)
{
    mutex_.lock();

    lLcdData_.lReboot = mode;

    mutex_.unlock();
}

namespace {
/**
 * @brief Copy up to 20 lines into one page buffer of the VERB 0 menu.
 */
void fillVerbListPage(char (&page)[20][28], int &count, const std::vector<std::string> &lines)
{
    const int maxLines = 20;
    const int lineLen  = 28;   // 27 chars + NUL

    int n = (int)lines.size();
    if (n > maxLines) n = maxLines;

    for (int i = 0; i < n; ++i) {
        std::strncpy(page[i], lines[i].c_str(), lineLen - 1);
        page[i][lineLen - 1] = '\0';
    }

    count = n;
}
} // namespace

void HmiData::lSetVerbList(const std::vector<std::string> &verbs, const std::vector<std::string> &vars)
{
    mutex_.lock();

    // Page 0: verb list. Page 1: variable list.
    fillVerbListPage(lLcdData_.lVerbListLines[0], lLcdData_.lVerbListCount[0], verbs);
    fillVerbListPage(lLcdData_.lVerbListLines[1], lLcdData_.lVerbListCount[1], vars);

    lLcdData_.lVerbListPage = 0;
    lLcdData_.lVerbList = DRAW_ON;

    mutex_.unlock();
}

void HmiData::lSetVerbListPage(int page)
{
    mutex_.lock();

    // Wrap around modulo 2 (handles negative input as well).
    lLcdData_.lVerbListPage = ((page % 2) + 2) % 2;

    mutex_.unlock();
}

void HmiData::clearVerbList()
{
    mutex_.lock();

    lLcdData_.lVerbList = DRAW_OFF;
    lLcdData_.lVerbListCount[0] = 0;
    lLcdData_.lVerbListCount[1] = 0;
    lLcdData_.lVerbListPage = 0;

    mutex_.unlock();
}