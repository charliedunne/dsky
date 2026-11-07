#include "UdpDoorProgram.h"

// STL
#include <cctype>
#include <cstring>
#include <string>

// POSIX networking
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// Configuration
#include "config.h"

// Logging
#include "Logger.h"

UdpDoorProgram::UdpDoorProgram(HmiData &data) :
    BgProgram(UDPDOOR_PROGRAM, data, "UDPDOOR", UDPDOOR_PERIOD_MS),
    sock_(-1)
{
}

UdpDoorProgram::~UdpDoorProgram()
{
  closeSocket();
}

bool UdpDoorProgram::openSocket()
{
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0) {
    LogError << "UdpDoorProgram: socket() failed" << std::endl;
    return false;
  }

  // Allow rebinding after a quick restart.
  int reuse = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

  // Bound the receive timeout so the periodic loop and stop() stay responsive.
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = (UDPDOOR_PERIOD_MS / 2) * 1000;
  setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

  struct sockaddr_in addr;
  std::memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(UDP_DOOR_PORT);

  if (bind(fd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) < 0) {
    LogError << "UdpDoorProgram: bind() to port " << UDP_DOOR_PORT
             << " failed (already in use?)" << std::endl;
    close(fd);
    return false;
  }

  sock_ = fd;
  LogInfo << "UdpDoorProgram: listening on UDP port " << UDP_DOOR_PORT << std::endl;
  return true;
}

void UdpDoorProgram::closeSocket()
{
  if (sock_ >= 0) {
    close(sock_);
    sock_ = -1;
  }
}

void UdpDoorProgram::start()
{
  if (!openSocket()) {
    return;
  }
  BgProgram::start();
}

void UdpDoorProgram::stop()
{
  BgProgram::stop();
  closeSocket();
}

void UdpDoorProgram::operation()
{
  char buf[UDPDOOR_MAX_PAYLOAD];
  const ssize_t n = recvfrom(sock_, buf, sizeof(buf) - 1, 0, nullptr, nullptr);
  if (n < 0) {
    // Timeout or socket error: nothing to do this cycle.
    return;
  }
  buf[n] = '\0';

  // Trim surrounding whitespace so "1\n", "\r\n0" and the like all match.
  char *begin = buf;
  char *end = buf + n;
  while (begin < end && std::isspace(static_cast<unsigned char>(*begin))) ++begin;
  while (end > begin && std::isspace(static_cast<unsigned char>(*(end - 1)))) --end;

  // Lowercase in place for case-insensitive token matching.
  for (char *p = begin; p < end; ++p) {
    *p = static_cast<char>(std::tolower(static_cast<unsigned char>(*p)));
  }
  const std::string token(begin, end);

  if (token == "1" || token == "on" || token == "pos" || token == "true") {
    hmiData_.lSetDoor(DRAW_ON);
    LogInfo << "UdpDoorProgram: positive signal received, DOOR alarm ON" << std::endl;
  } else if (token == "0" || token == "off" || token == "neg" || token == "false") {
    hmiData_.lSetDoor(DRAW_OFF);
    LogInfo << "UdpDoorProgram: negative signal received, DOOR alarm OFF" << std::endl;
  } else {
    LogWarning << "UdpDoorProgram: unrecognized payload ignored: '" << token << "'" << std::endl;
  }
}
