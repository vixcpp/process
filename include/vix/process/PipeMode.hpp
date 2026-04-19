/**
 *
 *  @file PipeMode.hpp
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
#ifndef VIX_PROCESS_PIPEMODE_HPP
#define VIX_PROCESS_PIPEMODE_HPP

namespace vix::process
{
  /**
   * @enum PipeMode
   * @brief Defines how a process stream is handled.
   *
   * Used for stdin, stdout, and stderr.
   *
   * Design goals:
   * - explicit behavior
   * - no hidden defaults
   * - portable across POSIX and Windows
   */
  enum class PipeMode
  {
    /**
     * @brief Inherit from parent process.
     */
    Inherit,

    /**
     * @brief Create a pipe to capture or write.
     */
    Pipe,

    /**
     * @brief Redirect to null device.
     */
    Null
  };

} // namespace vix::process

#endif // VIX_PROCESS_PIPEMODE_HPP
