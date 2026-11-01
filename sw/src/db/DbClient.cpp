#include "DbClient.h"

#include <mariadb/mysql.h>

#include <Logger.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>

namespace {

/**
 * @brief Trim leading and trailing whitespace from a string.
 */
std::string trim(const std::string &s)
{
  const auto begin = s.find_first_not_of(" \t\r\n");
  if (begin == std::string::npos) {
    return "";
  }
  const auto end = s.find_last_not_of(" \t\r\n");
  return s.substr(begin, end - begin + 1);
}

/**
 * @brief Parse a key=value line; returns true if the key was recognized.
 */
bool parseLine(const std::string &line, const std::string &key, std::string &value)
{
  const auto eq = line.find('=');
  if (eq == std::string::npos) {
    return false;
  }
  const std::string k = trim(line.substr(0, eq));
  if (k != key) {
    return false;
  }
  value = trim(line.substr(eq + 1));
  return true;
}

} // namespace

DbClient::~DbClient()
{
  stop();
}

/**
 * @brief Parse a key=value config file.
 *
 * Recognized keys: host, port, user, password, database.
 * Lines starting with '#' and blank lines are skipped.
 * Port defaults to 3306 if omitted or non-numeric.
 */
bool DbClient::loadConfig(const std::string &path)
{
  std::ifstream file(path);
  if (!file.is_open()) {
    LogError << "DbClient: cannot open config file: " << path << std::endl;
    return false;
  }

  std::string line;
  bool haveHost = false, haveUser = false, haveDb = false;

  while (std::getline(file, line)) {
    const std::string t = trim(line);
    if (t.empty() || t[0] == '#') {
      continue;
    }
    std::string val;
    if (parseLine(t, "host", val)) {
      host_ = val;
      haveHost = true;
    } else if (parseLine(t, "port", val)) {
      char *endp = nullptr;
      const long p = std::strtol(val.c_str(), &endp, 10);
      port_ = (endp && *endp == '\0' && p > 0) ? static_cast<unsigned int>(p) : 3306;
    } else if (parseLine(t, "user", val)) {
      user_ = val;
      haveUser = true;
    } else if (parseLine(t, "password", val)) {
      password_ = val;
    } else if (parseLine(t, "database", val)) {
      database_ = val;
      haveDb = true;
    }
  }

  if (!haveHost || !haveUser || !haveDb) {
    LogError << "DbClient: config file missing required keys (host/user/database): " << path << std::endl;
    return false;
  }

  LogInfo << "DbClient: loaded config from " << path << " (host=" << host_
          << " port=" << port_ << " user=" << user_ << " db=" << database_ << ")" << std::endl;
  return true;
}

/**
 * @brief Start the background refresh thread.
 */
void DbClient::start()
{
  if (thread_.joinable()) {
    return;
  }
  stop_ = false;
  thread_ = std::thread(&DbClient::threadFunc, this);
}

/**
 * @brief Stop the refresh thread and disconnect from the database.
 */
void DbClient::stop()
{
  if (!thread_.joinable()) {
    return;
  }
  stop_ = true;
  thread_.join();
  /* Thread is joined – safe to touch mysql_ here */
  disconnectDb();
}

/**
 * @brief Return a copy of the current reading cache.
 */
std::vector<DbReading> DbClient::snapshot() const
{
  std::lock_guard<std::mutex> lock(cacheMutex_);
  return readings_;
}

/**
 * @brief Background thread: immediate first refresh, then sleep in 1 s slices.
 */
void DbClient::threadFunc()
{
  LogInfo << "DbClient: refresh thread started (period=" << DB_REFRESH_PERIOD_S << "s)" << std::endl;

  while (!stop_.load()) {
    refreshCycle();

    /* Sleep in 1 s slices so stop() is responsive */
    for (int i = 0; i < DB_REFRESH_PERIOD_S && !stop_.load(); ++i) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }

  LogInfo << "DbClient: refresh thread exiting" << std::endl;
}

/**
 * @brief One full refresh: fetch device list, then latest reading per device.
 *
 * All DB I/O is done outside the cache lock. The merge into readings_ happens
 * under a single lock_guard so snapshot() never blocks on network.
 */
