include_directories(../src)

add_executable(LILC_TEST test.c)
target_link_libraries(LILC_TEST LILC_CORE)

add_test(NAME TestLilc COMMAND $<TARGET_FILE:LILC_TEST>)

# Copy non-compiled test input/output files to build dir
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/parser DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/lexer DESTINATION ${CMAKE_CURRENT_BINARY_DIR})