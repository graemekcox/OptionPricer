# CMakeList.txt : CMake project for OptionPricer, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("OptionPricer")

# Add source to this project's executable.

# TODO: Add tests and install targets if needed.
find_package(cpprestsdk CONFIG REQUIRED)

add_executable (OptionPricer "OptionPricer.cpp" "OptionPricer.h" "includes/stock_types.h" "includes/option_funcs.h")


target_link_libraries(OptionPricer PRIVATE cpprestsdk::cpprest cpprestsdk::cpprestsdk_zlib_internal cpprestsdk::cpprestsdk_brotli_internal)