/**
 *
 *  @file ProcessOptions.hpp
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
#ifndef VIX_PROCESS_PROCESSOPTIONS_HPP
#define VIX_PROCESS_PROCESSOPTIONS_HPP

#include <string>

#include <vix/process/PipeMode.hpp>

namespace vix::process
{
  /**
   * @struct ProcessOptions
   * @brief Configuration for process execution.
   *
   * This structure controls:
   * - stdio behavior
   * - environment inheritance
   * - execution context
   *
   * Design goals:
   * - explicit configuration
   * - minimal but extensible
   * - consistent with Vix modules (env, path)
   */
  struct ProcessOptions
  {
    /**
     * @brief Standard input behavior.
     */
    PipeMode stdin_mode{PipeMode::Inherit};

    /**
     * @brief Standard output behavior.
     */
    PipeMode stdout_mode{PipeMode::Inherit};

    /**
     * @brief Standard error behavior.
     */
    PipeMode stderr_mode{PipeMode::Inherit};

    /**
     * @brief Whether to search the executable in PATH.
     */
    bool search_in_path{true};

    /**
     * @brief Whether to detach the process.
     *
     * Detached process:
     * - runs independently
     * - parent does not wait for it
     */
    bool detach{false};

    /**
     * @brief Whether to inherit the parent environment.
     */
    bool inherit_environment{true};

    /**
     * @brief Working directory for the process.
     *
     * Empty = current working directory.
     */
    std::string working_directory{};
  };

} // namespace vix::process

#endif // VIX_PROCESS_PROCESSOPTIONS_HPP
