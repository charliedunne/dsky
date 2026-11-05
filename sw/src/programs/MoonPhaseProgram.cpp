#include "MoonPhaseProgram.h"
#include "HmiData.h"
#include <Logger.h>
#include <cmath>
#include <string>

// Synodic month length in days (mean).
#define SYNODIC_MONTH 29.53058867
// Julian date of a known new moon: 2000-01-06 18:14 UTC.
#define NEW_MOON_EPOCH_JD 2451550.258333

/**
 * @brief Julian Day number for a civil (proleptic Gregorian) date at 0h UT.
 * @param y Year (e.g. 2026).
 * @param m Month, 1-12.
 * @param d Day of month.
 * @return Julian Day number (Meeus, Astronomical Algorithms, ch. 7).
 */
static double julianDay(int y, int m, int d) {
    if (m <= 2) {
        y -= 1;
        m += 12;
    }
    int a = y / 100;
    int b = 2 - a + a / 4;
    return floor(365.25 * (y + 4716)) + floor(30.6001 * (m + 1)) + d + b - 1524.5;
}

const int MoonPhaseProgram::need_[3] = {4, 2, 2};

/**
 * @brief Illuminated fraction of the moon for a civil date.
 * @param y Year (e.g. 2026).
 * @param m Month, 1-12.
 * @param d Day of month.
 * @return Illumination fraction in [0, 1]; 0 = new moon, 1 = full moon.
 */
double MoonPhaseProgram::illumination(int y, int m, int d) {
    // Noon UT of the given day: JD at 0h + half a day.
    double jdNoon = julianDay(y, m, d) + 0.5;
    // Phase in [0, 1): fraction of the synodic cycle since the epoch new moon.
    double phase = fmod((jdNoon - NEW_MOON_EPOCH_JD) / SYNODIC_MONTH, 1.0);
    if (phase < 0.0) {
        phase += 1.0;
    }
    return (1.0 - cos(2.0 * M_PI * phase)) / 2.0;
}

/**
 * @brief Number of days in a month, leap-year aware.
 * @param y Year.
 * @param m Month, 1-12.
 * @return Days in the month (28/29 for February).
 */
int MoonPhaseProgram::daysInMonth(int y, int m) {
    static const int table[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (m == 2 && ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0)) {
        return 29;
    }
    return table[m - 1];
}

MoonPhaseProgram::MoonPhaseProgram(HmiData &data) : FgProgram(MOONPHASE_PROGRAM, data, "MOONPHASE") {};

void MoonPhaseProgram::operation() {
    // Digit event codes are non-contiguous bit patterns: use a lookup table.
    static const EventId_t digitKeys[10] = {
        E_KEY_0, E_KEY_1, E_KEY_2, E_KEY_3, E_KEY_4,
        E_KEY_5, E_KEY_6, E_KEY_7, E_KEY_8, E_KEY_9
    };

    // Echo a field's string value on the right LCD (leading zeros preserved).
    auto echoField = [this](Field f, const std::string &s) {
        DrawMode mode = s.empty() ? DRAW_OFF : DRAW_ON;
        switch (f) {
            case YEAR:  hmiData_.rSetStrR1(s, mode); break;
            case MONTH: hmiData_.rSetStrR2(s, mode); break;
            default:    hmiData_.rSetStrR3(s, mode); break;
        }
    };

    // Light or unlight the label of a field on the right LCD.
    auto setLabel = [this](Field f, DrawMode mode) {
        switch (f) {
            case YEAR:  hmiData_.rSetR1Label(mode); break;
            case MONTH: hmiData_.rSetR2Label(mode); break;
            default:    hmiData_.rSetR3Label(mode); break;
        }
    };

    std::string yStr, mStr, dStr;
    std::string digits;
    Field field = YEAR;

    // Blank all echo fields, light the year label.
    echoField(YEAR, "");
    echoField(MONTH, "");
    echoField(DAY, "");
    setLabel(YEAR, DRAW_ON);

    LogTrace << "MOONPHASE: date entry started" << std::endl;

    while (true) {
        EventId_t key;
        if (!nextKey(key)) {
            return;
        }

        // RSET cancels the whole program.
        if (key == E_KEY_RSET) {
            LogTrace << "MOONPHASE: cancelled by RSET" << std::endl;
            return;
        }

        if (field == RESULT) {
            // Result shown: only ENTER finishes, everything else is ignored.
            if (key == E_KEY_ENTR) {
                break;
            }
            continue;
        }

        // Map the key to a digit; ignore non-digit keys during entry.
        int digit = -1;
        for (int i = 0; i < 10; i++) {
            if (key == digitKeys[i]) {
                digit = i;
                break;
            }
        }
        if (digit < 0) {
            continue;
        }

        digits += char('0' + digit);

        if ((int)digits.size() < need_[field]) {
            // Still collecting: echo the buffer so far.
            echoField(field, digits);
            continue;
        }

        // Field complete: validate the value.
        int val = std::stoi(digits);
        bool valid = true;
        switch (field) {
            case YEAR:
                break; // Any 4-digit year is accepted.
            case MONTH:
                valid = (val >= 1 && val <= 12);
                break;
            default: // DAY
                valid = (val >= 1 && val <= daysInMonth(std::stoi(yStr), std::stoi(mStr)));
                break;
        }

        if (!valid) {
            LogTrace << "MOONPHASE: invalid value '" << digits << "' for field " << (int)field
                     << ", re-prompting" << std::endl;
            digits.clear();
            echoField(field, ""); // Clear the field; its label stays lit.
            continue;
        }

        // Commit the field.
        switch (field) {
            case YEAR:  yStr = digits; break;
            case MONTH: mStr = digits; break;
            default:    dStr = digits; break;
        }
        setLabel(field, DRAW_OFF);
        echoField(field, digits);

        if (field == DAY) {
            // All three fields valid: compute and show the illumination.
            int pct = (int)lround(illumination(std::stoi(yStr), std::stoi(mStr), std::stoi(dStr)) * 100.0);
            hmiData_.lSetMoon(DRAW_ON, pct);
            LogTrace << "MOONPHASE: " << yStr << "-" << mStr << "-" << dStr
                     << " -> " << pct << "%" << std::endl;
            field = RESULT;
        } else {
            // Advance to the next field (YEAR -> MONTH -> DAY).
            digits.clear();
            field = (field == YEAR) ? MONTH : DAY;
            setLabel(field, DRAW_ON);
        }
    }

    LogTrace << "MOONPHASE: finished" << std::endl;
}
