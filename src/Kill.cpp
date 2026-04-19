/**
 *
 *  @file Kill.cpp
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

#include <vix/process/Kill.hpp>
#include <vix/process/ProcessError.hpp>

#ifndef _WIN32
#include "platform/PosixProcess.hpp"
#else
#include "platform/WindowsProcess.hpp"
#endif

namespace vix::process
{

  vix::error::Error kill(const Child &child)
  {
    if (!child.valid())
    {
      return make_process_error(
          ProcessErrorCode::InvalidArgument,
          "child process handle is invalid");
    }

#ifndef _WIN32
    return platform::kill_posix(child);
#else
    return platform::kill_windows(child);
#endif
  }

} // namespace vix::process
