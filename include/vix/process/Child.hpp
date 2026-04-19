/**
 *
 *  @file Child.hpp
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
#ifndef VIX_PROCESS_CHILD_HPP
#define VIX_PROCESS_CHILD_HPP

#include <memory>
#include <utility>

#include <vix/process/ProcessId.hpp>

namespace vix::process
{

  /**
   * @class Child
   * @brief Represents a spawned child process.
   *
   * Child is a lightweight handle storing:
   * - a stable public process id
   * - an optional opaque backend handle used internally by platform code
   *
   * Design goals:
   * - simple public API
   * - explicit validity
   * - portable abstraction across POSIX and Windows
   * - backend details hidden from module users
   */
  class Child
  {
  public:
    /**
     * @brief Construct an invalid child handle.
     */
    Child() = default;

    /**
     * @brief Construct a child from a process id.
     *
     * @param id Process identifier.
     */
    explicit Child(ProcessId id) noexcept
        : id_(id)
    {
    }

    /**
     * @brief Construct a child from a process id and opaque backend handle.
     *
     * @param id Process identifier.
     * @param backend Opaque backend state shared with platform code.
     */
    Child(ProcessId id, std::shared_ptr<void> backend) noexcept
        : id_(id),
          backend_(std::move(backend))
    {
    }

    /**
     * @brief Get the process id.
     */
    [[nodiscard]] ProcessId id() const noexcept
    {
      return id_;
    }

    /**
     * @brief Return true if this child handle is valid.
     *
     * A child is considered valid when it has a non-zero process id.
     */
    [[nodiscard]] bool valid() const noexcept
    {
      return id_ != 0;
    }

    /**
     * @brief Boolean conversion for convenience.
     *
     * Returns true when the child handle is valid.
     */
    [[nodiscard]] explicit operator bool() const noexcept
    {
      return valid();
    }

    /**
     * @brief Return true if a backend handle is attached.
     *
     * This is mainly useful for internal platform operations.
     */
    [[nodiscard]] bool has_backend() const noexcept
    {
      return static_cast<bool>(backend_);
    }

    /**
     * @brief Access the opaque backend handle.
     *
     * This API is intended for internal process backend code.
     */
    [[nodiscard]] const std::shared_ptr<void> &backend() const noexcept
    {
      return backend_;
    }

    /**
     * @brief Compare two child handles.
     */
    [[nodiscard]] bool operator==(const Child &other) const noexcept
    {
      return id_ == other.id_;
    }

    /**
     * @brief Compare two child handles.
     */
    [[nodiscard]] bool operator!=(const Child &other) const noexcept
    {
      return !(*this == other);
    }

  private:
    ProcessId id_{0};
    std::shared_ptr<void> backend_{};
  };

} // namespace vix::process

#endif // VIX_PROCESS_CHILD_HPP
