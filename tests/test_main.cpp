// This file's only job is to generate main() for the test binary.
// Catch2's single header does that automatically when this macro is
// defined in exactly one .cpp file before including it.
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
