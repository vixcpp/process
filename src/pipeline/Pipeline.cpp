/**
 *
 *  @file Pipeline.cpp
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

#include <vix/process/pipeline/Pipeline.hpp>

#include <vix/process/ProcessError.hpp>
#include <vix/process/Wait.hpp>

#ifndef _WIN32
#include "../platform/PosixProcess.hpp"
#else
#include "../platform/WindowsProcess.hpp"
#endif

namespace vix::process::pipeline
{
  vix::error::Result<PipelineChildren> spawn(
      const Command &first,
      const Command &second)
  {
    if (!first.valid())
    {
      return make_process_error(
          ProcessErrorCode::EmptyProgram,
          "pipeline first command program cannot be empty");
    }

    if (!second.valid())
    {
      return make_process_error(
          ProcessErrorCode::EmptyProgram,
          "pipeline second command program cannot be empty");
    }

#ifndef _WIN32
    return platform::spawn_pipeline_posix(first, second);
#else
    return platform::spawn_pipeline_windows(first, second);
#endif
  }

  vix::error::Result<PipelineResult> wait(
      const PipelineChildren &children)
  {
    if (!children.first.valid())
    {
      return make_process_error(
          ProcessErrorCode::InvalidArgument,
          "pipeline first child handle is invalid");
    }

    if (!children.second.valid())
    {
      return make_process_error(
          ProcessErrorCode::InvalidArgument,
          "pipeline second child handle is invalid");
    }

    auto first_wait = vix::process::wait(children.first);
    if (!first_wait)
    {
      return first_wait.error();
    }

    auto second_wait = vix::process::wait(children.second);
    if (!second_wait)
    {
      return second_wait.error();
    }

    PipelineResult result;
    result.first_exit_code = first_wait.value();
    result.second_exit_code = second_wait.value();
    return result;
  }

} // namespace vix::process::pipeline
