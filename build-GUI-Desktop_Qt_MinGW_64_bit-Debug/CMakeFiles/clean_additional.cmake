# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\Prueba3_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\Prueba3_autogen.dir\\ParseCache.txt"
  "Prueba3_autogen"
  )
endif()
