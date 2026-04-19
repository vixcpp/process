/**
 *
 *  @file process_tests.cpp
 *  @author Gaspard Kirira
 *
 *  Vix.cpp
 *
 */

#include <vix/tests/tests.hpp>

#include <vix/process/Spawn.hpp>
#include <vix/process/Status.hpp>
#include <vix/process/Wait.hpp>
#include <vix/process/Output.hpp>
#include <vix/process/Kill.hpp>
#include <vix/process/Terminate.hpp>
#include <vix/process/Command.hpp>
#include <vix/process/Child.hpp>

using namespace vix::tests;
using namespace vix::process;

int main()
{
  auto &registry = TestRegistry::instance();

  // Spawn - invalid command
  registry.add(TestCase("spawn: empty command should fail", []
                        {
  Command cmd("");

  auto result = spawn(cmd);

  Assert::is_true(!result); }));

  // Output - invalid command
  registry.add(TestCase("output: empty command should fail", []
                        {
  Command cmd("");

  auto result = output(cmd);

  Assert::is_true(!result); }));

  // Status - invalid child
  registry.add(TestCase("status: invalid child should fail", []
                        {
    Child child; // invalid

    auto result = status(child);

    Assert::is_true(!result); }));

  // Wait - invalid child
  registry.add(TestCase("wait: invalid child should fail", []
                        {
    Child child;

    auto result = wait(child);

    Assert::is_true(!result); }));

  // Kill - invalid child
  registry.add(TestCase("kill: invalid child should fail", []
                        {
    Child child;

    auto err = kill(child);

    Assert::is_true(err.has_error()); }));

  // Terminate - invalid child
  registry.add(TestCase("terminate: invalid child should fail", []
                        {
    Child child;

    auto err = terminate(child);

    Assert::is_true(err.has_error()); }));

  // Spawn - not implemented
  registry.add(TestCase("spawn: not implemented yet", []
                        {
    Command cmd("echo");

    auto result = spawn(cmd);

    Assert::is_true(!result); }));

  // Output - not implemented
  registry.add(TestCase("output: not implemented yet", []
                        {
    Command cmd("echo");

    auto result = output(cmd);

    Assert::is_true(!result); }));

  return TestRunner::run_all_and_exit();
}
