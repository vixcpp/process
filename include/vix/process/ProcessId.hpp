/**
 *
 *  @file ProcessId.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2026, Gaspard Kirira.
 *  All rights reserved.
 *  https://github.com/vixcpp/vix
 *
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 *
 */
#ifndef VIX_PROCESS_PROCESSID_HPP
#define VIX_PROCESS_PROCESSID_HPP

#include <cstdint>

namespace vix::process
{
  /**
   * @brief Platform-independent process identifier.
   *
   * On POSIX:
   *   - maps to pid_t
   *
   * On Windows:
   *   - maps to DWORD / HANDLE (abstracted)
   *
   * Design goals:
   * - simple and lightweight
   * - stable type across platforms
   */
  using ProcessId = std::uint64_t;

} // namespace vix::process

#endif // VIX_PROCESS_PROCESSID_HPP
