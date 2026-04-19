/**
 *
 *  @file process_tests.cpp
 *  @author Gaspard Kirira
 *
 *  Vix.cpp
 *
 */

#include <string>

#include <vix/tests/tests.hpp>

#include <vix/process/Child.hpp>
#include <vix/process/Command.hpp>
#include <vix/process/Kill.hpp>
#include <vix/process/Output.hpp>
#include <vix/process/ProcessResult.hpp>
#include <vix/process/Spawn.hpp>
#include <vix/process/Status.hpp>
#include <vix/process/Terminate.hpp>
#include <vix/process/Wait.hpp>
#include <vix/process/pipeline/Pipeline.hpp>

using namespace vix::tests;
using namespace vix::process;

int main()
{
  auto &registry = TestRegistry::instance();

  registry.add(TestCase("child: default child should be invalid", []
                        {
                          Child child;
                          Assert::is_true(!child.valid());
                          Assert::is_true(!static_cast<bool>(child)); }));

  registry.add(TestCase("child: child with pid should be valid", []
                        {
                          Child child(static_cast<ProcessId>(42));
                          Assert::is_true(child.valid());
                          Assert::is_true(static_cast<bool>(child));
                          Assert::equal(child.id(), static_cast<ProcessId>(42)); }));

  registry.add(TestCase("process_output: success reflects exit code", []
                        {
                          ProcessOutput ok;
                          ok.exit_code = 0;
                          Assert::is_true(ok.success());

                          ProcessOutput fail;
                          fail.exit_code = 1;
                          Assert::is_true(!fail.success()); }));

  registry.add(TestCase("pipeline_result: success reflects both exit codes", []
                        {
                          pipeline::PipelineResult ok;
                          ok.first_exit_code = 0;
                          ok.second_exit_code = 0;
                          Assert::is_true(ok.success());

                          pipeline::PipelineResult fail_first;
                          fail_first.first_exit_code = 1;
                          fail_first.second_exit_code = 0;
                          Assert::is_true(!fail_first.success());

                          pipeline::PipelineResult fail_second;
                          fail_second.first_exit_code = 0;
                          fail_second.second_exit_code = 2;
                          Assert::is_true(!fail_second.success()); }));

  registry.add(TestCase("pipeline_children: valid requires both children", []
                        {
                          pipeline::PipelineChildren children;
                          Assert::is_true(!children.valid());

                          children.first = Child(static_cast<ProcessId>(1));
                          Assert::is_true(!children.valid());

                          children.second = Child(static_cast<ProcessId>(2));
                          Assert::is_true(children.valid()); }));

  registry.add(TestCase("spawn: empty command should fail", []
                        {
                          Command cmd("");

                          auto result = spawn(cmd);

                          Assert::is_true(!result); }));

  registry.add(TestCase("output: empty command should fail", []
                        {
                          Command cmd("");

                          auto result = output(cmd);

                          Assert::is_true(!result); }));

  registry.add(TestCase("status: invalid child should fail", []
                        {
                          Child child;

                          auto result = status(child);

                          Assert::is_true(!result); }));

  registry.add(TestCase("wait: invalid child should fail", []
                        {
                          Child child;

                          auto result = wait(child);

                          Assert::is_true(!result); }));

  registry.add(TestCase("kill: invalid child should fail", []
                        {
                          Child child;

                          auto err = kill(child);

                          Assert::is_true(err.has_error()); }));

  registry.add(TestCase("terminate: invalid child should fail", []
                        {
                          Child child;

                          auto err = terminate(child);

                          Assert::is_true(err.has_error()); }));

  registry.add(TestCase("pipeline spawn: empty first command should fail", []
                        {
                          Command first("");
                          Command second("cat");

                          auto result = pipeline::spawn(first, second);

                          Assert::is_true(!result); }));

  registry.add(TestCase("pipeline spawn: empty second command should fail", []
                        {
                          Command first("echo");
                          Command second("");

                          auto result = pipeline::spawn(first, second);

                          Assert::is_true(!result); }));

  registry.add(TestCase("pipeline wait: invalid first child should fail", []
                        {
                          pipeline::PipelineChildren children;
                          children.second = Child(static_cast<ProcessId>(2));

                          auto result = pipeline::wait(children);

                          Assert::is_true(!result); }));

  registry.add(TestCase("pipeline wait: invalid second child should fail", []
                        {
                          pipeline::PipelineChildren children;
                          children.first = Child(static_cast<ProcessId>(1));

                          auto result = pipeline::wait(children);

                          Assert::is_true(!result); }));

#ifndef _WIN32
  registry.add(TestCase("output: echo should capture stdout", []
                        {
                          Command cmd("echo");
                          cmd.arg("hello-from-vix-process-test");

                          auto result = output(cmd);

                          Assert::is_true(static_cast<bool>(result));
                          Assert::equal(result.value().exit_code, 0);
                          Assert::is_true(
                              result.value().stdout_text.find("hello-from-vix-process-test") != std::string::npos); }));

  registry.add(TestCase("spawn/wait: true should exit with code 0", []
                        {
                          Command cmd("true");

                          auto spawned = spawn(cmd);

                          Assert::is_true(static_cast<bool>(spawned));

                          auto waited = wait(spawned.value());

                          Assert::is_true(static_cast<bool>(waited));
                          Assert::equal(waited.value(), 0); }));

  registry.add(TestCase("pipeline: echo to cat should succeed", []
                        {
                          Command producer("echo");
                          producer.arg("hello-pipeline");

                          Command consumer("cat");

                          auto spawned = pipeline::spawn(producer, consumer);

                          Assert::is_true(static_cast<bool>(spawned));

                          auto waited = pipeline::wait(spawned.value());

                          Assert::is_true(static_cast<bool>(waited));
                          Assert::equal(waited.value().first_exit_code, 0);
                          Assert::equal(waited.value().second_exit_code, 0); }));
#else
  registry.add(TestCase("output: cmd echo should capture stdout", []
                        {
                          Command cmd("cmd");
                          cmd.arg("/C");
                          cmd.arg("echo hello-from-vix-process-test");

                          auto result = output(cmd);

                          Assert::is_true(static_cast<bool>(result));
                          Assert::equal(result.value().exit_code, 0);
                          Assert::is_true(
                              result.value().stdout_text.find("hello-from-vix-process-test") != std::string::npos); }));

  registry.add(TestCase("spawn/wait: cmd exit 0 should succeed", []
                        {
                          Command cmd("cmd");
                          cmd.arg("/C");
                          cmd.arg("exit 0");

                          auto spawned = spawn(cmd);

                          Assert::is_true(static_cast<bool>(spawned));

                          auto waited = wait(spawned.value());

                          Assert::is_true(static_cast<bool>(waited));
                          Assert::equal(waited.value(), 0); }));

  registry.add(TestCase("pipeline: cmd echo to sort should succeed", []
                        {
                          Command producer("cmd");
                          producer.arg("/C");
                          producer.arg("echo hello-pipeline");

                          Command consumer("sort");

                          auto spawned = pipeline::spawn(producer, consumer);

                          Assert::is_true(static_cast<bool>(spawned));

                          auto waited = pipeline::wait(spawned.value());

                          Assert::is_true(static_cast<bool>(waited));
                          Assert::equal(waited.value().first_exit_code, 0);
                          Assert::equal(waited.value().second_exit_code, 0); }));
#endif

  return TestRunner::run_all_and_exit();
}
