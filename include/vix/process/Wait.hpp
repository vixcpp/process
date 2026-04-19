/**
 *
 *  @file Wait.hpp
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
#ifndef VIX_PROCESS_WAIT_HPP
#define VIX_PROCESS_WAIT_HPP

#include <vix/process/Child.hpp>
#include <vix/process/ProcessResult.hpp>

namespace vix::process
{
  /**
   * @brief Wait until the child process exits and return its exit code.
   *
   * This is a blocking operation.
   *
   * Expected behavior:
   * - waits for process termination
   * - extracts the final exit status
   * - returns an error if waiting fails
   *
   * @param child Child process handle.
   * @return ProcessExitCodeResult containing the exit code on success.
   */
  [[nodiscard]] ProcessExitCodeResult wait(const Child &child);

} // namespace vix::process

#endif // VIX_PROCESS_WAIT_HPP
