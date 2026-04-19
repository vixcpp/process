/**
 *
 *  @file WindowsProcess.hpp
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
#ifndef VIX_PROCESS_PLATFORM_WINDOWSPROCESS_HPP
#define VIX_PROCESS_PLATFORM_WINDOWSPROCESS_HPP

#ifdef _WIN32

#define NOMINMAX
#include <windows.h>

#include <vix/error/Error.hpp>
#include <vix/process/ProcessResult.hpp>

namespace vix::process
{
  class Child;
  class Command;
}

namespace vix::process::platform
{

  /**
   * @struct WindowsProcess
   * @brief Internal Windows backend state for a spawned process.
   *
   * This structure is private to the process backend implementation.
   * It is intended to be stored opaquely inside Child.
   */
  struct WindowsProcess
  {
    /**
     * @brief Native process handle.
     */
    HANDLE process_handle{nullptr};

    /**
     * @brief Native primary thread handle.
     */
    HANDLE thread_handle{nullptr};

    /**
     * @brief Native process id.
     */
    DWORD pid{0};

    /**
     * @brief Read handle of captured stdout pipe.
     *
     * nullptr means no pipe is attached.
     */
    HANDLE stdout_read_handle{nullptr};

    /**
     * @brief Read handle of captured stderr pipe.
     *
     * nullptr means no pipe is attached.
     */
    HANDLE stderr_read_handle{nullptr};

    /**
     * @brief Write handle of stdin pipe.
     *
     * nullptr means no pipe is attached.
     */
    HANDLE stdin_write_handle{nullptr};

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
   * @brief Spawn a Windows child process.
   */
  [[nodiscard]] SpawnResult spawn_windows(const Command &command);

  /**
   * @brief Query whether a Windows child process is still running.
   */
  [[nodiscard]] ProcessRunningResult status_windows(const Child &child);

  /**
   * @brief Wait for a Windows child process and return its exit code.
   */
  [[nodiscard]] ProcessExitCodeResult wait_windows(const Child &child);

  /**
   * @brief Run a Windows command to completion and capture output.
   */
  [[nodiscard]] ProcessOutputResult output_windows(const Command &command);

  /**
   * @brief Forcefully kill a Windows child process.
   */
  [[nodiscard]] vix::error::Error kill_windows(const Child &child);

  /**
   * @brief Gracefully terminate a Windows child process.
   */
  [[nodiscard]] vix::error::Error terminate_windows(const Child &child);

} // namespace vix::process::platform

#endif // _WIN32
#endif // VIX_PROCESS_PLATFORM_WINDOWSPROCESS_HPP
