#ifndef COLOR_H
#define COLOR_H

class Color {

 private:
  int red_ = 0;
  int green_ = 0;
  int blue_ = 0;
  int alpha_ = 0;

 public:

  // Constructor
  Color();
  Color(int r, int g, int b);
  Color(int r, int g, int b, int a);

  // Getters
  int r();
  int g();
  int b();
  int a();

  // Setters
  void set(int r, int g, int b);
  void set(int r, int g, int b, int a);

  // Cast overload
  operator int() const;

};

#endif /* COLOR_H */
