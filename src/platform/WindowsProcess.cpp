/**
 *
 *  @file WindowsProcess.cpp
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

#ifdef _WIN32

#include <memory>
#include <string>

#include <vix/process/Child.hpp>
#include <vix/process/Command.hpp>
#include <vix/process/ProcessError.hpp>
#include <vix/process/ProcessResult.hpp>

#include "WindowsProcess.hpp"

namespace vix::process::platform
{
  /**
   * @brief Spawn a Windows child process.
   *
   * This backend function is intentionally kept internal to the module.
   */
  [[nodiscard]] SpawnResult spawn_windows(const Command &command)
  {
    if (!command.valid())
    {
      return make_process_error(
          ProcessErrorCode::EmptyProgram,
          "process program cannot be empty");
    }

    return make_process_error(
        ProcessErrorCode::UnsupportedOperation,
        "Windows spawn backend is not implemented yet");
  }

  /**
   * @brief Query whether a Windows child process is still running.
   */
  [[nodiscard]] ProcessRunningResult status_windows(const Child &child)
  {
    if (!child.valid())
    {
      return make_process_error(
          ProcessErrorCode::InvalidArgument,
          "child process handle is invalid");
    }

    return make_process_error(
        ProcessErrorCode::UnsupportedOperation,
        "Windows status backend is not implemented yet");
  }

  /**
   * @brief Wait for a Windows child process and return its exit code.
   */
  [[nodiscard]] ProcessExitCodeResult wait_windows(const Child &child)
  {
    if (!child.valid())
    {
      return make_process_error(
          ProcessErrorCode::InvalidArgument,
          "child process handle is invalid");
    }

    return make_process_error(
        ProcessErrorCode::UnsupportedOperation,
        "Windows wait backend is not implemented yet");
  }

  /**
   * @brief Forcefully kill a Windows child process.
   */
  [[nodiscard]] vix::error::Error kill_windows(const Child &child)
  {
    if (!child.valid())
    {
      return make_process_error(
          ProcessErrorCode::InvalidArgument,
          "child process handle is invalid");
    }

    return make_process_error(
        ProcessErrorCode::UnsupportedOperation,
        "Windows kill backend is not implemented yet");
  }

  /**
   * @brief Gracefully terminate a Windows child process.
   */
  [[nodiscard]] vix::error::Error terminate_windows(const Child &child)
  {
    if (!child.valid())
    {
      return make_process_error(
          ProcessErrorCode::InvalidArgument,
          "child process handle is invalid");
    }

    return make_process_error(
        ProcessErrorCode::UnsupportedOperation,
        "Windows terminate backend is not implemented yet");
  }

  /**
   * @brief Run a Windows command to completion and capture output.
   */
  [[nodiscard]] ProcessOutputResult output_windows(const Command &command)
  {
    if (!command.valid())
    {
      return make_process_error(
          ProcessErrorCode::EmptyProgram,
          "process program cannot be empty");
    }

    return make_process_error(
        ProcessErrorCode::UnsupportedOperation,
        "Windows output backend is not implemented yet");
  }

} // namespace vix::process::platform

#endif // _WIN32
