# GenerateEmbeddedConfig.cmake
# Usage: cmake -DINPUT_JSON=<path> -DOUTPUT_HEADER=<path> -P GenerateEmbeddedConfig.cmake
if(NOT DEFINED INPUT_JSON)
  message(FATAL_ERROR "INPUT_JSON not defined")
endif()
if(NOT DEFINED OUTPUT_HEADER)
  message(FATAL_ERROR "OUTPUT_HEADER not defined")
endif()

file(READ "${INPUT_JSON}" _json_content)

# Note: UTF-8 BOM (EF BB BF) if present is preserved in the raw string.
# It is stripped at runtime in config_struct.cpp before json::parse().
# No CMake-side stripping needed (CMake lacks \x escape in this context).

# Choose a raw string delimiter that does not appear in the JSON.
# MSVC raw string delimiter must be <= 16 chars (C3512).
set(_delim "COYOTE")
set(_delim_suffix 0)
while(_json_content MATCHES "${_delim}")
  math(EXPR _delim_suffix "${_delim_suffix} + 1")
  set(_delim "COYOTE${_delim_suffix}")
  if(_delim_suffix GREATER 9999)
    message(FATAL_ERROR "Unable to find suitable raw string delimiter")
  endif()
  string(LENGTH "${_delim}" _delim_len)
  if(_delim_len GREATER 16)
    message(FATAL_ERROR "Delimiter exceeds 16 chars: ${_delim}")
  endif()
endwhile()

get_filename_component(_out_dir "${OUTPUT_HEADER}" DIRECTORY)
file(MAKE_DIRECTORY "${_out_dir}")

# Write header atomically via temp file then rename to avoid partial writes.
set(_tmp "${OUTPUT_HEADER}.tmp")
file(WRITE "${_tmp}" "#pragma once\n")
file(APPEND "${_tmp}" "// Auto-generated from ${INPUT_JSON} — do not edit.\n")
file(APPEND "${_tmp}" "#include <string_view>\n\n")
file(APPEND "${_tmp}" "inline constexpr std::string_view kEmbeddedConfigJson = R\"${_delim}(\n")
file(APPEND "${_tmp}" "${_json_content}\n")
file(APPEND "${_tmp}" ")${_delim}\";\n")

file(RENAME "${_tmp}" "${OUTPUT_HEADER}")
string(LENGTH "${_json_content}" _json_len)
message(STATUS "Generated ${OUTPUT_HEADER} (delimiter=${_delim}, ${_json_len} bytes)")
