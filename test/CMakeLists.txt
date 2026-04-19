add_executable(vix_process_tests
  process_tests.cpp
)

target_link_libraries(vix_process_tests
  PRIVATE
    vix::process
    vix::tests
)

add_test(NAME vix_process_tests COMMAND vix_process_tests)


# -------------------------------
# Async tests
# -------------------------------

add_executable(vix_process_async_tests
  process_async_tests.cpp
)

target_link_libraries(vix_process_async_tests
  PRIVATE
    vix::process
    vix::async
    vix::tests
)

add_test(NAME vix_process_async_tests COMMAND vix_process_async_tests)
