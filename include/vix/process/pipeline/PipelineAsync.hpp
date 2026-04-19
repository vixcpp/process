/**
 *
 *  @file PipelineAsync.hpp
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
#ifndef VIX_PROCESS_PIPELINE_PIPELINEASYNC_HPP
#define VIX_PROCESS_PIPELINE_PIPELINEASYNC_HPP

#include <chrono>

#include <vix/async/core/cancel.hpp>
#include <vix/async/core/io_context.hpp>
#include <vix/async/core/task.hpp>
#include <vix/process/pipeline/Pipeline.hpp>

namespace vix::process::pipeline::async
{
  /**
   * @brief Spawn a two-stage pipeline asynchronously.
   *
   * This function offloads the blocking synchronous pipeline spawn operation
   * to the async runtime CPU pool, then resumes the awaiting coroutine on
   * the owning io_context scheduler.
   *
   * The pipeline semantic is:
   * - stdout of the first command is connected to stdin of the second command
   * - both child handles are returned on success
   *
   * @param ctx Async execution context.
   * @param first Producer command.
   * @param second Consumer command.
   * @param ct Optional cancellation token.
   * @return task<PipelineChildren> producing the spawned pipeline children.
   *
   * @throws std::runtime_error If pipeline spawning fails.
   * @throws std::system_error If cancellation is observed by the async runtime.
   */
  [[nodiscard]] vix::async::core::task<PipelineChildren> spawn(
      vix::async::core::io_context &ctx,
      Command first,
      Command second,
      vix::async::core::cancel_token ct = {});

  /**
   * @brief Wait asynchronously for both pipeline stages to complete.
   *
   * This function waits for both child processes concurrently using the async
   * process waiting API, then aggregates the two exit codes into a
   * PipelineResult.
   *
   * @param ctx Async execution context.
   * @param children Pipeline child handles to observe.
   * @param ct Optional cancellation token.
   * @param poll_interval Delay between status checks in the async wait loop.
   * @return task<PipelineResult> producing both exit codes.
   *
   * @throws std::runtime_error If waiting on one of the child processes fails.
   * @throws std::system_error If cancellation is requested.
   */
  [[nodiscard]] vix::async::core::task<PipelineResult> wait(
      vix::async::core::io_context &ctx,
      const PipelineChildren &children,
      vix::async::core::cancel_token ct = {},
      std::chrono::milliseconds poll_interval = std::chrono::milliseconds(50));

} // namespace vix::process::pipeline::async

#endif // VIX_PROCESS_PIPELINE_PIPELINEASYNC_HPP
