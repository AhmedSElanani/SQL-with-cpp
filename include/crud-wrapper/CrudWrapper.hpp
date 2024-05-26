#pragma once

#include "ICruddable.hpp"
#include <concepts>
#include <exception>
#include <filesystem>
#include <memory>
#include <sqlite3.h>

/// @brief namespace for SQL with c++
namespace sql_with_cpp {

/// @brief class for CRUD operations implementing the interface ICruddable
/// @note this class is based on sqlite3 database engine
class CrudWrapper : public ICruddable {
  /// @brief a custom deleter for sqlite3
  struct Sqlite3Closer {
    void operator()(sqlite3 *p) const { sqlite3_close(p); }
  };

  /// @brief a custom deleter for sqlite3_stmt
  struct Sqlite3StmtCloser {
    void operator()(sqlite3_stmt *p) const { sqlite3_finalize(p); }
  };

public:
  /// @brief deleted default constructor for allowing only construction when
  ///        passing a path to the database
  /// @note this is the default behavior since a parametrized constructor is
  ///       already defined, but this is just for explicitly, and defensiveness
  CrudWrapper() = delete;

  /// @brief parametrized constructor for CRUD wrapper class
  /// @param path filesystem path to the database
  explicit CrudWrapper(
      const std::convertible_to<std::filesystem::path> auto &path)
      : m_db_path{path} {
    if (std::error_code err; std::filesystem::exists(m_db_path, err) == false) {
      throw std::filesystem::filesystem_error(
          "Path to database not found! Error code: ", err);
    }

    sqlite3 *dbPtr{nullptr};
    const auto rCode{sqlite3_open(m_db_path.string().c_str(), &dbPtr)};
    if (rCode != SQLITE_OK) {
      throw std::runtime_error(
          std::string{"Failed to open database, sqlite3 error: "} +
          sqlite3_errstr(rCode));
    }

    m_db = std::unique_ptr<sqlite3, Sqlite3Closer>{dbPtr};
  }

private:
  /// @brief path to the database to connect to
  const std::filesystem::path m_db_path{""};

  /// @brief unique pointer that owns the handle to the sqlite3 database
  std::unique_ptr<sqlite3, Sqlite3Closer> m_db{nullptr};

  /// @brief unique pointer that owns the handle to the sqlite3 database
  ///        statement
  std::unique_ptr<sqlite3_stmt, Sqlite3StmtCloser> m_stmt{nullptr};
};

} // namespace sql_with_cpp