void DbClient::refreshCycle()
{
  if (!ensureConnected()) {
    return;
  }

  /* Fetch device list (id → short_name) */
  std::vector<std::pair<int, std::string>> devices;
  const bool devsOk = queryDevices(devices);

  /* If the device-list query failed, fall back to cached ids */
  std::vector<int> ids;
  if (devsOk) {
    for (const auto &d : devices) {
      ids.push_back(d.first);
    }
  } else {
    std::lock_guard<std::mutex> lock(cacheMutex_);
    for (const auto &r : readings_) {
      ids.push_back(r.id);
    }
  }

  /* Fetch latest reading per device */
  struct LocalReading {
    int id;
    double value;
    time_t ts;
    bool ok;
  };
  std::vector<LocalReading> locals;
  locals.reserve(ids.size());

  for (const auto id : ids) {
    double val = 0.0;
    time_t ts = 0;
    const bool ok = queryLatest(id, val, ts);
    locals.push_back({id, val, ts, ok});
  }

  /* Merge under a single lock */
  std::lock_guard<std::mutex> lock(cacheMutex_);

  if (devsOk) {
    /* Rebuild the cache from the fresh device list */
    const std::vector<DbReading> prev = readings_;
    readings_.clear();
    for (const auto &d : devices) {
      DbReading r;
      r.id = d.first;
      r.name = d.second;
      /* Preserve any previously-fetched value for this id */
      const auto it = std::find_if(prev.begin(), prev.end(),
                                   [&](const DbReading &x) { return x.id == d.first; });
      if (it != prev.end()) {
        r.valid = it->valid;
        r.tempC = it->tempC;
        r.lastUpdate = it->lastUpdate;
      }
      readings_.push_back(std::move(r));
    }
  }

  /* Apply fresh readings */
  for (const auto &lr : locals) {
    if (!lr.ok) {
      continue;
    }
    const auto it = std::find_if(readings_.begin(), readings_.end(),
                                 [&](const DbReading &x) { return x.id == lr.id; });
    if (it != readings_.end()) {
      it->tempC = lr.value;
      it->lastUpdate = lr.ts;
      it->valid = true;
    }
  }
}

/**
 * @brief Ensure a live DB connection, reconnecting if necessary.
 */
bool DbClient::ensureConnected()
{
  if (mysql_ != nullptr) {
    if (mysql_ping(mysql_) == 0) {
      connFailLogged_ = false;
      return true;
    }
    /* Ping failed – drop and reconnect */
    disconnectDb();
  }

  mysql_ = mysql_init(nullptr);
  if (mysql_ == nullptr) {
    if (!connFailLogged_) {
      LogError << "DbClient: mysql_init failed" << std::endl;
      connFailLogged_ = true;
    }
    return false;
  }

  /* Set a bounded connect timeout before connecting */
  const int timeout = DB_CONNECT_TIMEOUT_S;
  mysql_optionsv(mysql_, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);

  if (mysql_real_connect(mysql_, host_.c_str(), user_.c_str(), password_.c_str(),
                         database_.c_str(), port_, nullptr, 0) == nullptr) {
    if (!connFailLogged_) {
      LogWarning << "DbClient: connect failed: " << mysql_error(mysql_) << std::endl;
      connFailLogged_ = true;
    }
    mysql_close(mysql_);
    mysql_ = nullptr;
    return false;
  }

  if (connFailLogged_) {
    LogInfo << "DbClient: reconnected to " << host_ << ":" << port_ << "/" << database_ << std::endl;
  }
  connFailLogged_ = false;
  return true;
}

/**
 * @brief Query the device table for all DS18B20 sensors.
 */
bool DbClient::queryDevices(std::vector<std::pair<int, std::string>> &out) const
{
  out.clear();

  if (mysql_query(mysql_, "SELECT id, short_name FROM device WHERE device_type='DS18B20' ORDER BY id") != 0) {
    LogWarning << "DbClient: queryDevices failed: " << mysql_error(mysql_) << std::endl;
    return false;
  }

  MYSQL_RES *res = mysql_store_result(mysql_);
  if (res == nullptr) {
    /* No result set (should not happen for a SELECT) */
    LogWarning << "DbClient: queryDevices: no result set" << std::endl;
    return false;
  }

  while (MYSQL_ROW row = mysql_fetch_row(res)) {
    if (row[0] == nullptr || row[1] == nullptr) {
      continue;
    }
    const int id = static_cast<int>(std::strtol(row[0], nullptr, 10));
    out.emplace_back(id, std::string(row[1]));
  }

  mysql_free_result(res);
  return true;
}

/**
 * @brief Fetch the latest (value, timestamp) for a given device_id.
 *
 * Uses a prepared statement recreated on every call so it stays valid across
 * reconnects. Returns true if a row was found; false on error or no rows.
 */
