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

    // VERB 0 menu: human-readable list of implemented verb/noun combos
    DrawMode lVerbList;                 // DRAW_ON while the list is shown
    char     lVerbListLines[20][28];    // up to 20 lines, each max 27 chars + NUL
    int      lVerbListCount;            // number of valid lines (0..20)

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
     * @brief Fill the VERB 0 menu with the given lines and show it.
     *        Caps at 20 lines / 27 chars per line.
     */
    void lSetVerbList(const std::vector<std::string> &lines);

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
    void rSetnR2(int nR2, DrawMode mode);
    void rSetnR3(int nR3, DrawMode mode);

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

    void _debugPrintLcdData();
};

#endif /* _HMIDATA_H_ */