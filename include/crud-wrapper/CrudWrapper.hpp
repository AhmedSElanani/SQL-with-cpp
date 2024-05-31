#pragma once

#include "ICruddable.hpp"
#include <concepts>
#include <exception>
#include <filesystem>
#include <memory>
#include <sqlite3.h>
#include <string>
#include <vector>

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

  /// @brief method to get the columns names in a given table
  /// @param tableName the name of the table to peek its columns names
  /// @return a vector of strings that represent name of each column
  auto peekColumnsNames(std::string const &tableName) const
      -> std::vector<std::string> {
    return getColumnsNamesFromStatement(selectAllFromTableStatement(tableName));
  }
private:
  /// @brief path to the database to connect to
  const std::filesystem::path m_db_path{""};

  /// @brief unique pointer that owns the handle to the sqlite3 database
  std::unique_ptr<sqlite3, Sqlite3Closer> m_db{nullptr};

  /// @brief private method to prepare select all from statement on a table
  ///        given its name, and returns its statement pointer wrapped in a
  ///        unique pointer
  /// @param tableName the name of the table to prepare the statement for
  /// @return unique pointer to the statement prepared
  auto selectAllFromTableStatement(std::string const &tableName) const noexcept
      -> std::unique_ptr<sqlite3_stmt, Sqlite3StmtCloser> {
    const auto initializeStatementPtr{[&db = m_db, &tableName]() {
      if (db == nullptr) {
        return static_cast<sqlite3_stmt *>(nullptr);
      }

      sqlite3_stmt *stmtPtr{nullptr};
      constexpr auto useStrLenInternally{-1};
      constexpr auto pzTailPtr{nullptr};

      sqlite3_prepare_v2(db.get(),
                         std::string{"SELECT * FROM " + tableName}.c_str(),
                         useStrLenInternally, &stmtPtr, pzTailPtr);

      return stmtPtr;
    }};

    std::unique_ptr<sqlite3_stmt, Sqlite3StmtCloser> stmt{
        initializeStatementPtr()};

    return stmt;
  }

  /// @brief a private method to read and return each column name in a given
  ///        prepared statement
  /// @param stmt unique pointer to prepared statement
  /// @return vector of names to each column from the prepared statement
  auto getColumnsNamesFromStatement(
      std::unique_ptr<sqlite3_stmt, Sqlite3StmtCloser> const &stmt)
      const noexcept -> std::vector<std::string> {
    if (stmt == nullptr) {
      return {};
    }

    std::size_t const noOfColumns{
        static_cast<std::size_t>(sqlite3_column_count(stmt.get()))};

    std::vector<std::string> columnsNames;
    columnsNames.reserve(noOfColumns);

    for (auto i{0U}; i < noOfColumns; ++i) {
      columnsNames.emplace_back(
          sqlite3_column_name(stmt.get(), static_cast<int>(i)));
    }

    return columnsNames;
  }
};

} // namespace sql_with_cpp
