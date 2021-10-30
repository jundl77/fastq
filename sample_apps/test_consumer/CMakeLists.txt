set(TARGET_NAME test_consumer_app)

add_executable(${TARGET_NAME} test_consumer.cc)
target_link_libraries(${TARGET_NAME} consumer_lib)
