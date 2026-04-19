/**
 *
 *  @file WindowsProcess.cpp
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

#ifdef _WIN32

#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <exception>
#include <thread>
#include <vector>
#include <stdexcept>

#include <vix/process/Child.hpp>
#include <vix/process/Command.hpp>
#include <vix/process/PipeMode.hpp>
#include <vix/process/ProcessError.hpp>
#include <vix/process/ProcessResult.hpp>
#include <vix/process/pipeline/Pipeline.hpp>

#include "WindowsProcess.hpp"

namespace vix::process::platform
{
  namespace
  {
    void close_handle_if_valid(HANDLE &handle) noexcept
    {
      if (handle != nullptr && handle != INVALID_HANDLE_VALUE)
      {
        ::CloseHandle(handle);
        handle = nullptr;
      }
    }

    std::string win32_message(const char *prefix, DWORD code)
    {
      LPSTR buffer = nullptr;

      const DWORD flags =
          FORMAT_MESSAGE_ALLOCATE_BUFFER |
          FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS;

      const DWORD size = ::FormatMessageA(
          flags,
          nullptr,
          code,
          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
          reinterpret_cast<LPSTR>(&buffer),
          0,
          nullptr);

      std::string message(prefix);
      message += ": ";

      if (size != 0 && buffer != nullptr)
      {
        message += buffer;

        while (!message.empty() &&
               (message.back() == '\r' || message.back() == '\n' || message.back() == ' '))
        {
          message.pop_back();
        }
      }
      else
      {
        message += "Windows error ";
        message += std::to_string(code);
      }

      if (buffer != nullptr)
      {
        ::LocalFree(buffer);
      }

      return message;
    }

    std::wstring widen(std::string_view value)
    {
      if (value.empty())
      {
        return std::wstring{};
      }

      const int size = ::MultiByteToWideChar(
          CP_UTF8,
          0,
          value.data(),
          static_cast<int>(value.size()),
          nullptr,
          0);

      if (size <= 0)
      {
        return std::wstring(value.begin(), value.end());
      }

      std::wstring out(static_cast<std::size_t>(size), L'\0');

      ::MultiByteToWideChar(
          CP_UTF8,
          0,
          value.data(),
          static_cast<int>(value.size()),
          out.data(),
          size);

      return out;
    }

    bool needs_quotes(std::string_view value) noexcept
    {
      if (value.empty())
      {
        return true;
      }

      for (const char ch : value)
      {
        if (ch == ' ' || ch == '\t' || ch == '"')
        {
          return true;
        }
      }

      return false;
    }

    std::wstring quote_windows_arg(std::string_view arg)
    {
      if (!needs_quotes(arg))
      {
        return widen(arg);
      }

      std::wstring out;
      out.push_back(L'"');

      std::size_t backslashes = 0;

      for (const char ch : arg)
      {
        if (ch == '\\')
        {
          ++backslashes;
          continue;
        }

        if (ch == '"')
        {
          out.append(backslashes * 2 + 1, L'\\');
          out.push_back(L'"');
          backslashes = 0;
          continue;
        }

        if (backslashes != 0)
        {
          out.append(backslashes, L'\\');
          backslashes = 0;
        }

        out.push_back(static_cast<wchar_t>(static_cast<unsigned char>(ch)));
      }

      if (backslashes != 0)
      {
        out.append(backslashes * 2, L'\\');
      }

      out.push_back(L'"');
      return out;
    }

    std::wstring build_command_line(const Command &command)
    {
      std::wstring cmdline = quote_windows_arg(command.program());

      for (const auto &arg : command.args())
      {
        cmdline.push_back(L' ');
        cmdline += quote_windows_arg(arg);
      }

      return cmdline;
    }

    std::vector<std::wstring> build_env_storage(const Command &command)
    {
      std::vector<std::wstring> env_storage;

      if (command.options().inherit_environment)
      {
        LPWCH block = ::GetEnvironmentStringsW();
        if (block != nullptr)
        {
          const wchar_t *it = block;
          while (*it != L'\0')
          {
            std::wstring entry(it);
            env_storage.push_back(entry);
            it += entry.size() + 1;
          }
          ::FreeEnvironmentStringsW(block);
        }
      }

      for (const auto &[key_utf8, value_utf8] : command.environment())
      {
        const std::wstring key = widen(key_utf8);
        const std::wstring value = widen(value_utf8);
        const std::wstring entry = key + L"=" + value;
        const std::wstring prefix = key + L"=";

        bool replaced = false;

        for (auto &existing : env_storage)
        {
          if (existing.size() >= prefix.size() &&
              existing.compare(0, prefix.size(), prefix) == 0)
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

    std::vector<wchar_t> build_environment_block(const Command &command)
    {
      auto env_storage = build_env_storage(command);
      std::vector<wchar_t> block;

      if (env_storage.empty())
      {
        block.push_back(L'\0');
        block.push_back(L'\0');
        return block;
      }

      std::size_t total = 1;
      for (const auto &entry : env_storage)
      {
        total += entry.size() + 1;
      }

      block.reserve(total);

      for (const auto &entry : env_storage)
      {
        block.insert(block.end(), entry.begin(), entry.end());
        block.push_back(L'\0');
      }

      block.push_back(L'\0');
      return block;
    }

    HANDLE open_null_read()
    {
      return ::CreateFileW(
          L"NUL",
          GENERIC_READ,
          FILE_SHARE_READ | FILE_SHARE_WRITE,
          nullptr,
          OPEN_EXISTING,
          FILE_ATTRIBUTE_NORMAL,
          nullptr);
    }

    HANDLE open_null_write()
    {
      return ::CreateFileW(
          L"NUL",
          GENERIC_WRITE,
          FILE_SHARE_READ | FILE_SHARE_WRITE,
          nullptr,
          OPEN_EXISTING,
          FILE_ATTRIBUTE_NORMAL,
          nullptr);
    }

    bool make_inheritable_pipe(HANDLE &read_handle, HANDLE &write_handle)
    {
      SECURITY_ATTRIBUTES sa{};
      sa.nLength = sizeof(sa);
      sa.lpSecurityDescriptor = nullptr;
      sa.bInheritHandle = TRUE;

      if (!::CreatePipe(&read_handle, &write_handle, &sa, 0))
      {
        read_handle = nullptr;
        write_handle = nullptr;
        return false;
      }

      return true;
    }

    struct HandleSet
    {
      HANDLE stdin_read{nullptr};
      HANDLE stdin_write{nullptr};

      HANDLE stdout_read{nullptr};
      HANDLE stdout_write{nullptr};

      HANDLE stderr_read{nullptr};
      HANDLE stderr_write{nullptr};

      HANDLE child_stdin{nullptr};
      HANDLE child_stdout{nullptr};
      HANDLE child_stderr{nullptr};

      void close_all() noexcept
      {
        close_handle_if_valid(stdin_read);
        close_handle_if_valid(stdin_write);
        close_handle_if_valid(stdout_read);
        close_handle_if_valid(stdout_write);
        close_handle_if_valid(stderr_read);
        close_handle_if_valid(stderr_write);
        close_handle_if_valid(child_stdin);
        close_handle_if_valid(child_stdout);
        close_handle_if_valid(child_stderr);
      }
    };

    struct WindowsProcessDeleter
    {
      void operator()(WindowsProcess *proc) const noexcept
      {
        if (!proc)
        {
          return;
        }

        close_handle_if_valid(proc->process_handle);
        close_handle_if_valid(proc->thread_handle);
        close_handle_if_valid(proc->stdout_read_handle);
        close_handle_if_valid(proc->stderr_read_handle);
        close_handle_if_valid(proc->stdin_write_handle);
        delete proc;
      }
    };

    std::shared_ptr<void> make_backend_shared(WindowsProcess *proc)
    {
      return std::shared_ptr<void>(proc, WindowsProcessDeleter{});
    }

    bool prepare_stdio(const Command &command, HandleSet &hs, vix::error::Error &error)
    {
      const auto stdin_mode = command.options().stdin_mode;
      const auto stdout_mode = command.options().stdout_mode;
      const auto stderr_mode = command.options().stderr_mode;

      if (stdin_mode == PipeMode::Pipe)
      {
        if (!make_inheritable_pipe(hs.stdin_read, hs.stdin_write))
        {
          error = make_process_error(
              ProcessErrorCode::PipeCreationFailed,
              win32_message("failed to create stdin pipe", ::GetLastError()));
          return false;
        }

        hs.child_stdin = hs.stdin_read;
      }
      else if (stdin_mode == PipeMode::Null)
      {
        hs.child_stdin = open_null_read();
        if (hs.child_stdin == INVALID_HANDLE_VALUE || hs.child_stdin == nullptr)
        {
          hs.child_stdin = nullptr;
          error = make_process_error(
              ProcessErrorCode::PipeCreationFailed,
              win32_message("failed to open NUL for stdin", ::GetLastError()));
          return false;
        }
      }

      if (stdout_mode == PipeMode::Pipe)
      {
        if (!make_inheritable_pipe(hs.stdout_read, hs.stdout_write))
        {
          error = make_process_error(
              ProcessErrorCode::PipeCreationFailed,
              win32_message("failed to create stdout pipe", ::GetLastError()));
          return false;
        }

        hs.child_stdout = hs.stdout_write;
      }
      else if (stdout_mode == PipeMode::Null)
      {
        hs.child_stdout = open_null_write();
        if (hs.child_stdout == INVALID_HANDLE_VALUE || hs.child_stdout == nullptr)
        {
          hs.child_stdout = nullptr;
          error = make_process_error(
              ProcessErrorCode::PipeCreationFailed,
              win32_message("failed to open NUL for stdout", ::GetLastError()));
          return false;
        }
      }

      if (stderr_mode == PipeMode::Pipe)
      {
        if (!make_inheritable_pipe(hs.stderr_read, hs.stderr_write))
        {
          error = make_process_error(
              ProcessErrorCode::PipeCreationFailed,
              win32_message("failed to create stderr pipe", ::GetLastError()));
          return false;
        }

        hs.child_stderr = hs.stderr_write;
      }
      else if (stderr_mode == PipeMode::Null)
      {
        hs.child_stderr = open_null_write();
        if (hs.child_stderr == INVALID_HANDLE_VALUE || hs.child_stderr == nullptr)
        {
          hs.child_stderr = nullptr;
          error = make_process_error(
              ProcessErrorCode::PipeCreationFailed,
              win32_message("failed to open NUL for stderr", ::GetLastError()));
          return false;
        }
      }

      return true;
    }

    std::string read_handle_to_string(HANDLE handle, DWORD chunk_size = 4096)
    {
      std::string out;

      if (handle == nullptr || handle == INVALID_HANDLE_VALUE)
      {
        return out;
      }

      std::vector<char> buffer(chunk_size);

      while (true)
      {
        DWORD bytes_read = 0;
        const BOOL ok = ::ReadFile(
            handle,
            buffer.data(),
            static_cast<DWORD>(buffer.size()),
            &bytes_read,
            nullptr);

        if (!ok)
        {
          const DWORD err = ::GetLastError();
          if (err == ERROR_BROKEN_PIPE)
          {
            break;
          }

          throw std::runtime_error(
              win32_message("ReadFile failed while reading process output", err));
        }

        if (bytes_read == 0)
        {
          break;
        }

        out.append(buffer.data(), buffer.data() + bytes_read);
      }

      return out;
    }

    struct SpawnOverrideHandles
    {
      HANDLE stdin_handle{nullptr};
      HANDLE stdout_handle{nullptr};
      HANDLE stderr_handle{nullptr};

      bool has_stdin{false};
      bool has_stdout{false};
      bool has_stderr{false};
    };

    bool set_handle_inherit(HANDLE handle, bool inheritable)
    {
      if (handle == nullptr || handle == INVALID_HANDLE_VALUE)
      {
        return false;
      }

      const DWORD flags = inheritable ? HANDLE_FLAG_INHERIT : 0;
      return ::SetHandleInformation(handle, HANDLE_FLAG_INHERIT, flags) != 0;
    }

    bool prepare_stdio_with_overrides(
        const Command &command,
        const SpawnOverrideHandles &overrides,
        HandleSet &hs,
        vix::error::Error &error)
    {
      const auto stdin_mode = command.options().stdin_mode;
      const auto stdout_mode = command.options().stdout_mode;
      const auto stderr_mode = command.options().stderr_mode;

      if (overrides.has_stdin)
      {
        hs.child_stdin = overrides.stdin_handle;
      }
      else if (stdin_mode == PipeMode::Pipe)
      {
        if (!make_inheritable_pipe(hs.stdin_read, hs.stdin_write))
        {
          error = make_process_error(
              ProcessErrorCode::PipeCreationFailed,
              win32_message("failed to create stdin pipe", ::GetLastError()));
          return false;
        }

        hs.child_stdin = hs.stdin_read;
      }
      else if (stdin_mode == PipeMode::Null)
      {
        hs.child_stdin = open_null_read();
        if (hs.child_stdin == INVALID_HANDLE_VALUE || hs.child_stdin == nullptr)
        {
          hs.child_stdin = nullptr;
          error = make_process_error(
              ProcessErrorCode::PipeCreationFailed,
              win32_message("failed to open NUL for stdin", ::GetLastError()));
          return false;
        }
      }

      if (overrides.has_stdout)
      {
        hs.child_stdout = overrides.stdout_handle;
      }
      else if (stdout_mode == PipeMode::Pipe)
      {
        if (!make_inheritable_pipe(hs.stdout_read, hs.stdout_write))
        {
          error = make_process_error(
              ProcessErrorCode::PipeCreationFailed,
              win32_message("failed to create stdout pipe", ::GetLastError()));
          return false;
        }

        hs.child_stdout = hs.stdout_write;
      }
      else if (stdout_mode == PipeMode::Null)
      {
        hs.child_stdout = open_null_write();
        if (hs.child_stdout == INVALID_HANDLE_VALUE || hs.child_stdout == nullptr)
        {
          hs.child_stdout = nullptr;
          error = make_process_error(
              ProcessErrorCode::PipeCreationFailed,
              win32_message("failed to open NUL for stdout", ::GetLastError()));
          return false;
        }
      }

      if (overrides.has_stderr)
      {
        hs.child_stderr = overrides.stderr_handle;
      }
      else if (stderr_mode == PipeMode::Pipe)
      {
        if (!make_inheritable_pipe(hs.stderr_read, hs.stderr_write))
        {
          error = make_process_error(
              ProcessErrorCode::PipeCreationFailed,
              win32_message("failed to create stderr pipe", ::GetLastError()));
          return false;
        }

        hs.child_stderr = hs.stderr_write;
      }
      else if (stderr_mode == PipeMode::Null)
      {
        hs.child_stderr = open_null_write();
        if (hs.child_stderr == INVALID_HANDLE_VALUE || hs.child_stderr == nullptr)
        {
          hs.child_stderr = nullptr;
          error = make_process_error(
              ProcessErrorCode::PipeCreationFailed,
              win32_message("failed to open NUL for stderr", ::GetLastError()));
          return false;
        }
      }

      return true;
    }

    SpawnResult spawn_windows_with_overrides(
        const Command &command,
        const SpawnOverrideHandles &overrides)
    {
      if (!command.valid())
      {
        return make_process_error(
            ProcessErrorCode::EmptyProgram,
            "process program cannot be empty");
      }

      HandleSet hs;
      vix::error::Error prep_error;

      if (!prepare_stdio_with_overrides(command, overrides, hs, prep_error))
      {
        hs.close_all();
        return prep_error;
      }

      STARTUPINFOW startup{};
      startup.cb = sizeof(startup);
      startup.dwFlags = STARTF_USESTDHANDLES;
      startup.hStdInput =
          overrides.has_stdin
              ? overrides.stdin_handle
              : ((command.options().stdin_mode == PipeMode::Inherit)
                     ? ::GetStdHandle(STD_INPUT_HANDLE)
                     : hs.child_stdin);
      startup.hStdOutput =
          overrides.has_stdout
              ? overrides.stdout_handle
              : ((command.options().stdout_mode == PipeMode::Inherit)
                     ? ::GetStdHandle(STD_OUTPUT_HANDLE)
                     : hs.child_stdout);
      startup.hStdError =
          overrides.has_stderr
              ? overrides.stderr_handle
              : ((command.options().stderr_mode == PipeMode::Inherit)
                     ? ::GetStdHandle(STD_ERROR_HANDLE)
                     : hs.child_stderr);

      PROCESS_INFORMATION pi{};
      std::wstring program = widen(command.program());
      std::wstring command_line = build_command_line(command);
      std::wstring working_directory = widen(command.options().working_directory);
      std::vector<wchar_t> env_block = build_environment_block(command);

      DWORD creation_flags = CREATE_UNICODE_ENVIRONMENT;

      if (command.options().detach)
      {
        creation_flags |= DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP;
      }

      const wchar_t *current_directory =
          working_directory.empty() ? nullptr : working_directory.c_str();

      BOOL ok = FALSE;

      if (command.options().search_in_path)
      {
        ok = ::CreateProcessW(
            nullptr,
            command_line.data(),
            nullptr,
            nullptr,
            TRUE,
            creation_flags,
            env_block.data(),
            current_directory,
            &startup,
            &pi);
      }
      else
      {
        ok = ::CreateProcessW(
            program.c_str(),
            command_line.data(),
            nullptr,
            nullptr,
            TRUE,
            creation_flags,
            env_block.data(),
            current_directory,
            &startup,
            &pi);
      }

      if (!ok)
      {
        const DWORD err = ::GetLastError();
        hs.close_all();

        return make_process_error(
            ProcessErrorCode::SpawnFailed,
            win32_message("CreateProcessW failed", err));
      }

      if (!overrides.has_stdin)
      {
        close_handle_if_valid(hs.stdin_read);
        close_handle_if_valid(hs.child_stdin);
      }

      if (!overrides.has_stdout)
      {
        close_handle_if_valid(hs.stdout_write);
        close_handle_if_valid(hs.child_stdout);
      }

      if (!overrides.has_stderr)
      {
        close_handle_if_valid(hs.stderr_write);
        close_handle_if_valid(hs.child_stderr);
      }

      auto backend = new WindowsProcess();
      backend->process_handle = pi.hProcess;
      backend->thread_handle = pi.hThread;
      backend->pid = pi.dwProcessId;
      backend->stdout_read_handle = overrides.has_stdout ? nullptr : hs.stdout_read;
      backend->stderr_read_handle = overrides.has_stderr ? nullptr : hs.stderr_read;
      backend->stdin_write_handle = overrides.has_stdin ? nullptr : hs.stdin_write;
      backend->running = true;
      backend->exited = false;
      backend->exit_code = 0;

      return Child(
          static_cast<ProcessId>(pi.dwProcessId),
          make_backend_shared(backend));
    }

  } // namespace

  [[nodiscard]] SpawnResult spawn_windows(const Command &command)
  {
    return spawn_windows_with_overrides(command, {});
  }

  [[nodiscard]] ProcessRunningResult status_windows(const Child &child)
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

    auto backend = std::static_pointer_cast<WindowsProcess>(backend_void);
    if (!backend || backend->process_handle == nullptr)
    {
      return make_process_error(
          ProcessErrorCode::InvalidArgument,
          "child process Windows backend is invalid");
    }

    if (backend->exited)
    {
      backend->running = false;
      return false;
    }

    DWORD exit_code = STILL_ACTIVE;
    if (!::GetExitCodeProcess(backend->process_handle, &exit_code))
    {
      return make_process_error(
          ProcessErrorCode::StatusFailed,
          win32_message("GetExitCodeProcess failed while querying process status", ::GetLastError()));
    }

    if (exit_code == STILL_ACTIVE)
    {
      backend->running = true;
      return true;
    }

    backend->running = false;
    backend->exited = true;
    backend->exit_code = static_cast<int>(exit_code);

    return false;
  }

  [[nodiscard]] vix::error::Result<vix::process::pipeline::PipelineChildren> spawn_pipeline_windows(
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

    HANDLE pipe_read = nullptr;
    HANDLE pipe_write = nullptr;

    if (!make_inheritable_pipe(pipe_read, pipe_write))
    {
      return make_process_error(
          ProcessErrorCode::PipeCreationFailed,
          win32_message("failed to create pipeline pipe", ::GetLastError()));
    }

    if (!set_handle_inherit(pipe_read, false))
    {
      close_handle_if_valid(pipe_read);
      close_handle_if_valid(pipe_write);

      return make_process_error(
          ProcessErrorCode::PipeCreationFailed,
          win32_message("failed to update pipeline read handle inheritance", ::GetLastError()));
    }

    SpawnOverrideHandles first_overrides;
    first_overrides.stdout_handle = pipe_write;
    first_overrides.has_stdout = true;

    auto first_spawned = spawn_windows_with_overrides(first, first_overrides);
    if (!first_spawned)
    {
      close_handle_if_valid(pipe_read);
      close_handle_if_valid(pipe_write);
      return first_spawned.error();
    }

    if (!set_handle_inherit(pipe_write, false))
    {
      close_handle_if_valid(pipe_read);
      close_handle_if_valid(pipe_write);

      (void)kill_windows(first_spawned.value());
      (void)wait_windows(first_spawned.value());

      return make_process_error(
          ProcessErrorCode::PipeCreationFailed,
          win32_message("failed to update pipeline write handle inheritance", ::GetLastError()));
    }

    if (!set_handle_inherit(pipe_read, true))
    {
      close_handle_if_valid(pipe_read);
      close_handle_if_valid(pipe_write);

      (void)kill_windows(first_spawned.value());
      (void)wait_windows(first_spawned.value());

      return make_process_error(
          ProcessErrorCode::PipeCreationFailed,
          win32_message("failed to enable pipeline read handle inheritance", ::GetLastError()));
    }

    SpawnOverrideHandles second_overrides;
    second_overrides.stdin_handle = pipe_read;
    second_overrides.has_stdin = true;

    auto second_spawned = spawn_windows_with_overrides(second, second_overrides);

    close_handle_if_valid(pipe_read);
    close_handle_if_valid(pipe_write);

    if (!second_spawned)
    {
      (void)kill_windows(first_spawned.value());
      (void)wait_windows(first_spawned.value());
      return second_spawned.error();
    }

    vix::process::pipeline::PipelineChildren children;
    children.first = first_spawned.value();
    children.second = second_spawned.value();
    return children;
  }

  [[nodiscard]] ProcessExitCodeResult wait_windows(const Child &child)
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

    auto backend = std::static_pointer_cast<WindowsProcess>(backend_void);
    if (!backend || backend->process_handle == nullptr)
    {
      return make_process_error(
          ProcessErrorCode::InvalidArgument,
          "child process Windows backend is invalid");
    }

    if (backend->exited)
    {
      backend->running = false;
      return backend->exit_code;
    }

    const DWORD wait_rc = ::WaitForSingleObject(backend->process_handle, INFINITE);
    if (wait_rc != WAIT_OBJECT_0)
    {
      return make_process_error(
          ProcessErrorCode::WaitFailed,
          win32_message("WaitForSingleObject failed while waiting for process", ::GetLastError()));
    }

    DWORD exit_code = 0;
    if (!::GetExitCodeProcess(backend->process_handle, &exit_code))
    {
      return make_process_error(
          ProcessErrorCode::WaitFailed,
          win32_message("GetExitCodeProcess failed after process wait", ::GetLastError()));
    }

    backend->running = false;
    backend->exited = true;
    backend->exit_code = static_cast<int>(exit_code);

    return backend->exit_code;
  }

  [[nodiscard]] vix::error::Error kill_windows(const Child &child)
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

    auto backend = std::static_pointer_cast<WindowsProcess>(backend_void);
    if (!backend || backend->process_handle == nullptr)
    {
      return make_process_error(
          ProcessErrorCode::InvalidArgument,
          "child process Windows backend is invalid");
    }

    if (backend->exited)
    {
      backend->running = false;
      return {};
    }

    DWORD exit_code = STILL_ACTIVE;
    if (::GetExitCodeProcess(backend->process_handle, &exit_code))
    {
      if (exit_code != STILL_ACTIVE)
      {
        backend->running = false;
        backend->exited = true;
        backend->exit_code = static_cast<int>(exit_code);
        return {};
      }
    }

    if (!::TerminateProcess(backend->process_handle, 1))
    {
      return make_process_error(
          ProcessErrorCode::KillFailed,
          win32_message("TerminateProcess failed while killing process", ::GetLastError()));
    }

    backend->running = false;
    backend->exited = true;
    backend->exit_code = 1;

    return {};
  }
  [[nodiscard]] vix::error::Error terminate_windows(const Child &child)
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

    auto backend = std::static_pointer_cast<WindowsProcess>(backend_void);
    if (!backend || backend->process_handle == nullptr)
    {
      return make_process_error(
          ProcessErrorCode::InvalidArgument,
          "child process Windows backend is invalid");
    }

    if (backend->exited)
    {
      backend->running = false;
      return {};
    }

    DWORD exit_code = STILL_ACTIVE;
    if (::GetExitCodeProcess(backend->process_handle, &exit_code))
    {
      if (exit_code != STILL_ACTIVE)
      {
        backend->running = false;
        backend->exited = true;
        backend->exit_code = static_cast<int>(exit_code);
        return {};
      }
    }

    if (!::TerminateProcess(backend->process_handle, 1))
    {
      return make_process_error(
          ProcessErrorCode::TerminateFailed,
          win32_message("TerminateProcess failed while terminating process", ::GetLastError()));
    }

    backend->running = false;
    backend->exited = true;
    backend->exit_code = 1;

    return {};
  }

  [[nodiscard]] ProcessOutputResult output_windows(const Command &command)
  {
    if (!command.valid())
    {
      return make_process_error(
          ProcessErrorCode::EmptyProgram,
          "process program cannot be empty");
    }

    Command configured = command;
    configured.stdout_mode(PipeMode::Pipe);
    configured.stderr_mode(PipeMode::Pipe);
    configured.stdin_mode(PipeMode::Null);

    auto spawned = spawn_windows(configured);
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

    auto backend = std::static_pointer_cast<WindowsProcess>(backend_void);
    if (!backend || backend->process_handle == nullptr)
    {
      return make_process_error(
          ProcessErrorCode::CaptureFailed,
          "child process Windows backend is invalid");
    }

    try
    {
      close_handle_if_valid(backend->stdin_write_handle);

      std::string stdout_text;
      std::string stderr_text;
      std::exception_ptr stdout_ex;
      std::exception_ptr stderr_ex;

      std::thread stdout_reader(
          [&]()
          {
            try
            {
              stdout_text = read_handle_to_string(backend->stdout_read_handle);
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
              stderr_text = read_handle_to_string(backend->stderr_read_handle);
            }
            catch (...)
            {
              stderr_ex = std::current_exception();
            }
          });

      auto waited = wait_windows(child);

      if (stdout_reader.joinable())
      {
        stdout_reader.join();
      }

      if (stderr_reader.joinable())
      {
        stderr_reader.join();
      }

      close_handle_if_valid(backend->stdout_read_handle);
      close_handle_if_valid(backend->stderr_read_handle);

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
      close_handle_if_valid(backend->stdout_read_handle);
      close_handle_if_valid(backend->stderr_read_handle);

      return make_process_error(
          ProcessErrorCode::CaptureFailed,
          ex.what());
    }
  }

} // namespace vix::process::platform

#endif // _WIN32
