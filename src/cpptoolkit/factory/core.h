#ifndef CPP_TOOL_KIT_FACTORY_CORE_H_
#define CPP_TOOL_KIT_FACTORY_CORE_H_

#include <functional>
#include <string>
#include <unordered_map>

#include "manager/base_instance_manager.h"
#include "tool/common.h"
#include "tool/deleter.h"

namespace cpptoolkit {
namespace factory {

/// @brief Contains all data about registered classes
class Core {
 public:
  Core() = default;
  virtual ~Core() = default;

  /// @brief Get last error description
  /// @return Last error
  const std::string& LastError() noexcept { return error_; };

  /// @brief Get context with managed object
  /// @tparam T - Type of managed object
  /// @param [in] key - Unique key for a given object type
  /// @return PtrHolder with BaseContext and instance of managed object
  template <typename T>
  std::unique_ptr<BaseContext<T>> Get(
      const std::string& key = DEFAULT_KEY) noexcept;

  /// @brief Get unique_ptr that owns managed object (if object is nullptr check
  /// LastError())
  /// @tparam T - Type of managed object
  /// @param [in] key - Unique key for a given object type
  /// @return unique_ptr with managed object
  template <typename T>
  std::unique_ptr<T, Deleter<T>> GetUnique(
      const std::string& key = DEFAULT_KEY) noexcept;

  /// @brief Get shared_ptr that owns managed object (if object is nullptr check
  /// LastError())
  /// @tparam T - Type of managed object
  /// @param [in] key - Unique key for a given object type
  /// @return shared_ptr with managed object
  template <typename T>
  std::shared_ptr<T> GetShared(const std::string& key = DEFAULT_KEY) noexcept;

 protected:
  std::unordered_map<std::string, std::unique_ptr<AInstanceManager>> index_;
  std::string error_;
};

// Implementation

template <typename T>
inline std::unique_ptr<BaseContext<T>> Core::Get(
    const std::string& key) noexcept {
  std::string type_key = TypeKey<T>(key);
  const auto it = index_.find(type_key);
  if (it == index_.end()) {
    std::string error = "Type: " + type_key + " is not registered";
    std::unique_ptr<ErrorContext<T>> error_context =
        MakeUnique<ErrorContext<T>>(error);
    return error_context;
  }

  BaseInstanceManager<T>* instance_manager =
      reinterpret_cast<BaseInstanceManager<T>*>(it->second.get());
  return instance_manager->Get();
}

template <typename T>
inline std::unique_ptr<T, Deleter<T>> Core::GetUnique(
    const std::string& key) noexcept {
  std::unique_ptr<BaseContext<T>> context = Get<T>(key);
  if (!context->IsValid()) {
    error_ = context->Error();
    return std::unique_ptr<T, Deleter<T>>(nullptr, Deleter<T>(nullptr));
  }

  T* instance = context->GetInstance();
  std::unique_ptr<T, Deleter<T>> uptr(instance, Deleter<T>(std::move(context)));
  return uptr;
}

template <typename T>
inline std::shared_ptr<T> Core::GetShared(const std::string& key) noexcept {
  std::unique_ptr<BaseContext<T>> context = Get<T>(key);
  if (!context->IsValid()) {
    error_ = context->Error();
    return std::shared_ptr<T>();
  }

  T* instance = context->GetInstance();
  std::shared_ptr<T> sptr(
      instance, [ctx = std::move(context)](T* inst) mutable { ctx.reset(); });
  return sptr;
}

}  // namespace factory
}  // namespace cpptoolkit

#endif  // CPP_TOOL_KIT_FACTORY_CORE_H_