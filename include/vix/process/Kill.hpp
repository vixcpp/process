/**
 *
 *  @file Kill.hpp
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
#ifndef VIX_PROCESS_KILL_HPP
#define VIX_PROCESS_KILL_HPP

#include <vix/error/Error.hpp>
#include <vix/process/Child.hpp>

namespace vix::process
{
  /**
   * @brief Forcefully stop a running child process.
   *
   * This is the strongest termination mechanism exposed by the module.
   *
   * Expected behavior:
   * - forcibly terminates the child if possible
   * - returns success if termination request was accepted
   * - returns an error if the process is invalid or termination fails
   *
   * Typical mapping:
   * - POSIX: SIGKILL
   * - Windows: TerminateProcess
   *
   * @param child Child process handle.
   * @return Structured error. Success is represented by a default-constructed error.
   */
  [[nodiscard]] vix::error::Error kill(const Child &child);

} // namespace vix::process

#endif // VIX_PROCESS_KILL_HPP
