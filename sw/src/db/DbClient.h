#pragma once

#include <atomic>
#include <ctime>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

/* Forward declarations to avoid pulling mariadb headers into consumers */
struct st_mysql;
struct st_mysql_stmt;

/**
 * @brief A single temperature sensor reading cached from the database.
 */
struct DbReading {
  int id = 0;            ///< device_id in the holanda2 DB
  std::string name;      ///< device.short_name
  double tempC = 0.0;    ///< latest value (Celsius)
  bool valid = false;    ///< true once at least one successful fetch has occurred
  time_t lastUpdate = 0; ///< UNIX_TIMESTAMP of the row (data age, not fetch time)
};

/**
 * @brief Background client that periodically refreshes DS18B20 readings from MariaDB.
 *
 * A dedicated thread connects on start, queries all sensor devices and their latest
 * readings every DB_REFRESH_PERIOD_S seconds, and keeps a mutex-guarded cache that
 * consumers read via snapshot(). No consumer thread ever blocks on DB I/O.
 */
class DbClient {
public:
  static constexpr int DB_REFRESH_PERIOD_S = 30; ///< refresh cadence in seconds
  static constexpr int DB_CONNECT_TIMEOUT_S = 3; ///< bounded connect timeout in seconds

  /**
   * @brief Parse a key=value config file (host, port, user, password, database).
   * @param path Filesystem path to the config file.
   * @return true on success; false if the file is missing or required keys are absent.
   */
  bool loadConfig(const std::string &path);

  /**
   * @brief Start the background refresh thread. No-op if already running.
   */
  void start();

  /**
   * @brief Signal the refresh thread to stop and join it, then disconnect.
   */
  void stop();

  ~DbClient();

  /**
   * @brief Return a copy of the current cache (sorted by device id).
   */
  std::vector<DbReading> snapshot() const;

private:
  void threadFunc();
  void refreshCycle();
  bool ensureConnected();
  bool queryDevices(std::vector<std::pair<int, std::string>> &out) const;
  bool queryLatest(int deviceId, double &valueOut, time_t &tsOut) const;
  void disconnectDb();

  /* Connection parameters (set by loadConfig) */
  std::string host_;
  unsigned int port_ = 3306;
  std::string user_;
  std::string password_;
  std::string database_;

  /* Thread lifecycle */
  std::thread thread_;
  std::atomic<bool> stop_{true};

  /* DB handle – touched only by the refresh thread and by stop() after join */
  st_mysql *mysql_ = nullptr;
  bool connFailLogged_ = false;

  /* Cache */
  mutable std::mutex cacheMutex_;
  std::vector<DbReading> readings_;
};
