/**
 *
 *  @file terminate_process.cpp
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
  vix::process::Command command("sleep");
  command.arg("5");

  auto spawned = vix::process::spawn(command);
  if (!spawned)
  {
    vix::eprint("spawn failed:", spawned.error().message());
    return 1;
  }

  auto err = vix::process::terminate(spawned.value());
  if (err)
  {
    vix::eprint("terminate failed:", err.message());
    return 1;
  }

  vix::print("process termination requested:", spawned.value().id());
  return 0;
}
