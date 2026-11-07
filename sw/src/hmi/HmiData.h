#ifndef _HMIDATA_H_
#define _HMIDATA_H_

// STL
#include <array>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

// Hmi
#include "LeftLcdView.h"

/**
 * @brief Draw mode for HMI Elements
 */
typedef enum
{
    DRAW_OFF = 0,
    DRAW_ON = 1,
    DRAW_BLINK = 2,
    DRAW_ERROR = 3
} DrawMode;

#define UNASIGNED (0xFFFFFFFF)

/**
 * @brief HMI Right LCD Data structure
 */
typedef struct
{
    // Values
    int nProg;
    int nVerb;
    int nNoun;
    int nR1;
    int nR2;
    int nR3;

    // R1 string value (max 5 chars + NUL). Empty ('\0') when unused.
    char sR1[6];

    // Whether the R1 number widget shows its +/- sign (string values only).
    bool bR1ShowSign;

    // R2 string value (max 5 chars + NUL). Empty ('\0') when unused.
    char sR2[6];

    // R3 string value (max 5 chars + NUL). Empty ('\0') when unused.
    char sR3[6];

    // Modes for Values
    DrawMode nProgMode;
    DrawMode nVerbMode;
    DrawMode nNounMode;
    DrawMode nR1Mode;
    DrawMode nR2Mode;
    DrawMode nR3Mode;
    DrawMode lProgMode;
    DrawMode lVerbMode;
    DrawMode lNounMode;
    DrawMode lCompActyMode;
    DrawMode lR1Mode;
    DrawMode lR2Mode;
    DrawMode lR3Mode;

} RightLcdData;

/**
 * @brief HMI Left LCD alarm flags (flat persistent overlay)
 */
typedef struct
{
    // Flags
    DrawMode lUplinkAct;
    DrawMode lTemp;
    DrawMode lKeyErr;
    DrawMode lDoor;
    DrawMode lOpErr;
    DrawMode lShutdown;
    DrawMode lReboot;

} LeftAlarmFlags;

class HmiData
{

private:
    /**
     * @brief Mutex for safe access to the data. Mutable so the snapshot
     *        getters (getLeftView) can lock from const methods.
     */
    mutable std::mutex mutex_;

    /**
     * @brief Right LCD shared information
     */
    RightLcdData rLcdData_;

    /**
     * @brief Left LCD alarm flags (flat persistent overlay)
     */
    LeftAlarmFlags lAlarms_;

    /**
     * @brief Active left-LCD content view (nullptr = none). Exactly one at a
     *        time; showing a new view replaces the previous one.
     */
    std::shared_ptr<const LeftLcdView> lView_;

    /**
     * @brief Condition source for the TEMP alarm label. True while the
     *        database is reachable (published by DbMonitorProgram).
     *        The displayed lTemp flag is derived from this each frame.
     */
    bool dbHealthy_;

public:
    HmiData();
    virtual ~HmiData();

    const RightLcdData &getRightLcdData();
    void setRightLcdData(const RightLcdData &);

    /**
     * @brief Snapshot copy of the left-LCD alarm flags (taken under the lock,
     *        so the render thread never reads live member memory).
     */
    LeftAlarmFlags getLeftAlarms() const;

    /**
     * @brief Bulk-set the left-LCD alarm flags.
     */
    void setLeftAlarms(const LeftAlarmFlags &);

    void resetRightLcdData();
    void resetLeftLcdData();

    /**
     * @brief Show a left-LCD content view, replacing any previous one.
     *        The view is an immutable snapshot shared across threads.
     */
    void showLeftView(std::shared_ptr<const LeftLcdView> v);

    /**
     * @brief Hide the active content view (does not touch alarm flags).
     */
    void clearLeftView();

    /**
     * @brief Snapshot of the active content view (nullptr when none).
     *        The shared_ptr copy is taken under the lock, so the returned
     *        object stays valid and immutable for the caller.
     */
    std::shared_ptr<const LeftLcdView> getLeftView() const;

    /**
     * @brief Typed accessor: the active view as T, or nullptr when absent.
     */
    template <class T> const T *leftViewAs() const {
      return std::dynamic_pointer_cast<const T>(getLeftView()).get();
    }

    /**
     * @brief Build and show a VerbListView with up to three pages, starting
     *        on page 0. Each page caps at 20 lines / 27 chars per line.
     * @param p1[in] Optional second page (e.g. variable list).
     * @param p2[in] Optional third page (e.g. program list).
     * @param headers[in] Per-page header override; empty entry = default.
     */
    void showVerbList(const std::vector<std::string> &p0,
                      const std::vector<std::string> &p1 = {},
                      const std::vector<std::string> &p2 = {},
                      const std::array<std::string, 3> &headers = {});

