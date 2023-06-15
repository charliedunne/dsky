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

    int w_;
    int h_;

    Color bgColor_;
    Color fgColor_;

    void drawLabel(const Color c);

public:
    AlarmLabel(SDL_Renderer *r, const int x, const int y,
               const int w, const int h, std::string text);

    virtual ~AlarmLabel();

    // Base class virtual functions
    void drawOn();
    void drawOff();
    void drawErr();
};

#endif /* _ALARMLABEL_H_ */