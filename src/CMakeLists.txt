add_executable(test_server test_server.c)

add_executable(mc_hoap3_tcp mc_hoap3_tcp.cpp)
target_link_libraries(mc_hoap3_tcp PUBLIC mc_rtc::mc_control)
target_link_libraries(mc_hoap3_tcp PUBLIC Boost::program_options Boost::system)

install(
  TARGETS mc_hoap3_tcp
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
