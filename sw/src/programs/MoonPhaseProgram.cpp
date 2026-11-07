#include "MoonPhaseProgram.h"
#include "HmiData.h"
#include "MoonPhaseView.h"
#include <Logger.h>
#include <cmath>
#include <cstdio>
#include <ctime>
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
 * @brief Lunar phase for a civil date.
 * @param y Year (e.g. 2026).
 * @param m Month, 1-12.
 * @param d Day of month.
 * @return Phase in [0, 1): 0 = new moon, 0.5 = full moon.
 */
double MoonPhaseProgram::moonPhase(int y, int m, int d) {
    // Noon UT of the given day: JD at 0h + half a day.
    double jdNoon = julianDay(y, m, d) + 0.5;
    // Phase in [0, 1): fraction of the synodic cycle since the epoch new moon.
    double phase = fmod((jdNoon - NEW_MOON_EPOCH_JD) / SYNODIC_MONTH, 1.0);
    if (phase < 0.0) {
        phase += 1.0;
    }
    return phase;
}

/**
 * @brief Illuminated fraction of the moon for a civil date.
 * @param y Year (e.g. 2026).
 * @param m Month, 1-12.
 * @param d Day of month.
 * @return Illumination fraction in [0, 1]; 0 = new moon, 1 = full moon.
 */
