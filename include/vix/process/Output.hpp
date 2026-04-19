/**
 *
 *  @file Output.hpp
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
#ifndef VIX_PROCESS_OUTPUT_HPP
#define VIX_PROCESS_OUTPUT_HPP

#include <vix/process/Command.hpp>
#include <vix/process/ProcessResult.hpp>

namespace vix::process
{
  /**
   * @brief Run a command to completion and capture its output.
   *
   * This is a high-level convenience API.
   *
   * Expected behavior:
   * - spawns the process
   * - captures stdout and stderr when configured
   * - waits for process completion
   * - returns exit code and captured text
   *
   * Typical usage:
   * - short-lived commands
   * - CLI probing
   * - tooling integration
   *
   * @param command Process launch description.
   * @return ProcessOutputResult containing exit code and captured output.
   */
  [[nodiscard]] ProcessOutputResult output(const Command &command);

} // namespace vix::process

#endif // VIX_PROCESS_OUTPUT_HPP