    /*     HmiData& operator=(const HmiData &); */

    /* Setters */
    void rSetnProg(int nProg, DrawMode mode);
    void rSetnVerb(int nVerb, DrawMode mode);
    void rSetnNoun(int nNoun, DrawMode mode);
    void rSetnR1(int nR1, DrawMode mode);

    /**
     * @brief Set the R1 field to a string (letters + digits), max 5 chars.
     *        Longer input is truncated with a warning. Clears the integer value.
     */
    void rSetStrR1(const std::string &s, DrawMode mode, bool showSign = false);

    void rSetnR2(int nR2, DrawMode mode);

    /**
     * @brief Set the R2 field to a string (letters + digits), max 5 chars.
     *        Longer input is truncated with a warning. Clears the integer value.
     */
    void rSetStrR2(const std::string &s, DrawMode mode);

    void rSetnR3(int nR3, DrawMode mode);

    /**
     * @brief Set the R3 field to a string (letters + digits), max 5 chars.
     *        Longer input is truncated with a warning. Clears the integer value.
     */
    void rSetStrR3(const std::string &s, DrawMode mode);

    void rSetProgLabel(DrawMode mode);
    void rSetVerbLabel(DrawMode mode);
    void rSetNounLabel(DrawMode mode);
    void rSetCompActLabel(DrawMode mode);
    void rSetR1Label(DrawMode mode);
    void rSetR2Label(DrawMode mode);
    void rSetR3Label(DrawMode mode);

    void lSetUplinkAct(DrawMode mode);
    void lSetTemp(DrawMode mode);
    void lSetKeyErr(DrawMode mode);

    /**
     * @brief Set the DOOR alarm label draw mode (remote UDP signal).
     */
    void lSetDoor(DrawMode mode);

    void lSetOpErr(DrawMode mode);

    /**
     * @brief Set the SHUTDOWN alarm label draw mode (VERB 99 countdown).
     */
    void lSetShutdown(DrawMode mode);

    /**
     * @brief Set the REBOOT alarm label draw mode (VERB 98 countdown).
     */
    void lSetReboot(DrawMode mode);

    /**
     * @brief Build and show a MoonPhaseView, replacing any active view.
     * @param pct[in] Illumination percentage, 0..100.
     * @param phase[in] Lunar phase in [0,1): 0=new, 0.5=full.
     * @param editBlink[in] true to overlay the date-correction blink X
     *        (values are not valid yet).
     */
    void showMoon(int pct, float phase, bool editBlink = false);

    /**
     * @brief Build and show a TempView (TEMP MONITOR screen), replacing any
     *        active view. Rows are pre-formatted by the caller using only
     *        glyphs the Gorton font can draw.
     * @param lines[in] One row per sensor, e.g. "1 KITCHEN 23*5C".
     * @param selected[in] Index into lines of the selected row (-1 = none).
     * @param header[in] Header line; empty falls back to "TEMPERATURES".
     */
    void showTemp(const std::vector<std::string> &lines, int selected, const std::string &header = "TEMPERATURES");

    /**
     * @brief Build and show a VarView (READ/WRITE VAR screen), replacing any
     *        active view. Rows are pre-formatted by the caller using only
     *        glyphs the Gorton font can draw.
     * @param lines[in] One row per variable, e.g. "2 KEYPAD LIGHT 1".
     * @param selected[in] Index into lines of the selected row (-1 = none).
     * @param header[in] Header line; empty falls back to "VARIABLES".
     */
    void showVars(const std::vector<std::string> &lines, int selected, const std::string &header = "VARIABLES");

    /**
     * @brief Build and show a ProgListView (running-programs list screen),
     *        replacing any active view. Rows are pre-formatted by the caller
     *        using only glyphs the Gorton font can draw.
     * @param lines[in] One row per running program, e.g. "P05 MOONPHASE".
     * @param selected[in] Index into lines of the selected row (-1 = none).
     * @param header[in] Header line; empty falls back to "RUNNING PROGRAMS".
     */
    void showProgList(const std::vector<std::string> &lines, int selected, const std::string &header = "RUNNING PROGRAMS");

    /**
     * @brief Publish the database health condition (source of the TEMP alarm).
     *        Called by DbMonitorProgram; the displayed lTemp flag is derived
     *        from this value each frame by HmiLogic.
     */
    void setDbHealthy(bool healthy);

    /**
     * @brief Read the published database health condition.
     *
     * @return true while the database is reachable, false otherwise
     */
    bool isDbHealthy();

    void _debugPrintLcdData();
};

#endif /* _HMIDATA_H_ */