double MoonPhaseProgram::illumination(int y, int m, int d) {
    const double phase = moonPhase(y, m, d);
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

void MoonPhaseProgram::stepDate(int &y, int &m, int &d, int delta) {
    while (delta > 0) {
        if (d < daysInMonth(y, m)) {
            d++;
        } else {
            d = 1;
            if (m < 12) {
                m++;
            } else {
                m = 1;
                y++;
            }
        }
        delta--;
    }
    while (delta < 0) {
        if (d > 1) {
            d--;
        } else {
            if (m == 1) {
                m = 12;
                y--;
            } else {
                m--;
            }
            d = daysInMonth(y, m);
        }
        delta++;
    }
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

    // Set the draw mode of a field's label (RX_LINE) on the right LCD:
    // blinking while the field is being entered, fixed once committed.
    auto setLabel = [this](Field f, DrawMode mode) {
        switch (f) {
            case YEAR:  hmiData_.rSetR1Label(mode); break;
            case MONTH: hmiData_.rSetR2Label(mode); break;
            default:    hmiData_.rSetR3Label(mode); break;
        }
    };

    // Hide any content view on the left LCD (e.g. a previous MOONPHASE result).
    auto clearLeftLcd = [this]() {
        hmiData_.clearLeftView();
    };

    // Toggle the date-correction blink X on an active MOONPHASE result by
    // publishing a copy of the current view with a different editBlink flag.
    auto setEditBlink = [this](bool on) {
        if (const MoonPhaseView *m = hmiData_.leftViewAs<MoonPhaseView>()) {
            auto n = std::make_shared<MoonPhaseView>(*m);
            n->editBlink = on;
            hmiData_.showLeftView(n);
        }
    };

    std::string yStr, mStr, dStr;
    std::string digits;
    Field field = YEAR;

    // Blank all echo fields, blink the year label (active input prompt).
    echoField(YEAR, "");
    echoField(MONTH, "");
    echoField(DAY, "");
    setLabel(YEAR, DRAW_BLINK);

    LogTrace << "MOONPHASE: date entry started" << std::endl;

    while (true) {
        EventId_t key;
        if (!nextKey(key)) {
            clearLeftLcd();
            return;
        }

        // RSET cancels the whole program.
        if (key == E_KEY_RSET) {
            LogTrace << "MOONPHASE: cancelled by RSET" << std::endl;
            clearLeftLcd();
            return;
        }

        if (field == RESULT) {
            // Result shown: ENTER finishes; + / - step the date by one day
            // (with month/year rollover) and refresh the graphic; CLR re-opens
            // the day field so the operator can correct the date. While the
            // date is incomplete again, the left LCD blinks an X over the
            // previous moon image. Everything else is ignored.
            if (key == E_KEY_ENTR) {
                break;
            }
            if (key == E_KEY_PLUS || key == E_KEY_MINUS) {
                // Step the full date by one day, with month/year rollover.
                int y = std::stoi(yStr);
                int m = std::stoi(mStr);
                int d = std::stoi(dStr);
                stepDate(y, m, d, (key == E_KEY_PLUS) ? 1 : -1);
                char buf[8];
                snprintf(buf, sizeof(buf), "%04d", y);
                yStr = buf;
                snprintf(buf, sizeof(buf), "%02d", m);
                mStr = buf;
                snprintf(buf, sizeof(buf), "%02d", d);
                dStr = buf;
                echoField(YEAR, yStr);
                echoField(MONTH, mStr);
                echoField(DAY, dStr);
                const double phase = moonPhase(y, m, d);
                const int pct = (int)lround(illumination(y, m, d) * 100.0);
                hmiData_.showMoon(pct, (float)phase);
                LogTrace << "MOONPHASE: stepped date to " << yStr << "-" << mStr
                         << "-" << dStr << std::endl;
                continue;
            }
            if (key == E_KEY_CLR) {
                dStr.clear();
                digits.clear(); // stale committed day must not auto-commit
                echoField(DAY, "");
                setLabel(DAY, DRAW_BLINK);
                setEditBlink(true);
                field = DAY;
                LogTrace << "MOONPHASE: CLR at result - re-opening day editing" << std::endl;
                continue;
            }
            continue;
        }

        // CLR undoes the last data introduced, LIFO: first the current field's
        // partial buffer, then previously committed fields (R3 -> R2 -> R1).
        if (key == E_KEY_CLR) {
            if (!digits.empty()) {
                digits.clear();
                echoField(field, "");
                LogTrace << "MOONPHASE: CLR - cleared partial input of field "
                         << (int)field << std::endl;
            } else if (field > YEAR) {
                // Pop the previously committed field back to active.
                Field prev = (Field)(field - 1);
                switch (prev) {
                    case YEAR:  yStr.clear(); break;
                    case MONTH: mStr.clear(); break;
                    default:    dStr.clear(); break;
                }
                setLabel(field, DRAW_OFF); // the field left is no longer active
                echoField(prev, "");
                field = prev;
                setLabel(field, DRAW_BLINK);
                LogTrace << "MOONPHASE: CLR - cleared committed field "
                         << (int)prev << std::endl;
            }
            // Nothing left to clear: the event is ignored.
            continue;
        }

        // ENTER while waiting for year input with no digits entered:
        // auto-fill the current system date and show the phase graphic.
        if (key == E_KEY_ENTR && field == YEAR && digits.empty()) {
            time_t now = time(nullptr);
            struct tm *lt = localtime(&now);
            int y = lt->tm_year + 1900;
            int m = lt->tm_mon + 1;
            int d = lt->tm_mday;
            char buf[8];
            snprintf(buf, sizeof(buf), "%04d", y);
            yStr = buf;
            snprintf(buf, sizeof(buf), "%02d", m);
            mStr = buf;
            snprintf(buf, sizeof(buf), "%02d", d);
            dStr = buf;
            echoField(YEAR, yStr);
            echoField(MONTH, mStr);
            echoField(DAY, dStr);
            setLabel(YEAR, DRAW_ON);
            setLabel(MONTH, DRAW_ON); // default-off labels must be turned on
            setLabel(DAY, DRAW_ON);   // explicitly in this path
            const double phase = moonPhase(y, m, d);
            const int pct = (int)lround(illumination(y, m, d) * 100.0);
            hmiData_.showMoon(pct, (float)phase);
            LogTrace << "MOONPHASE: ENTER with empty year - using current date "
                     << yStr << "-" << mStr << "-" << dStr
                     << " -> " << pct << "% phase=" << phase << std::endl;
            field = RESULT;
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
            echoField(field, ""); // Clear the field; its label keeps blinking.
            continue;
        }

        // Commit the field.
        switch (field) {
            case YEAR:  yStr = digits; break;
            case MONTH: mStr = digits; break;
            default:    dStr = digits; break;
        }
        setLabel(field, DRAW_ON); // committed: label stays fixed on
        echoField(field, digits);

        if (field == DAY) {
            // All three fields valid: compute and show the phase graphic.
            const double phase = moonPhase(std::stoi(yStr), std::stoi(mStr), std::stoi(dStr));
            int pct = (int)lround(illumination(std::stoi(yStr), std::stoi(mStr), std::stoi(dStr)) * 100.0);
            hmiData_.showMoon(pct, (float)phase); // date complete: edit-blink off
            LogTrace << "MOONPHASE: " << yStr << "-" << mStr << "-" << dStr
                     << " -> " << pct << "% phase=" << phase << std::endl;
            field = RESULT;
        } else {
            // Advance to the next field (YEAR -> MONTH -> DAY).
            digits.clear();
            field = (field == YEAR) ? MONTH : DAY;
            setLabel(field, DRAW_BLINK);
        }
    }

    clearLeftLcd();
    LogTrace << "MOONPHASE: finished" << std::endl;
}
