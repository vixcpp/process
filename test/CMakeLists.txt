function(vix_process_enable_sanitizers target)
  if (VIX_ENABLE_SANITIZERS AND NOT MSVC)
    target_compile_options(${target}
      PRIVATE
        -fno-omit-frame-pointer
        -fsanitize=address,undefined
    )

    target_link_options(${target}
      PRIVATE
        -fsanitize=address,undefined
    )
  endif()
endfunction()

add_executable(vix_process_tests
  process_tests.cpp
)

target_link_libraries(vix_process_tests
  PRIVATE
    vix::process
    vix::tests
)

vix_process_enable_sanitizers(vix_process_tests)

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

vix_process_enable_sanitizers(vix_process_async_tests)

add_test(NAME vix_process_async_tests COMMAND vix_process_async_tests)
