#pragma once

#include <concepts>
#include <exception>
#include <filesystem>
#include <memory>
#include <sqlite3.h>
#include <string>
#include <vector>

#include "ICruddable.hpp"

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

  /// @brief a type alias for sqlite3 statement unique pointer type used
  using Stmt_Ptr_type = std::unique_ptr<sqlite3_stmt, Sqlite3StmtCloser>;
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
    const int rCode{sqlite3_open(m_db_path.string().c_str(), &dbPtr)};
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

  /// @brief a method to read all the rows in the given table, where the first
  ///        row represents the columns names
  /// @param tableName the name of the table to read all of its rows
  /// @return a vector of vector of strings, where each outer vector represents
  ///         a row, and the internal vector represents the data in the
  ///         respective row
  /// @note as mentioned above, the first row represents the column names, so
  ///       the actual data starts from index: 1
  auto getRows(std::string const &tableName) const
      -> std::vector<std::vector<std::string>> {
    Stmt_Ptr_type stmt{selectAllFromTableStatement(tableName)};

    std::vector<std::vector<std::string>> rows;

    // where first row shall be the columns names
    rows.emplace_back(getColumnsNamesFromStatement(stmt));

    // read and emplace remaining rows
    const auto noOfColumns{rows[0].size()};
    while (sqlite3_step(stmt.get()) == SQLITE_ROW) {
      std::vector<std::string> rowElements;
      rowElements.reserve(noOfColumns);

      for (auto i{0U}; i < noOfColumns; ++i) {
        rowElements.emplace_back(
            reinterpret_cast<const char *>( // had to because
                                            // sqlite3_column_text API returns
                                            // const unsigned char* instead
                sqlite3_column_text(stmt.get(), static_cast<int>(i))));
      }

      rows.emplace_back(std::move(rowElements));
    }

    return rows;
  }

  /// @brief a method to execute multiple statements that don't have a SELECT
  ///        statement as one of them (e.g CREATE, DROP, .. etc)
  /// @param statements the SQL statements to be executed
  /// @return true if statements were executed successfully, false otherwise
  /// @note callback is not used here as it could get very complex quickly,
  ///       there are more straight forward alternatives instead
  auto executeStatements(std::string const &statements) noexcept -> bool {
    constexpr auto callback{nullptr};
    constexpr auto callbackFirstArg{nullptr};
    constexpr auto errMsg{nullptr};

    const int rcode{sqlite3_exec(m_db.get(), statements.c_str(), callback,
                                 callbackFirstArg, errMsg)};

    return {rcode == SQLITE_OK};
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
      -> Stmt_Ptr_type {
    return initializeStatement(
        std::string{"SELECT * FROM " + tableName}.c_str(), m_db);
  }

  /// @brief a private static class method for preparing statements
  /// @param statement the statement to be prepared
  /// @param db the database for which the statement need to be prepared
  /// @return a unique pointer to the prepared statement
  static auto initializeStatement(std::string const &statement,
                                  std::unique_ptr<sqlite3, Sqlite3Closer> const
                                      &db) noexcept -> Stmt_Ptr_type {
    if (db == nullptr) {
      return static_cast<Stmt_Ptr_type>(nullptr);
    }

    sqlite3_stmt *stmtPtr{nullptr};
    constexpr auto useStrLenInternally{-1};
    constexpr auto pzTailPtr{nullptr};

    sqlite3_prepare_v2(db.get(), statement.c_str(), useStrLenInternally,
                       &stmtPtr, pzTailPtr);

    return Stmt_Ptr_type{stmtPtr};
  }

  /// @brief a private method to read and return each column name in a given
  ///        prepared statement
  /// @param stmt unique pointer to prepared statement
  /// @return vector of names to each column from the prepared statement
  auto getColumnsNamesFromStatement(Stmt_Ptr_type const &stmt) const noexcept
      -> std::vector<std::string> {
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
