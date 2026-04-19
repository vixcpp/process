/**
 *
 *  @file SpawnAsync.hpp
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
#ifndef VIX_PROCESS_ASYNC_SPAWNASYNC_HPP
#define VIX_PROCESS_ASYNC_SPAWNASYNC_HPP

#include <vix/async/core/cancel.hpp>
#include <vix/async/core/io_context.hpp>
#include <vix/async/core/task.hpp>
#include <vix/process/Child.hpp>
#include <vix/process/Command.hpp>

namespace vix::process::async
{
  /**
   * @brief Spawn a child process on the async CPU pool.
   *
   * This function offloads the blocking spawn operation to the async
   * runtime thread pool, then resumes the awaiting coroutine on the
   * owning io_context scheduler.
   *
   * @param ctx Async execution context.
   * @param command Command description to spawn.
   * @param ct Optional cancellation token.
   * @return task<Child> producing the spawned child handle.
   *
   * @throws std::runtime_error If spawning fails.
   * @throws std::system_error If cancellation is observed by the async runtime.
   */
  [[nodiscard]] vix::async::core::task<Child> spawn(
      vix::async::core::io_context &ctx,
      Command command,
      vix::async::core::cancel_token ct = {});
}

#endif // VIX_PROCESS_ASYNC_SPAWNASYNC_HPP
