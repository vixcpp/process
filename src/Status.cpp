/**
 *
 *  @file Spawn.cpp
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

#include <utility>

#include <vix/process/Spawn.hpp>
#include <vix/process/ProcessError.hpp>

#ifndef _WIN32
#include "platform/PosixProcess.hpp"
#else
#include "platform/WindowsProcess.hpp"
#endif

namespace vix::process
{
  SpawnResult spawn(Command command)
  {
    if (!command.valid())
    {
      return make_process_error(
          ProcessErrorCode::EmptyProgram,
          "process program cannot be empty");
    }

#ifndef _WIN32
    return platform::spawn_posix(command);
#else
    return platform::spawn_windows(command);
#endif
  }

} // namespace vix::process
