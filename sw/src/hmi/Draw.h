#ifndef DRAW_H
#define DRAW_H

// Graphics Libraries
#include "SDL2/SDL.h"

// External configuration
#include "config.h"

// HMI Definitions
#include "HmiData.h"

class Draw
{

protected:

    /**
     * @brief X position (left-upper corner)
     * 
     */
    int x_;

    /**
     * @brief Y position (left-upper corner)
     * 
     */
    int y_;


private:
    /**
     * @brief Internal variable that mange the mode
     * of the drawable object.
     */
    DrawMode mode_;

    /**
     * @brief Blinking frequency [in frames]
     *
     * @note The blinking period is for the entire animation. The
     * object will be visible for the half of the time and invisible
     * the other half. 
     */
    int blinkPeriod_;

    /**
     * @brief Internal counter for the blinking logic
     * 
     */
    int frameCounter_;

    /**
     * @brief Behaviour of the object rendering when the
     * mode is set to @b ON
     *
     * @attention This is a pure virtual function so the
     * implementation must be provided in derived classes
     *
     * @attention this function will be in the private
     * domain since it will be called internally by draw(),
     * but it is not meant to be available in the public
     * interface
     */
    virtual void drawOff() = 0;

    /**
     * @brief Behaviour of the object rendering when the
     * mode is set to @b ON
     *
     * @attention This is a pure virtual function so the
     * implementation must be provided in derived classes
     *
     * @attention this function will be in the private
     * domain since it will be called internally by draw(),
     * but it is not meant to be available in the public
     * interface
     */
    virtual void drawOn() = 0;

public:

    /**
     * @brief Construct a new Draw object
     * 
     * @param x [in] X position (left-top corner) [pixels]
     * @param y [in] Y position (left-top corner) [pixels]
     * @param period [OPTIONAL] Blinking period [frames]
     */
    Draw(const int x, const int y, 
    const int period = DEFAULT_BLINKING_PERIOD);

    /**
     * @brief Set the Mode object
     * 
     * @param mode [in] Desired mode (@b ON/OFF/BLINK)
     * @param period [OPTIONAL] Blinking period [frames]
     */
    void setMode(const DrawMode mode, const int period = DEFAULT_BLINKING_PERIOD);

    /**
     * @brief Drawing the object in the scene. This function
     * will render the object according to the selected mode
     * @b ON/OFF/BLINK
     */
    void draw();

    // Setters / Getterns

    /**
     * @brief Set the Blinking Period
     * 
     * @param period [in] Period [frames]
     */
    void setBlinkingPeriod(const int period);
};

#endif /* DRAW_H */
