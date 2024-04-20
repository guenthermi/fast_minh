include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  test_minh
  test_minh.cpp
)

target_include_directories(test_minh PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../fast_minh)

target_link_libraries(
  test_minh
  GTest::gtest_main
  fast_minh
)

include(GoogleTest)
gtest_discover_tests(test_minh)
