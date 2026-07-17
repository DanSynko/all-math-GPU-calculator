#include <gtest/gtest.h>

#ifdef __INTELLISENSE__
#include <string>
#include <vector>
#else
import std;
#endif

import types;
import lexer;
import pratt_parser;
import ir_generator;
import ir_optimizer;

import cuda_backend;

import test_expressions;




//struct ErrorTestCase {
//    
//}
//
//std::vector<ErrorTestCase> errors = {
//    
//};


class HappyPathParamTest : public testing::TestWithParam<std::vector<std::string>> {};

TEST_P(HappyPathParamTest, CheckHappyPath) {
    std::vector<std::string> const& current_input = GetParam();

    for (const auto& cheee : current_input) {
        Lexer lexer(cheee);
        auto tokens = lexer.tokenize();

        ASSERT_TRUE(tokens.has_value());

        //PrattParser parser(tokens.value());
        //auto ast = parser.parse();
        //ASSERT_TRUE(ast.has_value());
    }
}
INSTANTIATE_TEST_SUITE_P(
    HappyPathSuite,
    HappyPathParamTest,
    testing::Values(extreme_expressions)
);




//class ErrorPathParamTest : public testing::TestWithParam<std::vector<std::string>> {};
//
//TEST_P(ErrorPathParamTest, ErrorPathCheck) {
//
//}
//INSTANTIATE_TEST_SUITE_P(
//    ErrorPathSuite,
//    ErrorPathParamTest,
//    //testing::Values(extreme_expressions)
//);