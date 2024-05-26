#pragma once

/// @brief namespace for SQL with c++
namespace sql_with_cpp {

/// @brief interface for classes supporting CRUD operations
class ICruddable {
public:
  /// @brief virtual destructor for polymorphic behavior of classes implementing
  ///        this interface
  virtual ~ICruddable() noexcept = default;
};

} // namespace sql_with_cpp
