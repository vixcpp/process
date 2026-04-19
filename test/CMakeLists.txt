add_executable(vix_process_tests
  process_tests.cpp
)

target_link_libraries(vix_process_tests
  PRIVATE
    vix::process
    vix::tests
)

add_test(NAME vix_process_tests COMMAND vix_process_tests)
