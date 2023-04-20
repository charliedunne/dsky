#include <string>

class Digits {

 private:

  // Number of digits 
  unsigned int digits_;

  // String to show
  int value_;

 public:

  // Constructor
  Digits();

  // Destructor
  ~Digits();

  // Enable Label
  void SwitchOn();

  // Disable Label
  void SwitchOn();

  // Blink Label
  void Blink(unsigned int freq);

}
