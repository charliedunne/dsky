#include "Number.h"
#include <Logger.h>

Number::Number(SDL_Renderer *r, const int digits, const int x, const int y, const Color c, const bool glow) : Draw(r, x, y)
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
}

void Number::setValue(const std::string s)
{
  value_ = s;
}

void Number::drawOn()
{

  if (value_.size() > digits_.size())
  {
    LogError << "Overflow" << std::endl;

    for (int i = 0; i < digits_.size(); ++i)
    {
      digits_[i]->draw('o');
    }
  }
  else
  {

    for (int i = 0; i < digits_.size(); ++i)
    {

      const int nPadding = digits_.size() - value_.size();

      if (i < nPadding)
      {
        digits_[i]->draw('x');
      }
      else
      {
        digits_[i]->draw(value_[i - nPadding]);
      }
    }
  }
}

void Number::drawOff()
{

  for (int i = 0; i < digits_.size(); ++i)
  {
    digits_[i]->draw('x');
  }
}
