#include <gtest/gtest.h>
#include "../../source/coverage/coverage_wrappers.hpp"

using namespace coverage_wrappers;

TEST(LinearAlgebraForwarders, CallVector3TriangleVector4Forwarders)
{
    // Call compiled forwarders that in turn call header noinline helpers.
    call_vector3_forwarders();
    call_triangle_forwarders();
    call_vector4_forwarders();

    SUCCEED();
}
