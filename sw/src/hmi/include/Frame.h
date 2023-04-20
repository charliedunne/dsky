class Frame {

 private:

  // Frame id
  unsigned int frameId_;

  // Frame size and position
  unsigned int xPos_;
  unsigned int yPos_;
  unsigned int xSize_;
  unsigned int ySize_;

 public:

  // Constructor
  Frame(unsigned int xPos, unsigned int yPos, unsigned int xSize, unsigned int ySize);

  // Destructor
  ~Frame();

  virtual void render() = 0; 

};
