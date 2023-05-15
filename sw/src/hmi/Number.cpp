#include "Number.h"
#include <Logger.h>

Number::Number(SDL_Renderer *r, const int digits, const int x, const int y, const Color c, const bool glow) : Draw(x, y)
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

    // Set-up sign
    showSign_ = false;

    sign_ = new Sign(r);
    sign_->setPosition(x - S_WIDTH / 6, y);
    sign_->setColor(c);
    sign_->setGlow(glow);
  }
}

void Number::setValue(const std::string s)
{
  value_ = s;
}

void Number::drawOn()
{
  int valueSize = value_.size();

  /* Check negative values */
  if (value_[0] == '-')
  {
    valueSize--;
  }

  if (valueSize > digits_.size())
  {
    LogError << "Overflow" << std::endl;

    for (int i = 0; i < digits_.size(); ++i)
    {
      digits_[i]->draw('o');
    }
  }
  else
  {
    // Draw sign
    if (showSign_)
    {
      if (value_[0] == '-')
      {
        sign_->draw('-');
      }
      else
      {
        sign_->draw('+');
      }
    }

    // Draw digits
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

void Number::enableSign()
{
  showSign_ = true;
}

void Number::disableSign()
{
  showSign_ = false;
}