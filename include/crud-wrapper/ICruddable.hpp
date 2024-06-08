#pragma once

/// @brief namespace for SQL with c++
namespace sql_with_cpp {

/// @brief interface for classes supporting CRUD operations
class ICruddable {
public:
  /// @brief virtual destructor for polymorphic behavior of classes implementing
  ///        this interface
  virtual ~ICruddable() noexcept = default;

  /// @brief a method to return the columns' names of a given table as vector of
  ///        strings
  /// @param tableName the name of the table to peek its columns' names
  /// @return a vector of strings that represent name of each column
  [[nodiscard]] virtual std::vector<std::string>
  peekColumnsNames(std::string const &tableName) const = 0;

  /// @brief a method to execute multiple statements that don't have a SELECT
  ///        statement as one of them (e.g CREATE, DROP, .. etc)
  /// @param statements the SQL statements to be executed
  /// @return true if statements were executed successfully, false otherwise
  /// @note callback is not used here as it could get very complex quickly,
  ///       there are more straight forward alternatives instead
  virtual bool executeStatements(std::string const &statements) = 0;

  /// @brief a method to read all the rows in the given table, where the first
  ///        row represents the columns names
  /// @param tableName the name of the table to read all of its rows
  /// @return a vector of vector of strings, where each outer vector represents
  ///         a row, and the internal vector represents the data in the
  ///         respective row
  /// @note as mentioned above, the first row represents the column names, so
  ///       the actual data starts from index: 1
  [[nodiscard]] virtual std::vector<std::vector<std::string>>
  getRows(std::string const &tableName) const = 0;
};

} // namespace sql_with_cpp
