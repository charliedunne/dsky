#ifndef _BGPROGRAM_H_
#define _BGPROGRAM_H_

#include <thread>
#include "Program.h"

/**
 * @brief Base class for background programs.
 *
 * A background program runs a periodic loop on its own worker thread and does
 * not require user input while running. The HMI stays free (MODE_IDLE), so
 * VERB actions can run concurrently with the program.
 */
class BgProgram : public Program {

  private:

  /**
   * @brief Worker thread executing the periodic loop
   */
  std::thread thread_;

  /**
   * @brief Periodic loop executed on the worker thread.
   *
   * Calls operation() once per period until stop() clears the running flag.
   */
  void loop();

  public:
  BgProgram(int id, HmiData &data, const std::string& label = "", int period = 0);

  virtual ~BgProgram();

  /**
   * @brief Start the program and spawn its worker thread.
   */
  virtual void start() override;

  /**
   * @brief Stop the program and join its worker thread. No-op if not running.
   */
  virtual void stop() override;
};

#endif /* _BGPROGRAM_H_ */
