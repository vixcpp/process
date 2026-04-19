/**
 *
 *  @file process_async_tests.cpp
 *  @author Gaspard Kirira
 *
 *  Vix.cpp
 *
 */

#include <chrono>
#include <string>
#include <system_error>
#include <utility>

#include <vix/tests/tests.hpp>
#include <vix/async/async.hpp>

#include <vix/process/Command.hpp>
#include <vix/process/async/OutputAsync.hpp>
#include <vix/process/async/SpawnAsync.hpp>
#include <vix/process/async/WaitAsync.hpp>
#include <vix/process/pipeline/PipelineAsync.hpp>
#include <vix/process/Kill.hpp>
#include <vix/process/Wait.hpp>

using namespace vix::tests;

namespace
{
  template <typename Fn>
  void run_async_test(Fn &&fn)
  {
    vix::async::core::io_context ctx;
    std::move(fn)(ctx).start(ctx.get_scheduler());
    ctx.run();
  }

  vix::async::core::task<void> stop_later(
      vix::async::core::io_context &ctx,
      std::chrono::milliseconds delay)
  {
    co_await ctx.timers().sleep_for(delay);
    ctx.stop();
  }
} // namespace

int main()
{
  auto &registry = TestRegistry::instance();

  registry.add(TestCase("async spawn: empty command should fail", []
                        { run_async_test(
                              [](vix::async::core::io_context &ctx) -> vix::async::core::task<void>
                              {
                                bool failed = false;

                                try
                                {
                                  vix::process::Command cmd("");
                                  (void)co_await vix::process::async::spawn(ctx, std::move(cmd));
                                }
                                catch (...)
                                {
                                  failed = true;
                                }

                                Assert::is_true(failed);
                                ctx.stop();
                                co_return;
                              }); }));

  registry.add(TestCase("async output: empty command should fail", []
                        { run_async_test(
                              [](vix::async::core::io_context &ctx) -> vix::async::core::task<void>
                              {
                                bool failed = false;

                                try
                                {
                                  vix::process::Command cmd("");
                                  (void)co_await vix::process::async::output(ctx, std::move(cmd));
                                }
                                catch (...)
                                {
                                  failed = true;
                                }

                                Assert::is_true(failed);
                                ctx.stop();
                                co_return;
                              }); }));

  registry.add(TestCase("async wait: invalid child should fail", []
                        { run_async_test(
                              [](vix::async::core::io_context &ctx) -> vix::async::core::task<void>
                              {
                                bool failed = false;

                                try
                                {
                                  vix::process::Child child;
                                  (void)co_await vix::process::async::wait(ctx, child);
                                }
                                catch (...)
                                {
                                  failed = true;
                                }

                                Assert::is_true(failed);
                                ctx.stop();
                                co_return;
                              }); }));

  registry.add(TestCase("async pipeline spawn: empty first command should fail", []
                        { run_async_test(
                              [](vix::async::core::io_context &ctx) -> vix::async::core::task<void>
                              {
                                bool failed = false;

                                try
                                {
                                  vix::process::Command first("");
                                  vix::process::Command second("cat");

                                  (void)co_await vix::process::pipeline::async::spawn(
                                      ctx,
                                      std::move(first),
                                      std::move(second));
                                }
                                catch (...)
                                {
                                  failed = true;
                                }

                                Assert::is_true(failed);
                                ctx.stop();
                                co_return;
                              }); }));

  registry.add(TestCase("async pipeline wait: invalid children should fail", []
                        { run_async_test(
                              [](vix::async::core::io_context &ctx) -> vix::async::core::task<void>
                              {
                                bool failed = false;

                                try
                                {
                                  vix::process::pipeline::PipelineChildren children;
                                  (void)co_await vix::process::pipeline::async::wait(ctx, children);
                                }
                                catch (...)
                                {
                                  failed = true;
                                }

                                Assert::is_true(failed);
                                ctx.stop();
                                co_return;
                              }); }));

#ifndef _WIN32
  registry.add(TestCase("async output: echo should capture stdout", []
                        { run_async_test(
                              [](vix::async::core::io_context &ctx) -> vix::async::core::task<void>
                              {
                                vix::process::Command cmd("echo");
                                cmd.arg("hello-from-vix-async-process-test");

                                auto result =
                                    co_await vix::process::async::output(ctx, std::move(cmd));

                                Assert::equal(result.exit_code, 0);
                                Assert::is_true(
                                    result.stdout_text.find("hello-from-vix-async-process-test") != std::string::npos);

                                ctx.stop();
                                co_return;
                              }); }));

  registry.add(TestCase("async spawn/wait: true should exit with code 0", []
                        { run_async_test(
                              [](vix::async::core::io_context &ctx) -> vix::async::core::task<void>
                              {
                                vix::process::Command cmd("true");

                                auto child =
                                    co_await vix::process::async::spawn(ctx, std::move(cmd));

                                int exit_code =
                                    co_await vix::process::async::wait(ctx, child);

                                Assert::equal(exit_code, 0);

                                ctx.stop();
                                co_return;
                              }); }));

  registry.add(TestCase("async pipeline: echo to cat should succeed", []
                        { run_async_test(
                              [](vix::async::core::io_context &ctx) -> vix::async::core::task<void>
                              {
                                vix::process::Command producer("echo");
                                producer.arg("hello-pipeline");

                                vix::process::Command consumer("cat");

                                auto children =
                                    co_await vix::process::pipeline::async::spawn(
                                        ctx,
                                        std::move(producer),
                                        std::move(consumer));

                                auto result =
                                    co_await vix::process::pipeline::async::wait(ctx, children);

                                Assert::equal(result.first_exit_code, 0);
                                Assert::equal(result.second_exit_code, 0);

                                ctx.stop();
                                co_return;
                              }); }));

  registry.add(TestCase("async cancel: wait should observe cancellation", []
                        { run_async_test(
                              [](vix::async::core::io_context &ctx) -> vix::async::core::task<void>
                              {
                                vix::async::core::cancel_source cancel;

                                vix::process::Command cmd("sleep");
                                cmd.arg("5");

                                auto child =
                                    co_await vix::process::async::spawn(ctx, std::move(cmd));

                                ctx.timers().after(
                                    std::chrono::milliseconds(100),
                                    [&cancel]()
                                    {
                                      cancel.request_cancel();
                                    });

                                bool cancelled = false;

                                try
                                {
                                  (void)co_await vix::process::async::wait(
                                      ctx,
                                      child,
                                      cancel.token(),
                                      std::chrono::milliseconds(20));
                                }
                                catch (const std::system_error &)
                                {
                                  cancelled = true;
                                }
                                catch (...)
                                {
                                }

                                Assert::is_true(cancelled);

                                (void)vix::process::kill(child);
                                (void)vix::process::wait(child);

                                ctx.stop();
                                co_return;
                              }); }));
#else
  registry.add(TestCase("async output: cmd echo should capture stdout", []
                        { run_async_test(
                              [](vix::async::core::io_context &ctx) -> vix::async::core::task<void>
                              {
                                vix::process::Command cmd("cmd");
                                cmd.arg("/C");
                                cmd.arg("echo hello-from-vix-async-process-test");

                                auto result =
                                    co_await vix::process::async::output(ctx, std::move(cmd));

                                Assert::equal(result.exit_code, 0);
                                Assert::is_true(
                                    result.stdout_text.find("hello-from-vix-async-process-test") != std::string::npos);

                                ctx.stop();
                                co_return;
                              }); }));

  registry.add(TestCase("async spawn/wait: cmd exit 0 should succeed", []
                        { run_async_test(
                              [](vix::async::core::io_context &ctx) -> vix::async::core::task<void>
                              {
                                vix::process::Command cmd("cmd");
                                cmd.arg("/C");
                                cmd.arg("exit 0");

                                auto child =
                                    co_await vix::process::async::spawn(ctx, std::move(cmd));

                                int exit_code =
                                    co_await vix::process::async::wait(ctx, child);

                                Assert::equal(exit_code, 0);

                                ctx.stop();
                                co_return;
                              }); }));

  registry.add(TestCase("async pipeline: cmd echo to sort should succeed", []
                        { run_async_test(
                              [](vix::async::core::io_context &ctx) -> vix::async::core::task<void>
                              {
                                vix::process::Command producer("cmd");
                                producer.arg("/C");
                                producer.arg("echo hello-pipeline");

                                vix::process::Command consumer("sort");

                                auto children =
                                    co_await vix::process::pipeline::async::spawn(
                                        ctx,
                                        std::move(producer),
                                        std::move(consumer));

                                auto result =
                                    co_await vix::process::pipeline::async::wait(ctx, children);

                                Assert::equal(result.first_exit_code, 0);
                                Assert::equal(result.second_exit_code, 0);

                                ctx.stop();
                                co_return;
                              }); }));

  registry.add(TestCase("async cancel: wait should observe cancellation", []
                        { run_async_test(
                              [](vix::async::core::io_context &ctx) -> vix::async::core::task<void>
                              {
                                vix::async::core::cancel_source cancel;

                                vix::process::Command cmd("ping");
                                cmd.arg("127.0.0.1");
                                cmd.arg("-n");
                                cmd.arg("6");

                                auto child =
                                    co_await vix::process::async::spawn(ctx, std::move(cmd));

                                ctx.timers().after(
                                    std::chrono::milliseconds(100),
                                    [&cancel]()
                                    {
                                      cancel.request_cancel();
                                    });

                                bool cancelled = false;

                                try
                                {
                                  (void)co_await vix::process::async::wait(
                                      ctx,
                                      child,
                                      cancel.token(),
                                      std::chrono::milliseconds(20));
                                }
                                catch (const std::system_error &)
                                {
                                  cancelled = true;
                                }
                                catch (...)
                                {
                                }

                                Assert::is_true(cancelled);

                                (void)vix::process::kill(child);
                                (void)vix::process::wait(child);

                                ctx.stop();
                                co_return;
                              }); }));
#endif

  return TestRunner::run_all_and_exit();
}
