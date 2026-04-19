/**
 *
 *  @file Command.hpp
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
#ifndef VIX_PROCESS_COMMAND_HPP
#define VIX_PROCESS_COMMAND_HPP

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <vix/process/PipeMode.hpp>
#include <vix/process/ProcessOptions.hpp>

namespace vix::process
{
  /**
   * @class Command
   * @brief Describes a process to spawn.
   *
   * Command is the main builder object of the process module.
   * It stores:
   * - program path or executable name
   * - arguments
   * - environment overrides
   * - execution options
   *
   * Design goals:
   * - explicit and readable
   * - easy to compose
   * - no hidden shell parsing
   * - portable across POSIX and Windows
   */
  class Command
  {
  public:
    /**
     * @brief Construct a command from a program path or executable name.
     *
     * @param program Executable path or program name.
     */
    explicit Command(std::string program)
        : program_(std::move(program))
    {
    }

    /**
     * @brief Set the program path or executable name.
     *
     * @param program Executable path or program name.
     * @return Reference to this command.
     */
    Command &program(std::string program)
    {
      program_ = std::move(program);
      return *this;
    }

    /**
     * @brief Add one argument.
     *
     * @param value Argument value.
     * @return Reference to this command.
     */
    Command &arg(std::string value)
    {
      args_.push_back(std::move(value));
      return *this;
    }

    /**
     * @brief Add many arguments.
     *
     * @param values Arguments to append.
     * @return Reference to this command.
     */
    Command &args(std::vector<std::string> values)
    {
      for (auto &value : values)
      {
        args_.push_back(std::move(value));
      }
      return *this;
    }

    /**
     * @brief Add one environment override.
     *
     * The override is stored as a key-value pair and may later be
     * merged with the inherited process environment depending on options.
     *
     * @param key Environment variable name.
     * @param value Environment variable value.
     * @return Reference to this command.
     */
    Command &env(std::string key, std::string value)
    {
      environment_.emplace_back(std::move(key), std::move(value));
      return *this;
    }

    /**
     * @brief Set the working directory for the child process.
     *
     * @param value Working directory path.
     * @return Reference to this command.
     */
    Command &cwd(std::string value)
    {
      options_.working_directory = std::move(value);
      return *this;
    }

    /**
     * @brief Set stdin pipe mode.
     *
     * @param mode Pipe handling mode.
     * @return Reference to this command.
     */
    Command &stdin_mode(PipeMode mode) noexcept
    {
      options_.stdin_mode = mode;
      return *this;
    }

    /**
     * @brief Set stdout pipe mode.
     *
     * @param mode Pipe handling mode.
     * @return Reference to this command.
     */
    Command &stdout_mode(PipeMode mode) noexcept
    {
      options_.stdout_mode = mode;
      return *this;
    }

    /**
     * @brief Set stderr pipe mode.
     *
     * @param mode Pipe handling mode.
     * @return Reference to this command.
     */
    Command &stderr_mode(PipeMode mode) noexcept
    {
      options_.stderr_mode = mode;
      return *this;
    }

    /**
     * @brief Control whether executable lookup may use PATH.
     *
     * @param value True to search in PATH, false otherwise.
     * @return Reference to this command.
     */
    Command &search_in_path(bool value) noexcept
    {
      options_.search_in_path = value;
      return *this;
    }

    /**
     * @brief Control whether the child process is detached.
     *
     * @param value True to detach, false otherwise.
     * @return Reference to this command.
     */
    Command &detach(bool value) noexcept
    {
      options_.detach = value;
      return *this;
    }

    /**
     * @brief Control whether the parent environment is inherited.
     *
     * @param value True to inherit, false otherwise.
     * @return Reference to this command.
     */
    Command &inherit_environment(bool value) noexcept
    {
      options_.inherit_environment = value;
      return *this;
    }

    /**
     * @brief Access the configured program.
     */
    [[nodiscard]] const std::string &program() const noexcept
    {
      return program_;
    }

    /**
     * @brief Access the configured argument list.
     */
    [[nodiscard]] const std::vector<std::string> &args() const noexcept
    {
      return args_;
    }

    /**
     * @brief Access the configured environment overrides.
     */
    [[nodiscard]] const std::vector<std::pair<std::string, std::string>> &environment() const noexcept
    {
      return environment_;
    }

    /**
     * @brief Access the execution options.
     */
    [[nodiscard]] const ProcessOptions &options() const noexcept
    {
      return options_;
    }

    /**
     * @brief Return true if the command has a non-empty program.
     */
    [[nodiscard]] bool valid() const noexcept
    {
      return !program_.empty();
    }

  private:
    std::string program_{};
    std::vector<std::string> args_{};
    std::vector<std::pair<std::string, std::string>> environment_{};
    ProcessOptions options_{};
  };

} // namespace vix::process

#endif // VIX_PROCESS_COMMAND_HPP
