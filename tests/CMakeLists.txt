add_executable(simple_cad_tests test_main.cpp)
target_link_libraries(simple_cad_tests PRIVATE simple_cad_core)
target_compile_definitions(simple_cad_tests PRIVATE
  SIMPLECAD_EXAMPLE_XY_PATH="${CMAKE_SOURCE_DIR}/example.xy"
)
add_test(NAME simple_cad_tests COMMAND simple_cad_tests)
