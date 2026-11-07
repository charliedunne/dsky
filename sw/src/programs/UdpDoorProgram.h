#ifndef _UDPDOORPROGRAM_H_
#define _UDPDOORPROGRAM_H_

#include "BgProgram.h"

/** Program ID for the UDP door monitor (PRO key selection) */
#define UDPDOOR_PROGRAM 4

/** Poll period [ms]: how often pending UDP packets are drained */
#define UDPDOOR_PERIOD_MS 100

/** Maximum payload length accepted from a remote sender [bytes] */
#define UDPDOOR_MAX_PAYLOAD 64

/**
 * @brief Background program: monitors a UDP port for door status signals.
 *
 * Listens on 0.0.0.0:UDP_DOOR_PORT for short text tokens sent by a remote
 * computer:
 *   - positive token ("1", "on", "pos", "true") -> DOOR alarm ON (solid)
 *   - negative token ("0", "off", "neg", "false") -> DOOR alarm OFF
 *
 * Matching is case-insensitive and surrounding whitespace is ignored. The
 * alarm is event-driven and persistent: it stays in its last state until a
 * new packet arrives; stopping the program does not clear it. Unknown
 * payloads are logged and ignored.
 */
class UdpDoorProgram : public BgProgram {

  private:

  /** UDP socket bound to the door port, -1 while not open */
  int sock_;

  /**
   * @brief Create and bind the UDP socket for the door port.
   *
   * @return true on success, false if the socket cannot be created or bound
   */
  bool openSocket();

  /** Close the UDP socket (no-op if already closed) */
  void closeSocket();

  public:
  UdpDoorProgram(HmiData &data);

  virtual ~UdpDoorProgram();

  /**
   * @brief Open the door socket and start the worker thread.
   *
   * Fails gracefully (program stays stopped, error logged) if the port is
   * already bound by another process.
   */
  void start() override;

  /**
   * @brief Stop the worker thread and close the door socket.
   */
  void stop() override;

  /**
   * @brief Drain one pending UDP packet and update the DOOR alarm state.
   */
  void operation() override;
};

#endif /* _UDPDOORPROGRAM_H_ */
