/**
 *
 *  @file Terminate.hpp
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
#ifndef VIX_PROCESS_TERMINATE_HPP
#define VIX_PROCESS_TERMINATE_HPP

#include <vix/error/Error.hpp>
#include <vix/process/Child.hpp>

namespace vix::process
{
  /**
   * @brief Request graceful termination of a running child process.
   *
   * This is a softer termination mechanism than kill().
   *
   * Expected behavior:
   * - asks the child process to terminate cleanly
   * - returns success if the request was delivered
   * - returns an error if the process is invalid or the request fails
   *
   * Typical mapping:
   * - POSIX: SIGTERM
   * - Windows: best-effort graceful termination strategy
   *
   * @param child Child process handle.
   * @return Structured error. Success is represented by a default-constructed error.
   */
  [[nodiscard]] vix::error::Error terminate(const Child &child);

} // namespace vix::process

#endif // VIX_PROCESS_TERMINATE_HPP
