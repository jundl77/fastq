set(TARGET_NAME test_producer_app)

add_executable(${TARGET_NAME} test_producer.cc)
target_link_libraries(${TARGET_NAME} producer_lib)
