#include "catch.hpp"

#include <gmp.h>

TEST_CASE( "Checking GMP version", "[GMP]" ) {
    REQUIRE(__GNU_MP_VERSION == 6);
    REQUIRE(__GNU_MP_VERSION_MINOR == 1);
    REQUIRE(__GNU_MP_VERSION_PATCHLEVEL == 2); 
}
