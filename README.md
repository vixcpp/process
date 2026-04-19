# vix/process

Process management module for Vix.cpp.

## Overview

`vix::process` provides a production-ready API to spawn, control, and interact with system processes.

It is designed for:

- deterministic process execution
- explicit I/O control
- safe error handling
- cross-platform behavior (POSIX + Windows)

## What you get

- Spawn processes without a shell
- Capture stdout and stderr safely
- Wait or poll process state
- Kill or terminate processes
- Build pipelines (process → process)
- Async integration via `vix::async`

## Philosophy

- No shell by default
- No string parsing hacks
- Explicit configuration over magic
- Structured errors, not hidden failures
- Same API across POSIX and Windows

## Basic Usage

```cpp
#include <vix/process/Process.hpp>

int main()
{
  vix::process::Command cmd("echo");
  cmd.arg("hello from vix");

  auto result = vix::process::spawn(cmd);

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

int exit_code = result.value();
```

## Capturing Output

```cpp
cmd.stdout_mode(vix::process::PipeMode::Pipe);
cmd.stderr_mode(vix::process::PipeMode::Pipe);

auto result = vix::process::output(cmd);

auto out = result.value();

std::cout << out.stdout_text;
std::cout << out.stderr_text;
```

## Pipeline (process → process)

```cpp
vix::process::Command producer("echo");
producer.arg("hello world");

vix::process::Command consumer("cat");

auto children = vix::process::pipeline::spawn(producer, consumer).value();

auto result = vix::process::pipeline::wait(children).value();

std::cout << result.first_exit_code;
std::cout << result.second_exit_code;
```

## Process Control

```cpp
vix::process::kill(child);       // force (SIGKILL / TerminateProcess)
vix::process::terminate(child);  // graceful (SIGTERM / best effort)
```

## Status Check

```cpp
auto running = vix::process::status(child);

if (running.value())
{
  // still running
}
```

## Async Usage

```cpp
auto child = co_await vix::process::async::spawn(ctx, cmd);

int code = co_await vix::process::async::wait(ctx, child);
```

Pipeline async:

```cpp
auto children =
  co_await vix::process::pipeline::async::spawn(ctx, p1, p2);

auto result =
  co_await vix::process::pipeline::async::wait(ctx, children);
```

## Error Handling

All operations return `Result<T>` or `Error`.

```cpp
auto result = vix::process::spawn(cmd);

if (!result)
{
  auto err = result.error();
}
```

No hidden exceptions unless explicitly using async wrappers.

## Important Design Choice

This module does not use a shell.

```cpp
Command("ls -la"); // WRONG
Command("ls").args({"-la"}); // CORRECT
```

If you need shell features (pipes, redirects, etc), you must explicitly call:

```cpp
Command("sh").arg("-c").arg("echo hello | sort");
```

This keeps behavior:

- predictable
- safe
- portable

## Examples

```text
examples/
|- basic_spawn.cpp
|- wait_process.cpp
|- capture_output.cpp
|- check_status.cpp
|- kill_process.cpp
|- terminate_process.cpp
|- pipeline_basic.cpp
|- pipeline_output.cpp
|- output_large.cpp
`- async/
   |- async_spawn_wait.cpp
   |- async_output.cpp
   |- async_cancel.cpp
   `- async_pipeline.cpp
```

## Current State

- API: stable
- Sync process: implemented
- Async layer: implemented
- Pipeline: implemented
- POSIX backend: in progress
- Windows backend: in progress

Some features may still return:

- `UnsupportedOperation`

## Design Notes

- `Command` is a builder, not a shell string
- Pipes are explicit (`PipeMode`)
- No implicit environment mutation
- Backend details hidden behind `Child`
- Same API across platforms

## Roadmap

- Full POSIX backend (`fork + exec + pipes`)
- Full Windows backend (`CreateProcess + pipes`)
- Streaming I/O (non-buffered pipes)
- Process groups & signals
- Timeout & cancellation
- High-performance async pipes

## License

MIT License
Copyright (c) 2026 Gaspard Kirira \
https://github.com/vixcpp/vix

