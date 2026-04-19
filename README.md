# vix/process

Process management module for Vix.cpp.

## Overview

`vix::process` provides a simple and explicit API to spawn, control, and interact with system processes.

It is designed to:

- Launch processes without relying on shell parsing
- Capture stdout and stderr
- Wait for process completion
- Query process status
- Terminate or kill processes
- Provide a portable abstraction across POSIX and Windows

## Philosophy

- No hidden shell behavior
- No string parsing hacks
- Explicit process configuration
- Strong error handling via `vix::error`
- Portable by design
- Built for real-world systems

## Basic Usage

```cpp
#include <vix/process/Process.hpp>

int main()
{
  vix::process::Command command("echo");
  command.arg("hello from vix");

  auto result = vix::process::spawn(command);

  if (!result)
  {
    return 1;
  }

  return 0;
}
```

## Command API

```cpp
vix::process::Command cmd("program");

cmd.arg("value");
cmd.args({"a", "b", "c"});

cmd.cwd("/working/dir");

cmd.env("KEY", "VALUE");

cmd.stdout_mode(vix::process::PipeMode::Pipe);
cmd.stderr_mode(vix::process::PipeMode::Pipe);

cmd.detach(false);
cmd.search_in_path(true);
```

## Waiting for a Process

```cpp
auto child = vix::process::spawn(cmd);

auto result = vix::process::wait(child.value());
```

## Capturing Output

```cpp
cmd.stdout_mode(vix::process::PipeMode::Pipe);

auto result = vix::process::output(cmd);

auto text = result.value().stdout_text;
```

## Process Control

```cpp
vix::process::kill(child);
vix::process::terminate(child);
```

## Status Check

```cpp
auto running = vix::process::status(child);
```

## Error Handling

All operations return `Result<T>` or `vix::error::Error`.

```cpp
if (!result)
{
  auto err = result.error();
}
```

## Current State

The module structure and API are stable.

Platform backends are being implemented:

- POSIX: `fork / exec / pipes`
- Windows: `CreateProcess`

Some operations may currently return:

- `UnsupportedOperation`

## Examples

```text
examples/
├── basic_spawn.cpp
├── wait_process.cpp
├── capture_output.cpp
├── check_status.cpp
├── kill_process.cpp
└── terminate_process.cpp
```

## Design Notes

- `Command` is a builder object, not a shell string
- No implicit shell execution
- Environment is explicit and controlled
- Pipes are opt-in via `PipeMode`
- Errors are structured, not exceptions by default

## Roadmap

- POSIX backend (`fork + exec`)
- Windows backend (`CreateProcess`)
- Async process integration (`vix::async`)
- Stream-based I/O for pipes
- Process groups and signals
- Timeout and cancellation support

## License

MIT License
Copyright (c) 2026 Gaspard Kirira \
https://github.com/vixcpp/vix

