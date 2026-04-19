/**
 *
 *  @file PosixProcess.cpp
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

#ifndef _WIN32

#include <cerrno>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cerrno>
#include <stdexcept>
#include <exception>
#include <thread>
#include <vector>

#include <vix/process/Child.hpp>
#include <vix/process/Command.hpp>
#include <vix/process/PipeMode.hpp>
#include <vix/process/ProcessError.hpp>
#include <vix/process/ProcessResult.hpp>

#include "PosixProcess.hpp"

extern char **environ;

namespace vix::process::platform
{
  namespace
  {
    void close_if_valid(int &fd) noexcept
    {
      if (fd >= 0)
      {
        ::close(fd);
        fd = -1;
      }
    }

    std::string errno_message(const char *prefix, int err)
    {
      std::string message(prefix);
      message += ": ";
      message += std::strerror(err);
      return message;
    }

    bool set_cloexec(int fd)
    {
      const int flags = ::fcntl(fd, F_GETFD);
      if (flags < 0)
      {
        return false;
      }

      return ::fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == 0;
    }

    bool make_pipe(int fds[2])
    {
#if defined(__linux__)
      if (::pipe2(fds, O_CLOEXEC) == 0)
      {
        return true;
      }
      return false;
#else
      if (::pipe(fds) != 0)
      {
        return false;
      }

      if (!set_cloexec(fds[0]) || !set_cloexec(fds[1]))
      {
        close_if_valid(fds[0]);
        close_if_valid(fds[1]);
        return false;
      }

      return true;
#endif
    }

    int open_dev_null_readonly()
    {
      return ::open("/dev/null", O_RDONLY);
    }

    int open_dev_null_writeonly()
    {
      return ::open("/dev/null", O_WRONLY);
    }

    bool write_full(int fd, const void *data, std::size_t size) noexcept
    {
      const auto *ptr = static_cast<const char *>(data);

      while (size > 0)
      {
        const ssize_t written = ::write(fd, ptr, size);
        if (written < 0)
        {
          if (errno == EINTR)
          {
            continue;
          }
          return false;
        }

        ptr += written;
        size -= static_cast<std::size_t>(written);
      }

      return true;
    }

    std::string read_fd_to_string(int fd, std::size_t chunk_size = 4096)
    {
      std::string out;

      if (fd < 0)
      {
        return out;
      }

      std::vector<char> buffer(chunk_size);

      while (true)
      {
        const ssize_t n = ::read(fd, buffer.data(), buffer.size());

        if (n == 0)
        {
          break;
        }

        if (n < 0)
        {
          if (errno == EINTR)
          {
            continue;
          }

          throw std::runtime_error(
              errno_message("read failed while reading process output", errno));
        }

        out.append(buffer.data(), static_cast<std::size_t>(n));
      }

      return out;
    }

    std::vector<std::string> build_env_storage(const Command &command)
    {
      std::vector<std::string> env_storage;

      if (command.options().inherit_environment)
      {
        for (char **env = environ; env != nullptr && *env != nullptr; ++env)
        {
          env_storage.emplace_back(*env);
        }
      }

      for (const auto &[key, value] : command.environment())
      {
        const std::string entry = key + "=" + value;
        const std::string prefix = key + "=";

        bool replaced = false;

        for (auto &existing : env_storage)
        {
          if (existing.compare(0, prefix.size(), prefix) == 0)
          {
            existing = entry;
            replaced = true;
            break;
          }
        }

        if (!replaced)
        {
          env_storage.push_back(entry);
        }
      }

      return env_storage;
    }

    std::vector<char *> build_envp(std::vector<std::string> &env_storage)
    {
      std::vector<char *> envp;
      envp.reserve(env_storage.size() + 1);

      for (auto &entry : env_storage)
      {
        envp.push_back(entry.data());
      }

      envp.push_back(nullptr);
      return envp;
    }

    std::vector<char *> build_argv(const Command &command)
    {
      std::vector<char *> argv;
      argv.reserve(command.args().size() + 2);

      argv.push_back(const_cast<char *>(command.program().c_str()));

      for (const auto &arg : command.args())
      {
        argv.push_back(const_cast<char *>(arg.c_str()));
      }

      argv.push_back(nullptr);
      return argv;
    }

    struct FdSet
    {
      int stdin_pipe[2]{-1, -1};
      int stdout_pipe[2]{-1, -1};
      int stderr_pipe[2]{-1, -1};
      int exec_error_pipe[2]{-1, -1};

      int stdin_child_fd{-1};
      int stdout_child_fd{-1};
      int stderr_child_fd{-1};

      void close_parent_side() noexcept
      {
        close_if_valid(stdin_pipe[0]);
        close_if_valid(stdout_pipe[1]);
        close_if_valid(stderr_pipe[1]);
        close_if_valid(exec_error_pipe[1]);
      }

      void close_all() noexcept
      {
        close_if_valid(stdin_pipe[0]);
        close_if_valid(stdin_pipe[1]);
        close_if_valid(stdout_pipe[0]);
        close_if_valid(stdout_pipe[1]);
        close_if_valid(stderr_pipe[0]);
        close_if_valid(stderr_pipe[1]);
        close_if_valid(exec_error_pipe[0]);
        close_if_valid(exec_error_pipe[1]);
        close_if_valid(stdin_child_fd);
        close_if_valid(stdout_child_fd);
        close_if_valid(stderr_child_fd);
      }
    };

    bool prepare_stdio(const Command &command, FdSet &fds, vix::error::Error &error)
    {
      const auto stdin_mode = command.options().stdin_mode;
      const auto stdout_mode = command.options().stdout_mode;
      const auto stderr_mode = command.options().stderr_mode;

      if (stdin_mode == PipeMode::Pipe)
      {
        if (!make_pipe(fds.stdin_pipe))
        {
          error = make_process_error(
              ProcessErrorCode::PipeCreationFailed,
              errno_message("failed to create stdin pipe", errno));
          return false;
        }
      }
      else if (stdin_mode == PipeMode::Null)
      {
        fds.stdin_child_fd = open_dev_null_readonly();
        if (fds.stdin_child_fd < 0)
        {
          error = make_process_error(
              ProcessErrorCode::PipeCreationFailed,
              errno_message("failed to open /dev/null for stdin", errno));
          return false;
        }
      }

      if (stdout_mode == PipeMode::Pipe)
      {
        if (!make_pipe(fds.stdout_pipe))
        {
          error = make_process_error(
              ProcessErrorCode::PipeCreationFailed,
              errno_message("failed to create stdout pipe", errno));
          return false;
        }
      }
      else if (stdout_mode == PipeMode::Null)
      {
        fds.stdout_child_fd = open_dev_null_writeonly();
        if (fds.stdout_child_fd < 0)
        {
          error = make_process_error(
              ProcessErrorCode::PipeCreationFailed,
              errno_message("failed to open /dev/null for stdout", errno));
          return false;
        }
      }

      if (stderr_mode == PipeMode::Pipe)
      {
        if (!make_pipe(fds.stderr_pipe))
        {
          error = make_process_error(
              ProcessErrorCode::PipeCreationFailed,
              errno_message("failed to create stderr pipe", errno));
          return false;
        }
      }
      else if (stderr_mode == PipeMode::Null)
      {
        fds.stderr_child_fd = open_dev_null_writeonly();
        if (fds.stderr_child_fd < 0)
        {
          error = make_process_error(
              ProcessErrorCode::PipeCreationFailed,
              errno_message("failed to open /dev/null for stderr", errno));
          return false;
        }
      }

      if (!make_pipe(fds.exec_error_pipe))
      {
        error = make_process_error(
            ProcessErrorCode::PipeCreationFailed,
            errno_message("failed to create exec error pipe", errno));
        return false;
      }

      return true;
    }

    void child_fail_and_exit(int error_fd, int err) noexcept
    {
      (void)write_full(error_fd, &err, sizeof(err));
      _exit(127);
    }

    void apply_child_stdio_or_exit(
        const Command &command,
        const FdSet &fds)
    {
      const auto stdin_mode = command.options().stdin_mode;
      const auto stdout_mode = command.options().stdout_mode;
      const auto stderr_mode = command.options().stderr_mode;

      if (stdin_mode == PipeMode::Pipe)
      {
        if (::dup2(fds.stdin_pipe[0], STDIN_FILENO) < 0)
        {
          child_fail_and_exit(fds.exec_error_pipe[1], errno);
        }
      }
      else if (stdin_mode == PipeMode::Null)
      {
        if (::dup2(fds.stdin_child_fd, STDIN_FILENO) < 0)
        {
          child_fail_and_exit(fds.exec_error_pipe[1], errno);
        }
      }

      if (stdout_mode == PipeMode::Pipe)
      {
        if (::dup2(fds.stdout_pipe[1], STDOUT_FILENO) < 0)
        {
          child_fail_and_exit(fds.exec_error_pipe[1], errno);
        }
      }
      else if (stdout_mode == PipeMode::Null)
      {
        if (::dup2(fds.stdout_child_fd, STDOUT_FILENO) < 0)
        {
          child_fail_and_exit(fds.exec_error_pipe[1], errno);
        }
      }

      if (stderr_mode == PipeMode::Pipe)
      {
        if (::dup2(fds.stderr_pipe[1], STDERR_FILENO) < 0)
        {
          child_fail_and_exit(fds.exec_error_pipe[1], errno);
        }
      }
      else if (stderr_mode == PipeMode::Null)
      {
        if (::dup2(fds.stderr_child_fd, STDERR_FILENO) < 0)
        {
          child_fail_and_exit(fds.exec_error_pipe[1], errno);
        }
      }
    }

    void close_child_fds(FdSet &fds) noexcept
    {
      close_if_valid(fds.stdin_pipe[0]);
      close_if_valid(fds.stdin_pipe[1]);
      close_if_valid(fds.stdout_pipe[0]);
      close_if_valid(fds.stdout_pipe[1]);
      close_if_valid(fds.stderr_pipe[0]);
      close_if_valid(fds.stderr_pipe[1]);
      close_if_valid(fds.stdin_child_fd);
      close_if_valid(fds.stdout_child_fd);
      close_if_valid(fds.stderr_child_fd);
    }

    struct PosixProcessDeleter
    {
      void operator()(PosixProcess *proc) const noexcept
      {
        if (!proc)
        {
          return;
        }

        close_if_valid(proc->stdout_read_fd);
        close_if_valid(proc->stderr_read_fd);
        close_if_valid(proc->stdin_write_fd);
        delete proc;
      }
    };

    std::shared_ptr<void> make_backend_shared(PosixProcess *proc)
    {
      return std::shared_ptr<void>(proc, PosixProcessDeleter{});
    }

    struct SpawnOverrideFds
    {
      int stdin_fd{-1};
      int stdout_fd{-1};
      int stderr_fd{-1};

      bool has_stdin{false};
      bool has_stdout{false};
      bool has_stderr{false};
    };

    bool prepare_stdio_with_overrides(
        const Command &command,
        const SpawnOverrideFds &overrides,
        FdSet &fds,
        vix::error::Error &error)
    {
      const auto stdin_mode = command.options().stdin_mode;
      const auto stdout_mode = command.options().stdout_mode;
      const auto stderr_mode = command.options().stderr_mode;

      if (overrides.has_stdin)
      {
        fds.stdin_child_fd = overrides.stdin_fd;
      }
      else if (stdin_mode == PipeMode::Pipe)
      {
        if (!make_pipe(fds.stdin_pipe))
        {
          error = make_process_error(
              ProcessErrorCode::PipeCreationFailed,
              errno_message("failed to create stdin pipe", errno));
          return false;
        }
      }
      else if (stdin_mode == PipeMode::Null)
      {
        fds.stdin_child_fd = open_dev_null_readonly();
        if (fds.stdin_child_fd < 0)
        {
          error = make_process_error(
              ProcessErrorCode::PipeCreationFailed,
              errno_message("failed to open /dev/null for stdin", errno));
          return false;
        }
      }

      if (overrides.has_stdout)
      {
        fds.stdout_child_fd = overrides.stdout_fd;
      }
      else if (stdout_mode == PipeMode::Pipe)
      {
        if (!make_pipe(fds.stdout_pipe))
        {
          error = make_process_error(
              ProcessErrorCode::PipeCreationFailed,
              errno_message("failed to create stdout pipe", errno));
          return false;
        }
      }
      else if (stdout_mode == PipeMode::Null)
      {
        fds.stdout_child_fd = open_dev_null_writeonly();
        if (fds.stdout_child_fd < 0)
        {
          error = make_process_error(
              ProcessErrorCode::PipeCreationFailed,
              errno_message("failed to open /dev/null for stdout", errno));
          return false;
        }
      }

      if (overrides.has_stderr)
      {
        fds.stderr_child_fd = overrides.stderr_fd;
      }
      else if (stderr_mode == PipeMode::Pipe)
      {
        if (!make_pipe(fds.stderr_pipe))
        {
          error = make_process_error(
              ProcessErrorCode::PipeCreationFailed,
              errno_message("failed to create stderr pipe", errno));
          return false;
        }
      }
      else if (stderr_mode == PipeMode::Null)
      {
        fds.stderr_child_fd = open_dev_null_writeonly();
        if (fds.stderr_child_fd < 0)
        {
          error = make_process_error(
              ProcessErrorCode::PipeCreationFailed,
              errno_message("failed to open /dev/null for stderr", errno));
          return false;
        }
      }

      if (!make_pipe(fds.exec_error_pipe))
      {
        error = make_process_error(
            ProcessErrorCode::PipeCreationFailed,
            errno_message("failed to create exec error pipe", errno));
        return false;
      }

      return true;
    }

    SpawnResult spawn_posix_with_overrides(
        const Command &command,
        const SpawnOverrideFds &overrides)
    {
      if (!command.valid())
      {
        return make_process_error(
            ProcessErrorCode::EmptyProgram,
            "process program cannot be empty");
      }

      FdSet fds;
      vix::error::Error prep_error;

      if (!prepare_stdio_with_overrides(command, overrides, fds, prep_error))
      {
        fds.close_all();
        return prep_error;
      }

      auto argv = build_argv(command);
      auto env_storage = build_env_storage(command);
      auto envp = build_envp(env_storage);

      const pid_t pid = ::fork();
      if (pid < 0)
      {
        const int err = errno;
        fds.close_all();

        return make_process_error(
            ProcessErrorCode::SpawnFailed,
            errno_message("fork failed", err));
      }

      if (pid == 0)
      {
        close_if_valid(fds.exec_error_pipe[0]);

        apply_child_stdio_or_exit(command, fds);

        close_child_fds(fds);

        if (!command.options().working_directory.empty())
        {
          if (::chdir(command.options().working_directory.c_str()) != 0)
          {
            child_fail_and_exit(fds.exec_error_pipe[1], errno);
          }
        }

        if (command.options().detach)
        {
          if (::setsid() < 0)
          {
            child_fail_and_exit(fds.exec_error_pipe[1], errno);
          }
        }

        if (command.options().search_in_path)
        {
          ::execvpe(command.program().c_str(), argv.data(), envp.data());
        }
        else
        {
          ::execve(command.program().c_str(), argv.data(), envp.data());
        }

        child_fail_and_exit(fds.exec_error_pipe[1], errno);
      }

      close_if_valid(fds.exec_error_pipe[1]);

      if (!overrides.has_stdin)
      {
        close_if_valid(fds.stdin_pipe[0]);
      }

      if (!overrides.has_stdout)
      {
        close_if_valid(fds.stdout_pipe[1]);
      }

      if (!overrides.has_stderr)
      {
        close_if_valid(fds.stderr_pipe[1]);
      }

      if (!overrides.has_stdin)
      {
        close_if_valid(fds.stdin_child_fd);
      }

      if (!overrides.has_stdout)
      {
        close_if_valid(fds.stdout_child_fd);
      }

      if (!overrides.has_stderr)
      {
        close_if_valid(fds.stderr_child_fd);
      }

      int child_errno = 0;
      ssize_t read_rc = 0;

      do
      {
        read_rc = ::read(fds.exec_error_pipe[0], &child_errno, sizeof(child_errno));
      } while (read_rc < 0 && errno == EINTR);

      close_if_valid(fds.exec_error_pipe[0]);

      if (read_rc > 0)
      {
        close_if_valid(fds.stdin_pipe[1]);
        close_if_valid(fds.stdout_pipe[0]);
        close_if_valid(fds.stderr_pipe[0]);

        return make_process_error(
            ProcessErrorCode::SpawnFailed,
            errno_message("exec failed", child_errno));
      }

      auto backend = new PosixProcess();
      backend->pid = pid;
      backend->stdin_write_fd = overrides.has_stdin ? -1 : fds.stdin_pipe[1];
      backend->stdout_read_fd = overrides.has_stdout ? -1 : fds.stdout_pipe[0];
      backend->stderr_read_fd = overrides.has_stderr ? -1 : fds.stderr_pipe[0];
      backend->running = true;
      backend->exited = false;
      backend->exit_code = 0;

      return Child(
          static_cast<ProcessId>(pid),
          make_backend_shared(backend));
    }

  } // namespace

  [[nodiscard]] SpawnResult spawn_posix(const Command &command)
  {
    return spawn_posix_with_overrides(command, {});
  }

  [[nodiscard]] vix::error::Result<vix::process::pipeline::PipelineChildren> spawn_pipeline_posix(
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

    int pipe_fds[2]{-1, -1};
    if (!make_pipe(pipe_fds))
    {
      return make_process_error(
          ProcessErrorCode::PipeCreationFailed,
          errno_message("failed to create pipeline pipe", errno));
    }

    SpawnOverrideFds first_overrides;
    first_overrides.stdout_fd = pipe_fds[1];
    first_overrides.has_stdout = true;

    auto first_spawned = spawn_posix_with_overrides(first, first_overrides);
    if (!first_spawned)
    {
      close_if_valid(pipe_fds[0]);
      close_if_valid(pipe_fds[1]);
      return first_spawned.error();
    }

    SpawnOverrideFds second_overrides;
    second_overrides.stdin_fd = pipe_fds[0];
    second_overrides.has_stdin = true;

    auto second_spawned = spawn_posix_with_overrides(second, second_overrides);

    close_if_valid(pipe_fds[0]);
    close_if_valid(pipe_fds[1]);

    if (!second_spawned)
    {
      (void)kill_posix(first_spawned.value());
      (void)wait_posix(first_spawned.value());
      return second_spawned.error();
    }

    vix::process::pipeline::PipelineChildren children;
    children.first = first_spawned.value();
    children.second = second_spawned.value();
    return children;
  }

  [[nodiscard]] ProcessRunningResult status_posix(const Child &child)
  {
    if (!child.valid())
    {
      return make_process_error(
          ProcessErrorCode::InvalidArgument,
          "child process handle is invalid");
    }

    auto backend_void = child.backend();
    if (!backend_void)
    {
      return make_process_error(
          ProcessErrorCode::InvalidArgument,
          "child process backend is missing");
    }

    auto backend = std::static_pointer_cast<PosixProcess>(backend_void);
    if (!backend || backend->pid <= 0)
    {
      return make_process_error(
          ProcessErrorCode::InvalidArgument,
          "child process POSIX backend is invalid");
    }

    if (backend->exited)
    {
      backend->running = false;
      return false;
    }

    int status = 0;
    const pid_t rc = ::waitpid(backend->pid, &status, WNOHANG);

    if (rc == 0)
    {
      backend->running = true;
      return true;
    }

    if (rc < 0)
    {
      return make_process_error(
          ProcessErrorCode::StatusFailed,
          errno_message("waitpid failed while querying process status", errno));
    }

    backend->running = false;
    backend->exited = true;

    if (WIFEXITED(status))
    {
      backend->exit_code = WEXITSTATUS(status);
    }
    else if (WIFSIGNALED(status))
    {
      backend->exit_code = 128 + WTERMSIG(status);
    }
    else
    {
      backend->exit_code = 0;
    }

    return false;
  }

  [[nodiscard]] ProcessExitCodeResult wait_posix(const Child &child)
  {
    if (!child.valid())
    {
      return make_process_error(
          ProcessErrorCode::InvalidArgument,
          "child process handle is invalid");
    }

    const auto &backend_void = child.backend();
    if (!backend_void)
    {
      return make_process_error(
          ProcessErrorCode::InvalidArgument,
          "child process backend is missing");
    }

    auto backend = std::static_pointer_cast<PosixProcess>(backend_void);
    if (!backend || backend->pid <= 0)
    {
      return make_process_error(
          ProcessErrorCode::InvalidArgument,
          "child process POSIX backend is invalid");
    }

    if (backend->exited)
    {
      backend->running = false;
      return backend->exit_code;
    }

    int status = 0;
    pid_t rc = -1;

    do
    {
      rc = ::waitpid(backend->pid, &status, 0);
    } while (rc < 0 && errno == EINTR);

    if (rc < 0)
    {
      return make_process_error(
          ProcessErrorCode::WaitFailed,
          errno_message("waitpid failed while waiting for process", errno));
    }

    backend->running = false;
    backend->exited = true;

    if (WIFEXITED(status))
    {
      backend->exit_code = WEXITSTATUS(status);
    }
    else if (WIFSIGNALED(status))
    {
      backend->exit_code = 128 + WTERMSIG(status);
    }
    else
    {
      backend->exit_code = 0;
    }

    return backend->exit_code;
  }

  [[nodiscard]] vix::error::Error kill_posix(const Child &child)
  {
    if (!child.valid())
    {
      return make_process_error(
          ProcessErrorCode::InvalidArgument,
          "child process handle is invalid");
    }

    const auto &backend_void = child.backend();
    if (!backend_void)
    {
      return make_process_error(
          ProcessErrorCode::InvalidArgument,
          "child process backend is missing");
    }

    auto backend = std::static_pointer_cast<PosixProcess>(backend_void);
    if (!backend || backend->pid <= 0)
    {
      return make_process_error(
          ProcessErrorCode::InvalidArgument,
          "child process POSIX backend is invalid");
    }

    if (backend->exited)
    {
      backend->running = false;
      return {};
    }

    if (::kill(backend->pid, SIGKILL) == 0)
    {
      backend->running = false;
      return {};
    }

    const int err = errno;

    if (err == ESRCH)
    {
      backend->running = false;
      backend->exited = true;
      return {};
    }

    if (err == EPERM)
    {
      return make_process_error(
          ProcessErrorCode::PermissionDenied,
          errno_message("kill(SIGKILL) failed due to insufficient permissions", err));
    }

    return make_process_error(
        ProcessErrorCode::KillFailed,
        errno_message("kill(SIGKILL) failed while killing process", err));
  }

  [[nodiscard]] vix::error::Error terminate_posix(const Child &child)
  {
    if (!child.valid())
    {
      return make_process_error(
          ProcessErrorCode::InvalidArgument,
          "child process handle is invalid");
    }

    const auto &backend_void = child.backend();
    if (!backend_void)
    {
      return make_process_error(
          ProcessErrorCode::InvalidArgument,
          "child process backend is missing");
    }

    auto backend = std::static_pointer_cast<PosixProcess>(backend_void);
    if (!backend || backend->pid <= 0)
    {
      return make_process_error(
          ProcessErrorCode::InvalidArgument,
          "child process POSIX backend is invalid");
    }

    if (backend->exited)
    {
      backend->running = false;
      return {};
    }

    if (::kill(backend->pid, SIGTERM) == 0)
    {
      return {};
    }

    const int err = errno;

    if (err == ESRCH)
    {
      backend->running = false;
      backend->exited = true;
      return {};
    }

    if (err == EPERM)
    {
      return make_process_error(
          ProcessErrorCode::PermissionDenied,
          errno_message("kill(SIGTERM) failed due to insufficient permissions", err));
    }

    return make_process_error(
        ProcessErrorCode::TerminateFailed,
        errno_message("kill(SIGTERM) failed while terminating process", err));
  }

  [[nodiscard]] ProcessOutputResult output_posix(const Command &command)
  {
    if (!command.valid())
    {
      return make_process_error(
          ProcessErrorCode::EmptyProgram,
          "process program cannot be empty");
    }

    Command configured = command;
    configured.stdin_mode(PipeMode::Null);
    configured.stdout_mode(PipeMode::Pipe);
    configured.stderr_mode(PipeMode::Pipe);

    auto spawned = spawn_posix(configured);
    if (!spawned)
    {
      return spawned.error();
    }

    Child child = spawned.value();

    const auto &backend_void = child.backend();
    if (!backend_void)
    {
      return make_process_error(
          ProcessErrorCode::CaptureFailed,
          "child process backend is missing");
    }

    auto backend = std::static_pointer_cast<PosixProcess>(backend_void);
    if (!backend || backend->pid <= 0)
    {
      return make_process_error(
          ProcessErrorCode::CaptureFailed,
          "child process POSIX backend is invalid");
    }

    try
    {
      close_if_valid(backend->stdin_write_fd);

      std::string stdout_text;
      std::string stderr_text;
      std::exception_ptr stdout_ex;
      std::exception_ptr stderr_ex;

      std::thread stdout_reader(
          [&]()
          {
            try
            {
              stdout_text = read_fd_to_string(backend->stdout_read_fd);
            }
            catch (...)
            {
              stdout_ex = std::current_exception();
            }
          });

      std::thread stderr_reader(
          [&]()
          {
            try
            {
              stderr_text = read_fd_to_string(backend->stderr_read_fd);
            }
            catch (...)
            {
              stderr_ex = std::current_exception();
            }
          });

      auto waited = wait_posix(child);

      if (stdout_reader.joinable())
      {
        stdout_reader.join();
      }

      if (stderr_reader.joinable())
      {
        stderr_reader.join();
      }

      close_if_valid(backend->stdout_read_fd);
      close_if_valid(backend->stderr_read_fd);

      if (!waited)
      {
        return waited.error();
      }

      if (stdout_ex)
      {
        std::rethrow_exception(stdout_ex);
      }

      if (stderr_ex)
      {
        std::rethrow_exception(stderr_ex);
      }

      ProcessOutput result;
      result.exit_code = waited.value();
      result.stdout_text = std::move(stdout_text);
      result.stderr_text = std::move(stderr_text);

      return result;
    }
    catch (const std::exception &ex)
    {
      close_if_valid(backend->stdout_read_fd);
      close_if_valid(backend->stderr_read_fd);

      return make_process_error(
          ProcessErrorCode::CaptureFailed,
          ex.what());
    }
  }
} // namespace vix::process::platform

#endif // !_WIN32
