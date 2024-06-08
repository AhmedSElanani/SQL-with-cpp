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

  /// @brief a type alias for sqlite3 database unique pointer type used
  using Db_Ptr_type = std::unique_ptr<sqlite3, Sqlite3Closer>;

  /// @brief a type alias for sqlite3 statement unique pointer type used
  using Stmt_Ptr_type = std::unique_ptr<sqlite3_stmt, Sqlite3StmtCloser>;

  /// @brief a class that represents prepared statements in SQLite3 for
  ///        rebinding and reusability
  class PreparedStatement {
  public:
    /// @brief deleted default constructor for allowing only construction with
    ///        the specified parameters
    /// @note this is the default behavior since a parametrized constructor is
    ///       already defined, but this is just for explicitly, and
    ///       defensiveness
    PreparedStatement() = delete;

    /// @brief parametrized constructor to PreparedStatement class that takes
    ///        the SQL statement and reference to a cruddableObject object to
    ///        access its underlying database
    /// @param statement the statement to be prepared
    /// @param cruddableObject the object that wraps the database for which the
    ///                        statement is prepared
    PreparedStatement(std::string const &statement,
                      CrudWrapper const &crudWrapperObj) noexcept
        : m_stmt{initializeStatement(statement, crudWrapperObj.m_db)} {}

    /// @brief method to bind text to placeholder parameters according to sqlite
    ///        syntax
    /// @param text text to bind
    /// @param position position of placeholder to bind that text to
    /// @return true if binding text was successful, false otherwise
    auto bindText(std::string const &text,
                  std::size_t position) noexcept -> bool {
      if (m_stmt == nullptr) {
        return false;
      }

      // reset is necessary before calling bind() in case of rebinding with
      // new parameter after bind was called before to the same statement
      sqlite3_reset(m_stmt.get());

      constexpr auto useStrLenInternally{-1};
      const int rCode{sqlite3_bind_text(
          m_stmt.get(), static_cast<int>(position), text.c_str(),
          useStrLenInternally,
          // Very important note for below parameter:
          // I faced errors with ASAN when using SQLITE_STATIC
          // instead of SQLITE_TRANSIENT
          // SQLITE_TRANSIENT tells SQLite to copy the string
          // while SQLITE_STATIC means you guarantee that the string will be
          // valid until after the query is executed, which might not be the
          // case when using std::string::c_str()
          // So, it might be better to play it safe with this parameter
          SQLITE_TRANSIENT)};

      return {rCode == SQLITE_OK};
    }

    /// @brief method to return immutable reference to the underlying statement
    ///        pointer, which could be useful for compatibility with other APIs
    ///        implemented
    /// @return const reference to the underlying unique pointer to statement
    Stmt_Ptr_type const &get() const { return m_stmt; }

  private:
    /// @brief unique pointer to the underlying sqlite3 statement object
    Stmt_Ptr_type m_stmt{nullptr};
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
    const int rCode{sqlite3_open(m_db_path.string().c_str(), &dbPtr)};
    if (rCode != SQLITE_OK) {
      throw std::runtime_error(
          std::string{"Failed to open database, sqlite3 error: "} +
          sqlite3_errstr(rCode));
    }

    m_db = Db_Ptr_type{dbPtr};
  }

  /// @brief a class method that returns a prepared statement object based on
  ///        the passed statement parameter for reusability
  /// @param statement the statement to be prepared
  /// @return a prepared statement object based on the passed statement
  auto prepareStatement(std::string const &statement) const noexcept
      -> PreparedStatement {
    return PreparedStatement{statement, *this};
  }

  /// @brief implementation for the interface method
  /// @note for further info, check the interface documentation
  auto peekColumnsNames(std::string const &tableName) const
      -> std::vector<std::string> override {
    return getColumnsNamesFromStatement(
        buildSelectAllFromTableStatement(tableName));
  }

  /// @brief implementation for the interface method
  /// @note for further info, check the interface documentation
  auto getRows(std::string const &tableName) const
      -> std::vector<std::vector<std::string>> override {
    return getRowsFromStatement(buildSelectAllFromTableStatement(tableName));
  }

  /// @brief an overload to getRows method that takes prepared statement
  /// @param statement prepared statement object
  /// @return a vector of vector of strings, where each outer vector represents
  ///         a row, and the internal vector represents the data in the
  ///         respective row
  auto getRows(PreparedStatement const &statement) const
      -> std::vector<std::vector<std::string>> {
    return getRowsFromStatement(statement.get());
  }

  /// @brief implementation for the interface method
  /// @note for further info, check the interface documentation
  auto
  executeStatements(std::string const &statements) noexcept -> bool override {
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
  Db_Ptr_type m_db{nullptr};

  /// @brief private method to build select all from statement on a table
  ///        given its name, and returns its statement pointer wrapped in a
  ///        unique pointer
  /// @param tableName the name of the table to prepare the statement for
  /// @return unique pointer to the statement prepared
  auto buildSelectAllFromTableStatement(
      std::string const &tableName) const noexcept -> Stmt_Ptr_type {
    return initializeStatement(
        std::string{"SELECT * FROM " + tableName}.c_str(), m_db);
  }

  /// @brief a private static class method for preparing statements
  /// @param statement the statement to be prepared
  /// @param db the database for which the statement need to be prepared
  /// @return a unique pointer to the prepared statement
  static auto
  initializeStatement(std::string const &statement,
                      Db_Ptr_type const &db) noexcept -> Stmt_Ptr_type {
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

  /// @brief a private method to return all the rows given the statement passed
  /// @param stmt a unique pointer to sqlite3 prepared statement
  /// @return vector of vector of strings representing the results
  auto getRowsFromStatement(Stmt_Ptr_type const &stmt) const noexcept
      -> std::vector<std::vector<std::string>> {
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
};

} // namespace sql_with_cpp
