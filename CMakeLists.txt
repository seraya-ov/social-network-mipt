cmake_minimum_required(VERSION 3.2)

project(hl_event_server C CXX)

SET (EXAMPLE_BINARY "social_network")

# list(APPEND CMAKE_PREFIX_PATH "/usr/lib")

find_package(OpenSSL)
find_package(GTest REQUIRED)
find_package(Threads)
find_package(ZLIB)
find_package(Poco REQUIRED COMPONENTS Foundation Util Net XML JSON Crypto NetSSL)
find_package(mstch)
find_package(Boost)


if(NOT ${Poco_FOUND})
    message(FATAL_ERROR "Poco C++ Libraries not found.")
endif()

# include_directories(${Poco_INCLUDE_DIRS})
# include_directories(${GTEST_INCLUDE_DIRS})
# include_directories("/opt/homebrew/include")
# include_directories(${Boost_INCLUDE_DIR})
# link_directories("/usr/local/lib")
# link_directories("/opt/homebrew/lib")

include_directories(${Poco_INCLUDE_DIRS})
include_directories(${GTEST_INCLUDE_DIRS})
include_directories("/usr/include/mysql")
include_directories("/usr/local/include/mysql")
include_directories(${Boost_INCLUDE_DIR})
link_directories("/usr/lib")

add_executable(${EXAMPLE_BINARY} main.cpp 
                                 config/config.cpp 
                                 database/database.cpp
                                 database/user.cpp
                                 database/message.cpp)


target_include_directories(${EXAMPLE_BINARY} PRIVATE "${CMAKE_BINARY_DIR}")
target_compile_options(${EXAMPLE_BINARY} PRIVATE -Wall -Wextra -pedantic -Werror )

target_link_libraries(${EXAMPLE_BINARY} PRIVATE 
                             ${CMAKE_THREAD_LIBS_INIT} 
                             ${Poco_LIBRARIES}
                             ${Boost_LIBRARIES}
                             "PocoData"
                             "PocoDataMySQL"
                             "mysqlclient"
                             ZLIB::ZLIB
                             mstch::mstch)


set_target_properties(${EXAMPLE_BINARY} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${EXAMPLE_BINARY} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

install(TARGETS ${EXAMPLE_BINARY} RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT serayaov@gmail.com)
include(CPack)