/**
 *
 *  @file WaitAsync.hpp
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
#ifndef VIX_PROCESS_ASYNC_WAITASYNC_HPP
#define VIX_PROCESS_ASYNC_WAITASYNC_HPP

#include <chrono>

#include <vix/async/core/cancel.hpp>
#include <vix/async/core/io_context.hpp>
#include <vix/async/core/task.hpp>
#include <vix/process/Child.hpp>

namespace vix::process::async
{
  /**
   * @brief Wait asynchronously until a child process exits.
   *
   * This function polls the child status periodically and suspends the
   * coroutine between checks using the async timer service.
   *
   * @param ctx Async execution context.
   * @param child Child process handle to observe.
   * @param ct Optional cancellation token.
   * @param poll_interval Delay between status checks.
   * @return task<int> producing the final exit code.
   *
   * @throws std::runtime_error If the child handle is invalid or if a
   *         process status/wait operation fails.
   * @throws std::system_error If cancellation is requested.
   */
  [[nodiscard]] vix::async::core::task<int> wait(
      vix::async::core::io_context &ctx,
      const Child &child,
      vix::async::core::cancel_token ct = {},
      std::chrono::milliseconds poll_interval = std::chrono::milliseconds(50));
}

#endif // VIX_PROCESS_ASYNC_WAITASYNC_HPP
