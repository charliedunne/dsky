#include "Number.h"
#include <Logger.h>

Number::Number(SDL_Renderer *r, const int digits, const int x, const int y, const Color c, const bool glow)
{

  // Maximum number of digits
  if (digits <= 5)
  {

    for (int i = 0; i < digits; ++i)
    {

      Digit *d = new Digit(r);

      if (i == 0)
      {

        d->setPosition(x, y);
      }
      else
      {

        int offset = (S_WIDTH / 2 * S_FACTOR) * N_SPACE_FACTOR;

        d->setPosition(x + (offset * i), y);
      }

      d->setColor(c);

      d->setGlow(glow);

      digits_.push_back(d);
    }
  }

  nMode_ = N_ON;
  blinkFrames_ = DEFAULT_BLINK_FRAMES;
  frameCounter_ = 0;
}

void Number::setValue(const std::string s)
{
  value_ = s;
}

void drawNumber(std::vector<Digit *> &digits, std::string &value)
{
  if (value.size() > digits.size())
  {
    LogError << "Overflow" << std::endl;

    for (int i = 0; i < digits.size(); ++i)
    {
      digits[i]->draw('o');
    }
  }
  else
  {

    for (int i = 0; i < digits.size(); ++i)
    {

      const int nPadding = digits.size() - value.size();

      if (i < nPadding)
      {
        digits[i]->draw('x');
      }
      else
      {
        digits[i]->draw(value[i - nPadding]);
      }
    }
  }
}

void Number::draw()
{

  if ((nMode_ == N_ON) || ((nMode_ == N_BLINK) && (frameCounter_ < blinkFrames_ / 2))) {
    drawNumber(digits_, value_);
  }
  else {
    for (int i = 0; i < digits_.size(); ++i)
    {
      digits_[i]->draw('x');
    }

  }

  // Increase the frames counter
  frameCounter_ = (frameCounter_ + 1) % blinkFrames_;
}



void Number::setMode(const NumberMode mode, const int frames)
{

  if (mode == N_BLINK)
  {
    blinkFrames_ = frames;
  }

  nMode_ = mode;
}
