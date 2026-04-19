/**
 *
 *  @file ProcessError.hpp
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
#ifndef VIX_PROCESS_PROCESSERROR_HPP
#define VIX_PROCESS_PROCESSERROR_HPP

#include <string>
#include <utility>

#include <vix/error/Error.hpp>
#include <vix/error/ErrorCategory.hpp>
#include <vix/error/ErrorCode.hpp>

namespace vix::process
{
  /**
   * @enum ProcessErrorCode
   * @brief Process-specific semantic error codes.
   *
   * These codes describe common failures related to:
   * - process creation
   * - process lookup
   * - waiting and termination
   * - stdio redirection and capture
   * - invalid process configuration
   *
   * They complement the generic Vix error system.
   */
  enum class ProcessErrorCode
  {
    None = 0,
    EmptyProgram,
    InvalidProgram,
    ProgramNotFound,
    InvalidArgument,
    InvalidWorkingDirectory,
    SpawnFailed,
    WaitFailed,
    StatusFailed,
    KillFailed,
    TerminateFailed,
    PipeCreationFailed,
    CaptureFailed,
    NotRunning,
    AlreadyExited,
    PermissionDenied,
    Timeout,
    UnsupportedOperation
  };

  /**
   * @brief Return the default process error category.
   */
  [[nodiscard]] inline constexpr vix::error::ErrorCategory process_error_category() noexcept
  {
    return vix::error::ErrorCategory("process");
  }

  /**
   * @brief Convert a ProcessErrorCode to a generic Vix ErrorCode.
   */
  [[nodiscard]] inline constexpr vix::error::ErrorCode to_error_code(ProcessErrorCode code) noexcept
  {
    using vix::error::ErrorCode;

    switch (code)
    {
    case ProcessErrorCode::None:
      return ErrorCode::Ok;

    case ProcessErrorCode::EmptyProgram:
    case ProcessErrorCode::InvalidProgram:
    case ProcessErrorCode::InvalidArgument:
    case ProcessErrorCode::InvalidWorkingDirectory:
      return ErrorCode::InvalidArgument;

    case ProcessErrorCode::ProgramNotFound:
      return ErrorCode::NotFound;

    case ProcessErrorCode::PermissionDenied:
      return ErrorCode::PermissionDenied;

    case ProcessErrorCode::Timeout:
      return ErrorCode::Timeout;

    case ProcessErrorCode::UnsupportedOperation:
      return ErrorCode::NotSupported;

    case ProcessErrorCode::NotRunning:
    case ProcessErrorCode::AlreadyExited:
      return ErrorCode::InvalidState;

    case ProcessErrorCode::SpawnFailed:
    case ProcessErrorCode::WaitFailed:
    case ProcessErrorCode::StatusFailed:
    case ProcessErrorCode::KillFailed:
    case ProcessErrorCode::TerminateFailed:
    case ProcessErrorCode::PipeCreationFailed:
    case ProcessErrorCode::CaptureFailed:
      return ErrorCode::ExternalError;
    }

    return ErrorCode::Unknown;
  }

  /**
   * @brief Convert a ProcessErrorCode to a human-readable name.
   */
  [[nodiscard]] inline const char *to_string(ProcessErrorCode code) noexcept
  {
    switch (code)
    {
    case ProcessErrorCode::None:
      return "none";
    case ProcessErrorCode::EmptyProgram:
      return "empty_program";
    case ProcessErrorCode::InvalidProgram:
      return "invalid_program";
    case ProcessErrorCode::ProgramNotFound:
      return "program_not_found";
    case ProcessErrorCode::InvalidArgument:
      return "invalid_argument";
    case ProcessErrorCode::InvalidWorkingDirectory:
      return "invalid_working_directory";
    case ProcessErrorCode::SpawnFailed:
      return "spawn_failed";
    case ProcessErrorCode::WaitFailed:
      return "wait_failed";
    case ProcessErrorCode::StatusFailed:
      return "status_failed";
    case ProcessErrorCode::KillFailed:
      return "kill_failed";
    case ProcessErrorCode::TerminateFailed:
      return "terminate_failed";
    case ProcessErrorCode::PipeCreationFailed:
      return "pipe_creation_failed";
    case ProcessErrorCode::CaptureFailed:
      return "capture_failed";
    case ProcessErrorCode::NotRunning:
      return "not_running";
    case ProcessErrorCode::AlreadyExited:
      return "already_exited";
    case ProcessErrorCode::PermissionDenied:
      return "permission_denied";
    case ProcessErrorCode::Timeout:
      return "timeout";
    case ProcessErrorCode::UnsupportedOperation:
      return "unsupported_operation";
    }

    return "unknown";
  }

  /**
   * @brief Build a structured Vix error from a ProcessErrorCode.
   *
   * @param code Process-specific error code.
   * @param message Human-readable message.
   */
  [[nodiscard]] inline vix::error::Error make_process_error(ProcessErrorCode code, std::string message)
  {
    return vix::error::Error(
        to_error_code(code),
        process_error_category(),
        std::move(message));
  }

} // namespace vix::process

#endif // VIX_PROCESS_PROCESSERROR_HPP
