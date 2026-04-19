/**
 *
 *  @file Status.hpp
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
#ifndef VIX_PROCESS_STATUS_HPP
#define VIX_PROCESS_STATUS_HPP

#include <vix/process/Child.hpp>
#include <vix/process/ProcessResult.hpp>

namespace vix::process
{
  /**
   * @brief Return whether the child process is still running.
   *
   * This function checks the current process state without performing
   * a full blocking wait.
   *
   * Expected behavior:
   * - returns true if the process is alive
   * - returns false if the process already exited
   * - returns an error if the status cannot be queried
   *
   * @param child Child process handle.
   * @return ProcessRunningResult containing the running state.
   */
  [[nodiscard]] ProcessRunningResult status(const Child &child);

} // namespace vix::process

#endif // VIX_PROCESS_STATUS_HPP
