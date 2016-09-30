CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(NAME ${PROJECT_NAME})

SET(CMAKE_CXX_FLAGS "-std=c++11 -O3 -Wall -Werror -pedantic-errors")

IF(DEFINED EMSCRIPTEN)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-warn-absolute-paths -s TOTAL_MEMORY=67108864 -s FORCE_ALIGNED_MEMORY=1 -s EXPORTED_FUNCTIONS=\"['_sys_load_image', '_sys_run', '_sys_keypress', '_sys_can_read', '_sys_read', '_main']\"")
  #--preload-file ${CMAKE_SOURCE_DIR}/linux@/")
ENDIF()

# Use googletest.
INCLUDE_DIRECTORIES(3rdparty/gtest/include)
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} 3rdparty/gtest)
SET(gtest_disable_pthreads 1)
ADD_SUBDIRECTORY(3rdparty)

INCLUDE_DIRECTORIES(..)

SET(SOURCES
  bus.cc
  cpu.cc
  mmu.cc
  ram.cc
  system.cc
  uart.cc
)

SET(TEST_SOURCES
  assembler.cc
  cpu_test.cc
  mmu_test.cc
  ram_test.cc
)

SET(LIBS
)

SET(TEST_LIBS
  gtest
)

# Optionally use google C++ style linter cpplint.py.
IF(DEFINED LINT)
FILE(DOWNLOAD
  http://google-styleguide.googlecode.com/svn/trunk/cpplint/cpplint.py
  ${CMAKE_BINARY_DIR}/cpplint.py
)

ADD_CUSTOM_TARGET(Lint ALL ${PYTHON_EXECUTABLE} ${CMAKE_BINARY_DIR}/cpplint.py
  --root=${CMAKE_SOURCE_DIR}
  src/*.h src/*.cc
  src/*/*.h src/*/*.cc
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
UNSET(LINT)
ENDIF()

# Build targets.
IF(DEFINED EMSCRIPTEN)
  # Main JavaScript file.
  ADD_EXECUTABLE(${NAME}.js ${SOURCES} emscripten.cc)
  TARGET_LINK_LIBRARIES(${NAME}.js ${LIBS})

  # Main JavaScript html.
  ADD_EXECUTABLE(${NAME}-xtest.html ${SOURCES} main_web.cc)

  # Test JavaScript file.
  #ADD_EXECUTABLE(${NAME}-test.js ${SOURCES} ${TEST_SOURCES} test_main.cc)
  #TARGET_LINK_LIBRARIES(${NAME}-test.js ${LIBS} ${TEST_LIBS})
  #ADD_TEST(${NAME}-test nodejs ${NAME}-test.js)
ELSE()
  # Main executable.
  ADD_EXECUTABLE(${NAME} ${SOURCES} main.cc)

  # Test executable.
  ADD_EXECUTABLE(${NAME}-test ${SOURCES} ${TEST_SOURCES} test_main.cc)
  TARGET_LINK_LIBRARIES(${NAME}-test ${LIBS} ${TEST_LIBS})
  ADD_TEST(${NAME}-test ${NAME}-test)
ENDIF()

