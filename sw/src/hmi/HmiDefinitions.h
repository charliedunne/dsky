#ifndef _HMIDEFINITIONS_H_
#define _HMIDEFINITIONS_H_

/**
 * @TODO Organize here all the structures to share with the
 * HMI, incliding right and left LCDs
 */

typedef enum
  {
    DRAW_OFF = 0,
    DRAW_ON = 1,
    DRAW_BLINK = 2
  } DrawMode;


/**
 * @brief HMI Data structure
 */
typedef struct {

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

} HmiData;

#endif /* _HMIDEFINITIONS_H_ */
