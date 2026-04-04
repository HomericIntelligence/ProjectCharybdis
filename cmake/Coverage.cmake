if(${PROJECT_NAME}_ENABLE_COVERAGE)
  if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|.*Clang")
    message(WARNING "Coverage is only supported with GCC or Clang.")
    return()
  endif()

  # Add --coverage to compile and link flags for the whole project
  add_compile_options(--coverage -O0 -fno-inline)
  add_link_options(--coverage)
endif()
