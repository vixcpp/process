/**
 *
 *  @file Output.cpp
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

#include <vix/process/Output.hpp>
#include <vix/process/ProcessError.hpp>

#ifndef _WIN32
#include "platform/PosixProcess.hpp"
#else
#include "platform/WindowsProcess.hpp"
#endif

namespace vix::process
{
  ProcessOutputResult output(Command command)
  {
    if (!command.valid())
    {
      return make_process_error(
          ProcessErrorCode::EmptyProgram,
          "process program cannot be empty");
    }

#ifndef _WIN32
    return platform::output_posix(command);
#else
    return platform::output_windows(command);
#endif
  }

  ProcessOutputResult output_streamed(Command command, const ProcessOutputCallbacks &callbacks)
  {
#ifndef _WIN32
    return platform::output_posix_streamed(command, callbacks);
#else
    // Windows retains the safe captured implementation until its backend gains
    // equivalent overlapped-pipe callbacks; callers still receive final text.
    auto result = output(std::move(command));
    if (result)
    {
      if (callbacks.stdout_chunk && !result.value().stdout_text.empty()) callbacks.stdout_chunk(result.value().stdout_text);
      if (callbacks.stderr_chunk && !result.value().stderr_text.empty()) callbacks.stderr_chunk(result.value().stderr_text);
    }
    return result;
#endif
  }

} // namespace vix::process
