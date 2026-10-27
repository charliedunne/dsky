#ifndef _ALARMLABEL_H_
#define _ALARMLABEL_H_

// HMI classes
#include "Draw.h"
#include "Font.h"

class AlarmLabel : public Draw
{

private:
    /**
     * @brief Pointer to the renderer
     */
    SDL_Renderer *r_ = NULL;

    std::string text_;

    std::string fontBitmap_;
    std::string fontFnt_;

    /**
     * @brief Cached font, constructed once in the constructor.
     * Building a Font per draw decodes the glyph bitmap from disk every frame.
     */
    Font *font_ = nullptr;

    int w_;
    int h_;

    Color bgColor_;
    Color fgColor_;

    /**
     * @brief Target center of the text ink, in window-absolute coordinates.
     * When unset (-1) the text is centered on the label box instead.
     */
    int targetCx_ = -1;
    int targetCy_ = -1;

    void drawLabel(const Color c);

public:
    AlarmLabel(SDL_Renderer *r, const int x, const int y,
               const int w, const int h, std::string text);

    virtual ~AlarmLabel();

    /**
     * @brief Set the Foreground Color
     * 
     * @param fgColor[in] Foreground color.
     */
     void setFgColor(const Color fgColor);

    /**
     * @brief Set the Background Color
     * 
     * @param bgColor[in] Background color.
     */
     void setBgColor(const Color bgColor);

    /**
     * @brief Set the target center for the text ink (window-absolute).
     * The label box geometry is left unchanged; only the text moves.
     *
     * @param cx[in] Target x of the text ink center, in window coordinates.
     * @param cy[in] Target y of the text ink center, in window coordinates.
     */
    void setTargetCenter(const int cx, const int cy);


    // Base class virtual functions
    void drawOn();
    void drawOff();
    void drawErr();
};

#endif /* _ALARMLABEL_H_ */