/**
 *
 *  @file SpawnAsync.cpp
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
#include <utility>

#include <vix/async/core/thread_pool.hpp>
#include <vix/process/Spawn.hpp>
#include <vix/process/async/SpawnAsync.hpp>

namespace vix::process::async
{
  vix::async::core::task<Child> spawn(
      vix::async::core::io_context &ctx,
      Command command,
      vix::async::core::cancel_token ct)
  {
    co_return co_await ctx.cpu_pool().submit(
        [command = std::move(command)]() mutable -> Child
        {
          auto result = vix::process::spawn(command);

          if (!result)
          {
            throw std::runtime_error(std::string(result.error().message()));
          }

          return result.value();
        },
        std::move(ct));
  }
}
