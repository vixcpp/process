/**
 *
 *  @file WaitAsync.cpp
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

#include <chrono>
#include <stdexcept>
#include <string>
#include <system_error>

#include <vix/async/core/cancel.hpp>
#include <vix/async/core/error.hpp>
#include <vix/async/core/timer.hpp>
#include <vix/process/ProcessError.hpp>
#include <vix/process/Status.hpp>
#include <vix/process/Wait.hpp>
#include <vix/process/async/WaitAsync.hpp>

namespace vix::process::async
{
  vix::async::core::task<int> wait(
      vix::async::core::io_context &ctx,
      const Child &child,
      vix::async::core::cancel_token ct,
      std::chrono::milliseconds poll_interval)
  {
    if (!child.valid())
    {
      throw std::runtime_error("child process handle is invalid");
    }

    if (poll_interval.count() < 0)
    {
      throw std::runtime_error("poll interval cannot be negative");
    }

    while (true)
    {
      if (ct.is_cancelled())
      {
        throw std::system_error(vix::async::core::cancelled_ec());
      }

      auto running = vix::process::status(child);
      if (!running)
      {
        throw std::runtime_error(std::string(running.error().message()));
      }

      if (!running.value())
      {
        auto waited = vix::process::wait(child);
        if (!waited)
        {
          throw std::runtime_error(std::string(waited.error().message()));
        }

        co_return waited.value();
      }

      co_await ctx.timers().sleep_for(poll_interval, ct);
    }
  }
}
