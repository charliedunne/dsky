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