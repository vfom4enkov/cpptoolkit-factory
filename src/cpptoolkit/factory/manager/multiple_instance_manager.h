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

#ifndef CPP_TOOL_KIT_FACTORY_MULTIPLE_INSTANCE_MANAGER_H_
#define CPP_TOOL_KIT_FACTORY_MULTIPLE_INSTANCE_MANAGER_H_

#include "base_instance_manager.h"

namespace cpptoolkit {
namespace factory {
namespace engine {

/// @brief Instance manager for multiple objects
/// @tparam T type of managed object
template <typename T>
class MultipleInstanceManager : public BaseInstanceManager<T> {
 public:
  /// @brief Create MultipleInstanceManager
  /// @param class_name_key [in] unique key for current manager
  /// @param create [in] function for create instance of managed object
  /// @param core [in] pointer to the core_ with registered objects
  MultipleInstanceManager(
      std::string class_name_key,
      std::function<T*(cpptoolkit::factory::Resolver&)>&& create,
      cpptoolkit::factory::Core* core) noexcept
      : BaseInstanceManager<T>(class_name_key, std::move(create), core){};

  virtual ~MultipleInstanceManager() noexcept {};

  PtrHolder<BaseContext<T>> Get() noexcept override;
};

// Implementation

template <typename T>
inline PtrHolder<BaseContext<T>> MultipleInstanceManager<T>::Get() noexcept {
  PtrHolder<Context<T>> context = MakePtrHolder<Context<T>>();
  BaseInstanceManager<T>::Create(context.Get());
  return context;
}

}  // namespace engine
}  // namespace factory
}  // namespace cpptoolkit

#endif  // CPP_TOOL_KIT_FACTORY_MULTIPLE_INSTANCE_MANAGER_H_
