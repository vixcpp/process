/**
 *
 *  @file OutputAsync.hpp
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
#ifndef VIX_PROCESS_ASYNC_OUTPUTASYNC_HPP
#define VIX_PROCESS_ASYNC_OUTPUTASYNC_HPP

#include <vix/async/core/cancel.hpp>
#include <vix/async/core/io_context.hpp>
#include <vix/async/core/task.hpp>
#include <vix/process/Command.hpp>
#include <vix/process/ProcessResult.hpp>

namespace vix::process::async
{
  /**
   * @brief Run a command asynchronously and capture its output.
   *
   * This function offloads the blocking output() operation to the async
   * runtime thread pool, then resumes the awaiting coroutine on the
   * owning io_context scheduler.
   *
   * @param ctx Async execution context.
   * @param command Command description to run.
   * @param ct Optional cancellation token.
   * @return task<ProcessOutput> producing the captured process result.
   *
   * @throws std::runtime_error If process execution fails.
   * @throws std::system_error If cancellation is observed by the async runtime.
   */
  [[nodiscard]] vix::async::core::task<ProcessOutput> output(
      vix::async::core::io_context &ctx,
      Command command,
      vix::async::core::cancel_token ct = {});
}

#endif // VIX_PROCESS_ASYNC_OUTPUTASYNC_HPP
