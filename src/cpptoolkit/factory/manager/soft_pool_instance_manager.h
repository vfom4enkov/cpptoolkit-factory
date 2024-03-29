/*
 * BSD 2-Clause License
 *
 * Copyright (c) 2021, Vladimir Fomchenkov
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CPP_TOOL_KIT_FACTORY_POOL_INSTANCE_MANAGER_H_
#define CPP_TOOL_KIT_FACTORY_POOL_INSTANCE_MANAGER_H_

#include <mutex>
#include <queue>
#include <unordered_map>

#include "base_instance_manager.h"
#include "context/pool_context.h"

namespace cpptoolkit {
namespace factory {
namespace engine {

/// If the pool is emty this manager creates new one, if the pool is full
/// the object will be destroyed
/// @tparam T type of managed object
template <typename T>
class SoftPoolInstanceManager : public BaseInstanceManager<T>,
                                public AbstractPoolInstancePutback {
 public:
  /// @brief Create instance manager
  /// @param class_name_key [in] unique key for current manager
  /// @param create [in] function for create instance of managed object
  /// @param core [in] pointer to the core_ with registered objects
  /// @param pool_size [in] size of pool object
  SoftPoolInstanceManager(
      std::string class_name_key,
      std::function<T*(cpptoolkit::factory::Resolver&)>&& create,
      cpptoolkit::factory::Core* core, uint32_t pool_size) noexcept
      : BaseInstanceManager<T>(class_name_key, std::move(create), core),
        size_(pool_size){};

  virtual ~SoftPoolInstanceManager() noexcept = default;

  PtrHolder<BaseContext<T>> Get() noexcept override;
  void Callback(uintptr_t key) noexcept override;

 private:
  uint32_t size_;
  std::queue<uintptr_t> queue_;
  std::unordered_map<uintptr_t, PtrHolder<Context<T>>> index_;

  // thread section
  std::mutex mutex_;
};

template <typename T>
inline PtrHolder<BaseContext<T>> SoftPoolInstanceManager<T>::Get() noexcept {
  std::unique_lock<std::mutex> locker(mutex_);

  if (!queue_.empty()) {  // Get istance from pool
    uintptr_t key = queue_.front();
    const auto it = index_.find(key);
    PtrHolder<PoolContext<T>> ctx =
        MakePtrHolder<PoolContext<T>>(this, it->second.Get(), key);
    queue_.pop();
    return ctx;
  }

  // Create new instance
  PtrHolder<Context<T>> context = MakePtrHolder<Context<T>>();
  BaseInstanceManager<T>::Create(context.Get());

  if (!context->IsValid()) {
    return context;
  }

  Context<T>* context_ptr = context.Get();
  uintptr_t key = reinterpret_cast<uintptr_t>(context_ptr);
  PtrHolder<PoolContext<T>> ctx =
      MakePtrHolder<PoolContext<T>>(this, context_ptr, key);
  index_.emplace(key, std::move(context));
  return ctx;
}

template <typename T>
inline void SoftPoolInstanceManager<T>::Callback(uintptr_t key) noexcept {
  std::unique_lock<std::mutex> locker(mutex_);
  if (queue_.size() < size_) {
    queue_.push(key);
    return;
  }

  index_.erase(key);
}

}  // namespace engine
}  // namespace factory
}  // namespace cpptoolkit

#endif  // CPP_TOOL_KIT_FACTORY_POOL_INSTANCE_MANAGER_H_
