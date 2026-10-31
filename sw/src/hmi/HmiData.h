#ifndef _HMIDATA_H_
#define _HMIDATA_H_

// STL
#include <mutex>
#include <string>
#include <vector>

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
 * @brief HMI Left LCD Data structure
 */
typedef struct
{
    // Flags
    DrawMode lUplinkAct;
    DrawMode lTemp;
    DrawMode lKeyErr;
    DrawMode lOpErr;
    DrawMode lShutdown;
    DrawMode lReboot;

    // VERB 0 menu: two pages — page 0 = verb list, page 1 = variable list
    DrawMode lVerbList;                        // DRAW_ON while the list is shown
    char     lVerbListLines[2][20][28];        // up to 20 lines per page, max 27 chars + NUL
    int      lVerbListCount[2];                // number of valid lines per page (0..20)
    int      lVerbListPage;                    // active page index (0 or 1)
    char     lVerbListHeader[2][28];           // optional header line per page (empty = default)

} LeftLcdData;

class HmiData
{

private:
    /**
     * @brief Mutex for safe access to the data
     */
    std::mutex mutex_;

    /**
     * @brief Right LCD shared information
     */
    RightLcdData rLcdData_;

    /**
     * @brief Right LCD shared information
     */
    LeftLcdData lLcdData_;

public:
    HmiData();
    virtual ~HmiData();

    const RightLcdData &getRightLcdData();
    void setRightLcdData(const RightLcdData &);

    const LeftLcdData &getLeftLcdData();
    void setLeftLcdData(const LeftLcdData &);

    void resetRightLcdData();
    void resetLeftLcdData();

    /**
     * @brief Fill both pages of the VERB 0 menu and show it on page 0.
     *        Page 0 = verb list, page 1 = variable list.
     *        Each page caps at 20 lines / 27 chars per line.
     */
    void lSetVerbList(const std::vector<std::string> &verbs, const std::vector<std::string> &vars);

    /**
     * @brief Switch the active VERB 0 menu page (wraps modulo 2).
     */
    void lSetVerbListPage(int page);

    /**
     * @brief Set the header line of one list page.
     *        An empty string restores the default header for that page.
     */
    void lSetVerbListHeader(int page, const char *header);

    /**
     * @brief Hide/clear the VERB 0 menu (does not touch alarm flags).
     */
    void clearVerbList();

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
    void lSetOpErr(DrawMode mode);

    /**
     * @brief Set the SHUTDOWN alarm label draw mode (VERB 99 countdown).
     */
    void lSetShutdown(DrawMode mode);

    /**
     * @brief Set the REBOOT alarm label draw mode (VERB 98 countdown).
     */
    void lSetReboot(DrawMode mode);

    void _debugPrintLcdData();
};

#endif /* _HMIDATA_H_ */