/**
 *
 *  @file check_status.cpp
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

#include <vix/print.hpp>
#include <vix/process/Process.hpp>

int main()
{
  vix::process::Command command("echo");
  command.arg("status example");

  auto spawned = vix::process::spawn(command);
  if (!spawned)
  {
    vix::eprint("spawn failed:", spawned.error().message());
    return 1;
  }

  auto running = vix::process::status(spawned.value());
  if (!running)
  {
    vix::eprint("status failed:", running.error().message());
    return 1;
  }

  vix::print("process running =", running.value());
  return 0;
}
