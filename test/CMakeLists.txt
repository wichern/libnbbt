file(GLOB test_srcs "*.cpp")
add_executable("${PROJECT_NAME}-test" ${test_srcs})
target_link_libraries("${PROJECT_NAME}-test" ${PROJECT_NAME} gtest_main)
