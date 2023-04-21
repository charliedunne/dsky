#ifndef LABEL_H
#define LABEL_H


#include <string>


class Label {

 private:

  // String to show
  string text_;

  // Foreground color
  string fgColor_;

  // Background color
  string bgColor_;

 public:

  // Constructor
  Label();

  // Destructor
  ~Label();

  // Enable Label
  void SwitchOn();

  // Disable Label
  void SwitchOn();

  // Blink Label
  void Blink(unsigned int freq);

}

#endif /* LABEL_H */
