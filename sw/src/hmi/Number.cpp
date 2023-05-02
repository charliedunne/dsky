#include "Number.h"
#include <Logger.h>

Number::Number(SDL_Renderer *r, const int digits, const int x, const int y, const Color c) {

  // Maximum number of digits
  if (digits <= 5) {

    for (int i = 0; i < digits; ++i) {

      Digit *d = new Digit(r);

      if (i == 0) {

        d->setPosition(x, y);
      }
      else {

        int offset = (S_WIDTH/2*S_FACTOR)*N_SPACE_FACTOR;

        d->setPosition(x+(offset*i), y);
      }

      d->setColor(c);

      digits_.push_back(d);
    }
  }
}

void Number::setValue(const std::string s) {

  if (s.size() <= digits_.size()) {

    value_ = s;
  }
  else {

    LogError << "Excesive digits, expected <= " << digits_.size() << std::endl;
  }

}

void Number::draw() {

  for (int i = 0; i < digits_.size(); ++i) {

      digits_[i]->draw(value_[i]);
  }
}
