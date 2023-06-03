#ifndef _HMIDATA_H_
#define _HMIDATA_H_

// STL
#include <mutex>

/**
 * @brief Draw mode for HMI Elements
 */
typedef enum
{
    DRAW_OFF = 0,
    DRAW_ON = 1,
    DRAW_BLINK = 2
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

    // Flags

} RightLcdData;

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

public:

    HmiData();
    virtual ~ HmiData();

    const RightLcdData& getRightLcdData();
    void setRightLcdData(const RightLcdData &);

    void resetRightLcdData();

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

    void _debugPrintLcdData();


};

#endif /* _HMIDATA_H_ */