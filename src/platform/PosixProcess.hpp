/**
 *
 *  @file PosixProcess.hpp
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
#ifndef VIX_PROCESS_PLATFORM_POSIXPROCESS_HPP
#define VIX_PROCESS_PLATFORM_POSIXPROCESS_HPP

#ifndef _WIN32

#include <sys/types.h>

#include <vix/error/Error.hpp>
#include <vix/process/ProcessResult.hpp>
#include <vix/process/pipeline/Pipeline.hpp>

namespace vix::process
{
  class Child;
  class Command;
}

namespace vix::process::platform
{

  /**
   * @struct PosixProcess
   * @brief Internal POSIX backend state for a spawned process.
   *
   * This structure is private to the process backend implementation.
   * It is intended to be stored opaquely inside Child.
   */
  struct PosixProcess
  {
    /**
     * @brief Native POSIX process id.
     */
    pid_t pid{-1};

    /**
     * @brief Read end of captured stdout pipe.
     *
     * -1 means no pipe is attached.
     */
    int stdout_read_fd{-1};

    /**
     * @brief Read end of captured stderr pipe.
     *
     * -1 means no pipe is attached.
     */
    int stderr_read_fd{-1};

    /**
     * @brief Write end of stdin pipe.
     *
     * -1 means no pipe is attached.
     */
    int stdin_write_fd{-1};

    /**
     * @brief Cached running state.
     */
    bool running{false};

    /**
     * @brief Cached exit state.
     */
    bool exited{false};

    /**
     * @brief Cached exit code when available.
     */
    int exit_code{0};
  };

  /**
   * @brief Spawn a POSIX child process.
   */
  [[nodiscard]] SpawnResult spawn_posix(const Command &command);

  /**
   * @brief Query whether a POSIX child process is still running.
   */
  [[nodiscard]] ProcessRunningResult status_posix(const Child &child);

  /**
   * @brief Wait for a POSIX child process and return its exit code.
   */
  [[nodiscard]] ProcessExitCodeResult wait_posix(const Child &child);

  [[nodiscard]] vix::error::Result<vix::process::pipeline::PipelineChildren> spawn_pipeline_posix(
      const Command &first,
      const Command &second);

  /**
   * @brief Run a POSIX command to completion and capture output.
   */
  [[nodiscard]] ProcessOutputResult output_posix(const Command &command);

  /**
   * @brief Forcefully kill a POSIX child process.
   */
  [[nodiscard]] vix::error::Error kill_posix(const Child &child);

  /**
   * @brief Gracefully terminate a POSIX child process.
   */
  [[nodiscard]] vix::error::Error terminate_posix(const Child &child);

} // namespace vix::process::platform

#endif // !_WIN32
#endif // VIX_PROCESS_PLATFORM_POSIXPROCESS_HPP
