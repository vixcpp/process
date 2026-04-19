/**
 *
 *  @file Pipeline.hpp
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
#ifndef VIX_PROCESS_PIPELINE_PIPELINE_HPP
#define VIX_PROCESS_PIPELINE_PIPELINE_HPP

#include <vix/error/Result.hpp>
#include <vix/process/Child.hpp>
#include <vix/process/Command.hpp>

namespace vix::process::pipeline
{
  /**
   * @brief Child handles returned by a spawned two-stage pipeline.
   *
   * first  -> producer command
   * second -> consumer command
   */
  struct PipelineChildren
  {
    Child first;
    Child second;

    /**
     * @brief Return true if both child handles are valid.
     */
    [[nodiscard]] bool valid() const noexcept
    {
      return first.valid() && second.valid();
    }
  };

  /**
   * @brief Final result of a completed two-stage pipeline.
   */
  struct PipelineResult
  {
    int first_exit_code{0};
    int second_exit_code{0};

    /**
     * @brief Return true if both commands exited successfully.
     */
    [[nodiscard]] bool success() const noexcept
    {
      return first_exit_code == 0 && second_exit_code == 0;
    }
  };

  /**
   * @brief Spawn a two-stage pipeline.
   *
   * Intended semantic:
   * - stdout of the first command is connected to stdin of the second command
   * - both child handles are returned on success
   *
   * Current module contract:
   * - validates both commands
   * - delegates to platform backend when available
   * - returns a structured error otherwise
   *
   * @param first Producer command.
   * @param second Consumer command.
   * @return Result containing both child handles on success.
   */
  [[nodiscard]] vix::error::Result<PipelineChildren> spawn(
      const Command &first,
      const Command &second);

  /**
   * @brief Wait for both pipeline children to complete.
   *
   * @param children Pipeline child handles.
   * @return Result containing both exit codes on success.
   */
  [[nodiscard]] vix::error::Result<PipelineResult> wait(
      const PipelineChildren &children);

} // namespace vix::process::pipeline

#endif // VIX_PROCESS_PIPELINE_PIPELINE_HPP
