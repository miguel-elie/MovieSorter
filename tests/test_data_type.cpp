#include "catch.hpp"
#include "csv.hpp"
#include <string>

#define INT_IS_LONG_INT

using namespace csv;
using namespace csv::internals;

TEST_CASE( "Recognize Integers Properly", "[dtype_int]" ) {
    std::string a("1"), b(" 2018   "), c(" -69 ");
    long double out;

    REQUIRE(data_type(a, &out) ==  CSV_INT8);
    REQUIRE(out == 1);

    REQUIRE(data_type(b, &out) == CSV_INT16);
    REQUIRE(out == 2018);

    REQUIRE(data_type(c, &out) == CSV_INT8);
    REQUIRE(out == -69);
}

TEST_CASE( "Recognize Strings Properly", "[dtype_str]" ) {
    auto str = GENERATE(as<std::string> {}, "test", "999.999.9999", "510-123-4567", "510 123", "510 123 4567");

    SECTION("String Recognition") {
        REQUIRE(data_type(str) == CSV_STRING);
    }
}

TEST_CASE( "Recognize Null Properly", "[dtype_null]" ) {
    std::string null_str("");
    REQUIRE( data_type(null_str) ==  CSV_NULL );
}

TEST_CASE( "Recognize Floats Properly", "[dtype_float]" ) {
    std::string float_a("3.14"),
        float_b("       -3.14            "),
        e("2.71828");

    long double out;
    
    REQUIRE(data_type(float_a, &out) == CSV_DOUBLE);
    REQUIRE(is_equal(out, 3.14L));

    REQUIRE(data_type(float_b, &out) ==  CSV_DOUBLE);
    REQUIRE(is_equal(out, -3.14L));

    REQUIRE(data_type(e, &out) == CSV_DOUBLE);
    REQUIRE(is_equal(out, 2.71828L));
}

TEST_CASE("Integer Overflow", "[int_overflow]") {
    constexpr long double _INT_MAX = (long double)std::numeric_limits<int>::max();
    constexpr long double _LONG_MAX = (long double)std::numeric_limits<long int>::max();
    constexpr long double _LONG_LONG_MAX = (long double)std::numeric_limits<long long int>::max();

    std::string s;
    long double out;
    s = std::to_string((long long)_INT_MAX + 1);

    REQUIRE(data_type(s, &out) == CSV_INT64);
    REQUIRE(out == (long long)_INT_MAX + 1);
}

TEST_CASE( "Recognize Sub-Unit Double Values", "[regression_double]" ) {
    std::string s("0.15");
    long double out;
    REQUIRE(data_type(s, &out) == CSV_DOUBLE);
    REQUIRE(is_equal(out, 0.15L));
}

TEST_CASE( "Recognize Double Values", "[regression_double2]" ) {
    // Test converting double values back and forth
    long double out;
    std::string s;

    for (long double i = 0; i <= 2.0; i += 0.01) {
        s = std::to_string(i);
        REQUIRE(data_type(s, &out) == CSV_DOUBLE);
        REQUIRE(is_equal(out, i));
    }
}

TEST_CASE("Parse Scientific Notation", "[e_notation]") {
    // Test parsing e notation
    long double out;

    REQUIRE(data_type("2.17222E+02", &out) == CSV_DOUBLE);
    REQUIRE(is_equal(out, 217.222L));

    REQUIRE(data_type("4.55E+10", &out) == CSV_DOUBLE);
    REQUIRE(is_equal(out, 45500000000.0L));

    REQUIRE(data_type("4.55E+11", &out) == CSV_DOUBLE);
    REQUIRE(is_equal(out, 455000000000.0L));

    REQUIRE(data_type("4.55E-1", &out) == CSV_DOUBLE);
    REQUIRE(is_equal(out, 0.455L));

    REQUIRE(data_type("4.55E-5", &out) == CSV_DOUBLE);
    REQUIRE(is_equal(out, 0.0000455L));

    REQUIRE(data_type("4.55E-000000000005", &out) == CSV_DOUBLE);
    REQUIRE(is_equal(out, 0.0000455L));
}

TEST_CASE("Parse Different Flavors of Scientific Notation", "[sci_notation_diversity]") {
    auto number = GENERATE(as<std::string> {},
        "4.55e5", "4.55E5",
        "4.55E+5", "4.55e+5",
        "4.55E+05",
        "4.55e0000005", "4.55E0000005",
        "4.55e+0000005", "4.55E+0000005");

    SECTION("Recognize 455 thousand") {
        long double out;
        REQUIRE(data_type(number, &out) == CSV_DOUBLE);
        REQUIRE(is_equal(out, 455000.0L));
    }
}

TEST_CASE("Parse Scientific Notation Malformed", "[sci_notation]") {
    // Assert parsing butchered scientific notation won't cause a 
    // crash or any other weird side effects
    long double out;
    auto butchered = GENERATE(as<std::string>{},
        "4.55E000a",
        "4.55000x40",
        "4.55000E40E40");

    SECTION("Butchered Parsing Attempt") {
        REQUIRE(data_type(butchered, &out) == CSV_STRING);
    }
}