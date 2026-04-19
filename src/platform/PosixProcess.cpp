/**
 *
 *  @file PosixProcess.cpp
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

#ifndef _WIN32

#include <memory>
#include <string>

#include <vix/process/Child.hpp>
#include <vix/process/Command.hpp>
#include <vix/process/ProcessError.hpp>
#include <vix/process/ProcessResult.hpp>

#include "PosixProcess.hpp"

namespace vix::process::platform
{
  /**
   * @brief Spawn a POSIX child process.
   *
   * This backend function is intentionally kept internal to the module.
   */
  [[nodiscard]] SpawnResult spawn_posix(const Command &command)
  {
    if (!command.valid())
    {
      return make_process_error(
          ProcessErrorCode::EmptyProgram,
          "process program cannot be empty");
    }

    return make_process_error(
        ProcessErrorCode::UnsupportedOperation,
        "POSIX spawn backend is not implemented yet");
  }

  /**
   * @brief Query whether a POSIX child process is still running.
   */
  [[nodiscard]] ProcessRunningResult status_posix(const Child &child)
  {
    if (!child.valid())
    {
      return make_process_error(
          ProcessErrorCode::InvalidArgument,
          "child process handle is invalid");
    }

    return make_process_error(
        ProcessErrorCode::UnsupportedOperation,
        "POSIX status backend is not implemented yet");
  }

  /**
   * @brief Wait for a POSIX child process and return its exit code.
   */
  [[nodiscard]] ProcessExitCodeResult wait_posix(const Child &child)
  {
    if (!child.valid())
    {
      return make_process_error(
          ProcessErrorCode::InvalidArgument,
          "child process handle is invalid");
    }

    return make_process_error(
        ProcessErrorCode::UnsupportedOperation,
        "POSIX wait backend is not implemented yet");
  }

  /**
   * @brief Forcefully kill a POSIX child process.
   */
  [[nodiscard]] vix::error::Error kill_posix(const Child &child)
  {
    if (!child.valid())
    {
      return make_process_error(
          ProcessErrorCode::InvalidArgument,
          "child process handle is invalid");
    }

    return make_process_error(
        ProcessErrorCode::UnsupportedOperation,
        "POSIX kill backend is not implemented yet");
  }

  /**
   * @brief Gracefully terminate a POSIX child process.
   */
  [[nodiscard]] vix::error::Error terminate_posix(const Child &child)
  {
    if (!child.valid())
    {
      return make_process_error(
          ProcessErrorCode::InvalidArgument,
          "child process handle is invalid");
    }

    return make_process_error(
        ProcessErrorCode::UnsupportedOperation,
        "POSIX terminate backend is not implemented yet");
  }

  /**
   * @brief Run a POSIX command to completion and capture output.
   */
  [[nodiscard]] ProcessOutputResult output_posix(const Command &command)
  {
    if (!command.valid())
    {
      return make_process_error(
          ProcessErrorCode::EmptyProgram,
          "process program cannot be empty");
    }

    return make_process_error(
        ProcessErrorCode::UnsupportedOperation,
        "POSIX output backend is not implemented yet");
  }

} // namespace vix::process::platform

#endif // !_WIN32
