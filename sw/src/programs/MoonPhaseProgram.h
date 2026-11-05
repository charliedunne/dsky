#ifndef _MOONPHASEPROGRAM_H_
#define _MOONPHASEPROGRAM_H_

#include "FgProgram.h"

/* Program id selected on the keypad to launch the moon phase program */
#define MOONPHASE_PROGRAM 3

/**
 * @brief Foreground program that shows the moon phase illumination for a date.
 *
 * The operator enters a calendar date through the keypad, field by field:
 * R1 = year (4 digits), R2 = month (2 digits), R3 = day (2 digits). Fields
 * commit and advance automatically as soon as enough digits have been typed
 * (8 key presses in total, no ENTER needed between fields). The active field's
 * label blinks while digits are being entered and stays fixed on once the
 * field is committed. When the date is complete the program computes the
 * lunar illumination fraction for that day (0..100 %) and displays it on the
 * left LCD. ENTER finishes the program; RSET at any point cancels it.
 * During entry, CLR undoes the last data introduced by the operator, LIFO:
 * first the current field's partial digits, then previously committed fields
 * (R3 -> R2 -> R1); extra presses are ignored once nothing is left to clear.
 * After the result is shown, CLR re-opens the day field so the date can be
 * corrected; further CLRs keep unwinding LIFO (month, then year). While the
 * date is incomplete again, the left LCD blinks an X over the previous moon
 * graphic; completing the date removes it.
 */
class MoonPhaseProgram : public FgProgram {

  private:

  /**
   * @brief Input fields collected from the keypad, in entry order
   */
  enum Field { YEAR = 0, MONTH = 1, DAY = 2, RESULT };

  /**
   * @brief Digits required to commit each field (year=4, month=2, day=2)
   */
  static const int need_[3];

  /**
   * @brief Compute the lunar illumination fraction (0..1) for a civil date.
   *
   * @param y [in] Year (e.g. 2026)
   * @param m [in] Month (1..12)
   * @param d [in] Day of month (1..daysInMonth)
   * @return Illumination fraction in the range [0, 1].
   */
  double illumination(int y, int m, int d);

  /**
   * @brief Lunar phase for a civil date.
   *
   * @param y [in] Year (e.g. 2026)
   * @param m [in] Month (1..12)
   * @param d [in] Day of month (1..daysInMonth)
   * @return Phase in [0, 1): 0 = new moon, 0.5 = full moon.
   */
  double moonPhase(int y, int m, int d);

  /**
   * @brief Number of days in a given month (leap-year aware).
   *
   * @param y [in] Year
   * @param m [in] Month (1..12)
   * @return Number of days in the month.
   */
  static int daysInMonth(int y, int m);

  public:
  MoonPhaseProgram(HmiData &data);

  /**
   * @brief Collect the date from the keypad and show the illumination %.
   */
  void operation() override;
};

#endif /* _MOONPHASEPROGRAM_H_ */
