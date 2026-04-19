/**
 *
 *  @file Spawn.hpp
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
#ifndef VIX_PROCESS_SPAWN_HPP
#define VIX_PROCESS_SPAWN_HPP

#include <vix/process/Child.hpp>
#include <vix/process/Command.hpp>
#include <vix/process/ProcessResult.hpp>

namespace vix::process
{
  /**
   * @brief Spawn a new child process from a command description.
   *
   * This is the primary process creation entry point of the module.
   *
   * Expected behavior:
   * - validates the command
   * - resolves the executable if needed
   * - applies stdio and environment options
   * - launches the process
   *
   * @param command Process launch description.
   * @return SpawnResult containing a Child handle on success.
   */
  [[nodiscard]] SpawnResult spawn(const Command &command);

} // namespace vix::process

#endif // VIX_PROCESS_SPAWN_HPP