bool DbClient::queryLatest(int deviceId, double &valueOut, time_t &tsOut) const
{
  static const char SQL[] =
      "SELECT value, UNIX_TIMESTAMP(timestamp) FROM temp_log WHERE device_id=? ORDER BY id DESC LIMIT 1";

  MYSQL_STMT *stmt = mysql_stmt_init(mysql_);
  if (stmt == nullptr) {
    LogWarning << "DbClient: queryLatest: stmt_init failed: " << mysql_error(mysql_) << std::endl;
    return false;
  }

  if (mysql_stmt_prepare(stmt, SQL, std::strlen(SQL)) != 0) {
    LogWarning << "DbClient: queryLatest: prepare failed: " << mysql_stmt_error(stmt) << std::endl;
    mysql_stmt_close(stmt);
    return false;
  }

  /* Bind the device_id parameter as LONGLONG */
  long long param = static_cast<long long>(deviceId);
  MYSQL_BIND paramBind[1] = {};
  paramBind[0].buffer_type = MYSQL_TYPE_LONGLONG;
  paramBind[0].buffer = &param;

  if (mysql_stmt_bind_param(stmt, paramBind) != 0) {
    LogWarning << "DbClient: queryLatest: bind_param failed: " << mysql_stmt_error(stmt) << std::endl;
    mysql_stmt_close(stmt);
    return false;
  }

  if (mysql_stmt_execute(stmt) != 0) {
    const unsigned int errno_ = mysql_stmt_errno(stmt);
    /* Error 2013 / 2006 etc. – log it */
    LogWarning << "DbClient: queryLatest(" << deviceId << "): execute failed ("
               << errno_ << "): " << mysql_stmt_error(stmt) << std::endl;
    mysql_stmt_close(stmt);
    return false;
  }

  /* Bind result columns as strings into fixed buffers */
  char valBuf[64] = {};
  char tsBuf[64] = {};
  char valIsNull = 0;
  char tsIsNull = 0;

  MYSQL_BIND resBind[2] = {};
  resBind[0].buffer_type = MYSQL_TYPE_STRING;
  resBind[0].buffer = valBuf;
  resBind[0].buffer_length = sizeof(valBuf) - 1;
  resBind[0].is_null = &valIsNull;

  resBind[1].buffer_type = MYSQL_TYPE_STRING;
  resBind[1].buffer = tsBuf;
  resBind[1].buffer_length = sizeof(tsBuf) - 1;
  resBind[1].is_null = &tsIsNull;

  if (mysql_stmt_bind_result(stmt, resBind) != 0) {
    LogWarning << "DbClient: queryLatest(" << deviceId << "): bind_result failed: "
               << mysql_stmt_error(stmt) << std::endl;
    mysql_stmt_close(stmt);
    return false;
  }

  const int fetchRc = mysql_stmt_fetch(stmt);
  /* Capture errno before closing: the statement is invalid afterwards */
  const unsigned int fetchErrno = mysql_stmt_errno(stmt);
  mysql_stmt_close(stmt);

  if (fetchRc == 1) {
    /* No rows – check whether that's a real "no data" or an error */
    if (fetchErrno != 0) {
      LogWarning << "DbClient: queryLatest(" << deviceId << "): fetch error ("
                 << fetchErrno << ")" << std::endl;
      return false;
    }
    /* Genuine no-rows – sensor has never logged */
    return false;
  }

  if (fetchRc != 0) {
    LogWarning << "DbClient: queryLatest(" << deviceId << "): unexpected fetch rc=" << fetchRc << std::endl;
    return false;
  }

  /* Success – parse the string values */
  if (valIsNull || tsIsNull) {
    return false;
  }

  char *endp = nullptr;
  const double val = std::strtod(valBuf, &endp);
  if (endp == valBuf) {
    LogWarning << "DbClient: queryLatest(" << deviceId << "): bad value string '" << valBuf << "'" << std::endl;
    return false;
  }

  const long long ts = std::strtoll(tsBuf, &endp, 10);
  if (endp == tsBuf) {
    LogWarning << "DbClient: queryLatest(" << deviceId << "): bad timestamp string '" << tsBuf << "'" << std::endl;
    return false;
  }

  valueOut = val;
  tsOut = static_cast<time_t>(ts);
  return true;
}

/**
 * @brief Disconnect and free the MySQL handle. Safe to call when mysql_ is null.
 */
void DbClient::disconnectDb()
{
  if (mysql_ != nullptr) {
    mysql_close(mysql_);
    mysql_ = nullptr;
  }
}
