  # HSD generated
  file (TIMESTAMP "${CMAKE_ARGV3}" curr_filetime)
  if (EXISTS "${CMAKE_BINARY_DIR}/ark-filetime.txt")
    file (READ "${CMAKE_BINARY_DIR}/ark-filetime.txt" prev_filetime)
    if (curr_filetime STREQUAL prev_filetime)
      return ()
    endif ()
  endif ()

  message (STATUS "Inflating resources for testsuite execution")
  execute_process (COMMAND "${CMAKE_COMMAND}" -E tar xf "${CMAKE_ARGV3}"
    WORKING_DIRECTORY "${CMAKE_ARGV4}"
  )
  file (WRITE "${CMAKE_BINARY_DIR}/ark-filetime.txt" "${curr_filetime}")
