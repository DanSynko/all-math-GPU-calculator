export module test_expressions;

#ifdef __INTELLISENSE__
#include <array>
#include <vector>
#include <string>
#include <string_view>
#include <cstddef>
#else
import std.compat;
#endif

//import types;



export {
    //template <size_t N>
    //using TestExpressions = std::array<std::string_view, N>;

    //constexpr auto unknown_symbol_error_test = TestExpressions{
    //    "3#"sv,
    //    "#3"sv,
    //    "#3#"sv,
    //    "2 +w 2"sv,
    //    "2 + 2w>"sv
    //};

    // happy path
    std::vector<std::string> extreme_expressions = {
        "(((((2^3+3)^2-40)/5*3^2+7)^0.5-15)^3+2*(15-3*2)^4)^2/((((5*4-3^2)^2-100)/7+5)^3-999)",
        "((((2 ^ 2 * 2 + 3) * (2 * 2 * 2 + 3) - 40.1) / 5 * 3 * 3 + 7) * ((((2,2 * 2.3 * 2.4 + 3.33) * (2 * 2 * 2 + 3) - 40.500) / 5 * 3 * 3 + 7) - 15) - 15) / ((((5 * 4 - 3 * 3) * (5 * 4 - 3 * 3) - 100) / 7 + 5) * (((5 * 4.345 - 3 * 3) * (5.5 * 4 - 3 * 3) - 100) / 7 + 5) * (((5 * 4 - 3 ^ 3) * (5 * 4 - 3 * 3) - 100) / 7 + 5) - 999)"
    };


    // lexer
    std::vector<std::string> noinput_error = { "" };

    std::vector<std::string> unknown_symbol_error_test = {
        "3#",
        "#3",
        "#3#",
        "2 +w 2",
        "2 + 2w>"
    };

    std::vector<std::string> invalid_fp_error_test = {
        "2..2 + 2",
        "2 + 2..",
        "2.. + 2",
        ".2 + 2",
        "2 + 2."
    };

    // Pratt parser
    std::vector<std::string> parentheses_imbalance_error = {
        "((2 + 2) + 2",
        "(2 + 2)) + 2",
        "2 + 2)) + 2",
        "((2 + 2 + 2",
        ")2 + 2"
        "2 + 2)"
    };

    std::vector<std::string> unexpected_end_error = {
        "5 +",
        "2 + 2 -"
    };

    std::vector<std::string> invalid_prefix_operator_error = {
        "5-",
        "-5-",
        "%5",
        "%5%"
    };

    // ir optimizer(constant folding)
    std::vector<std::string> div_by_zero_error = {
        "5 / 0",
        "5 / ((2 * 22) - 44)"
    };

    std::vector<std::string> unsupported_complex_error = {
        "-5 ^ 0.5",
        "-23 ^ 0.1",
        "-41 ^ 5.2"
    };
}