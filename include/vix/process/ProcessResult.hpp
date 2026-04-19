/**
 *
 *  @file ProcessResult.hpp
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
#ifndef VIX_PROCESS_PROCESSRESULT_HPP
#define VIX_PROCESS_PROCESSRESULT_HPP

#include <string>

#include <vix/error/Result.hpp>

namespace vix::process
{
  class Child;

  /**
   * @struct ProcessOutput
   * @brief Captured result of a finished process execution.
   *
   * This structure keeps:
   * - the final exit code
   * - captured stdout text
   * - captured stderr text
   *
   * It is mainly used by high-level APIs such as output().
   */
  struct ProcessOutput
  {
    /**
     * @brief Process exit code.
     *
     * A value of 0 usually means success.
     */
    int exit_code{0};

    /**
     * @brief Captured standard output.
     */
    std::string stdout_text{};

    /**
     * @brief Captured standard error.
     */
    std::string stderr_text{};

    /**
     * @brief Return true if the process exited successfully.
     */
    [[nodiscard]] bool success() const noexcept
    {
      return exit_code == 0;
    }
  };

  /**
   * @brief Standard result type for process operations returning an exit code.
   */
  using ProcessExitCodeResult = vix::error::Result<int>;

  /**
   * @brief Standard result type for process operations returning a running state.
   */
  using ProcessRunningResult = vix::error::Result<bool>;

  /**
   * @brief Standard result type for process operations returning captured output.
   */
  using ProcessOutputResult = vix::error::Result<ProcessOutput>;

  /**
   * @brief Standard result type for process spawn operations.
   */
  using SpawnResult = vix::error::Result<Child>;

} // namespace vix::process

#endif // VIX_PROCESS_PROCESSRESULT_HPP
