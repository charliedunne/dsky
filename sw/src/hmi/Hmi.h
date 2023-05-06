#ifndef HMI_H
#define HMI_H

#include <vector>
#include "Frame.h"
#include "FrameRightLcd.h"
#include "FrameLeftLcd.h"

#include "SDL2/SDL.h"

typedef struct {

  // Values
  int nProg;
  int nVerb;
  int nNoun;
  int nR1;
  int nR2;
  int nR3;

  // Modes for Values
  DrawMode mProg;
  DrawMode mVerb;
  DrawMode mNoun;
  DrawMode mR1;
  DrawMode mR2;
  DrawMode mR3;

  // Flags
  bool fProg;
  bool fVerb;
  bool fNoun;
  bool fCompActy;
  bool fR1;
  bool fR2;
  bool fR3;
} HmiData;

class Hmi {

 private:

  // Frames
  FrameRightLcd * rightLcd_  = NULL;
  FrameLeftLcd * centerLcd_  = NULL;
  FrameLeftLcd * leftLcd_  = NULL;

  // Window properties
  int w_ = 0;
  int h_ = 0;

  // Window
  SDL_Window * win_ = NULL;

  // Renderer
  SDL_Renderer* ren_ = NULL;

public:

  // Default constructor
  Hmi(int w, int h);

  // Destructor
  ~Hmi();

  // Function to draw the HMI (it must be called every time it needs to be update)
  void render();

  void update(HmiData&);

  // Wait for event
  void wait();

};


#endif /* HMI_H */
