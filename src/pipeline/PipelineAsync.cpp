/**
 *
 *  @file PipelineAsync.cpp
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

#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

#include <vix/async/core/thread_pool.hpp>
#include <vix/async/core/when.hpp>
#include <vix/process/async/WaitAsync.hpp>
#include <vix/process/pipeline/PipelineAsync.hpp>

namespace vix::process::pipeline::async
{
  vix::async::core::task<PipelineChildren> spawn(
      vix::async::core::io_context &ctx,
      Command first,
      Command second,
      vix::async::core::cancel_token ct)
  {
    co_return co_await ctx.cpu_pool().submit(
        [first = std::move(first), second = std::move(second)]() mutable -> PipelineChildren
        {
          auto result = vix::process::pipeline::spawn(first, second);

          if (!result)
          {
            throw std::runtime_error(std::string(result.error().message()));
          }

          return result.value();
        },
        std::move(ct));
  }

  vix::async::core::task<PipelineResult> wait(
      vix::async::core::io_context &ctx,
      const PipelineChildren &children,
      vix::async::core::cancel_token ct,
      std::chrono::milliseconds poll_interval)
  {
    auto [first_code, second_code] =
        co_await vix::async::core::when_all(
            ctx.get_scheduler(),
            vix::process::async::wait(ctx, children.first, ct, poll_interval),
            vix::process::async::wait(ctx, children.second, ct, poll_interval));

    PipelineResult result;
    result.first_exit_code = first_code;
    result.second_exit_code = second_code;
    co_return result;
  }

} // namespace vix::process::pipeline::async
