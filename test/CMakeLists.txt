include_directories(../src)
add_executable(LILC_TEST test.c)
target_link_libraries(LILC_TEST LILC_CORE)
add_test(NAME TestLilc COMMAND $<TARGET_FILE:LILC_TEST>)
# Copy non-compiled test input/output files to build dir
# TODO glob these or something so we don't have to specify every one
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lexer/add_2.lilc ${CMAKE_CURRENT_BINARY_DIR}/lexer/add_2.lilc COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lexer/add_2.lexout ${CMAKE_CURRENT_BINARY_DIR}/lexer/add_2.lexout COPYONLY)