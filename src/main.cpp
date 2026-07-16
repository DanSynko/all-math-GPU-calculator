#ifdef __INTELLISENSE__
#include <iostream>
#include <print>
#include <vector>
#include <array>
#include <string>
#include <string_view>
#include <iterator>
#include <expected>
#include <variant>
#include <utility>
#include <charconv>
#include <fstream>
#else
import std;
#endif

#ifdef _WIN32
#include <Windows.h>
#endif
import settings;
import help;
import console_print;

import lookup_table;


import error_handler;

import types;

import lexer;
import pratt_parser;
import ir_generator;
import ir_optimizer;

import cuda_backend;


int main(int argc, char* argv[])
{
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
#endif

	Settings settings;

	bitmask current_settings = settings.load_from_config();

	if (argc > 1) {
		std::expected<bitmask, int> args_settings = settings.load_from_arguments(argc, argv);
		if (args_settings.has_value()) {
			current_settings = args_settings.value();
		}
		else {
			return args_settings.error();
		}
	}

	std::println("Welcome to all-math-gpu-calculator! This project was created for performing calculations from basic arithmetic to advanced mathematics.)");
	std::println("Enter: \n 1) 'exit' to close the program. \n 2) 'help' to show the expression printing guide and supported operators and symbols.\n");
	std::println("The entered mathematical expression will be displayed in a clean mathematical format.\n");

	while (true) {
		std::println("Enter any mathematical expression or command:");

		ErrorHandler error_handler;

		std::string expression;

		std::getline(std::cin, expression);

		if (expression == "help") {
			print_help();
			continue;
		}
		else if (expression == "exit") {
			break;
		}

		Lexer lexer(std::move(expression));

		std::expected<double, Errors> program_pipeline = lexer.tokenize()
			.transform([](const std::vector<Token>& correct_tokens) {
				std::println("┌──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐");
				Console::print_to_center("1. TOKENS", 0);

				for (const Token& token : correct_tokens) {
					token.print();
				}
				std::println();

				return correct_tokens;
			})
			.and_then([](const std::vector<Token>& correct_tokens) {
				Console::print_to_center("2. ABSTRACT SYNTAX TREE VALIDATION.", 0);

				PrattParser parser(correct_tokens);

				return parser.parse();
			})
			.and_then([current_settings](const AbstractSyntaxTree_SoA& soa_ast) {
				std::println("\033[32m Math expression is valid!\033[0m");

				Console::print_to_center("3. IR GENERATION.\n", 0);

				IRGenerator ir_generator(soa_ast);
				std::vector<IRInstruction> instructions = ir_generator.generate();
				ir_generator.print(instructions);
				std::println();

				Console::print_to_center("4. IR OPTIMIZATION.\n", 0);
				IROptimizer ir_optimizer(std::move(instructions), std::move(ir_generator.get_operands_pool()), current_settings);
				auto optimized_ir = ir_optimizer.optimize();
				ir_generator.print(std::move(optimized_ir).value_or(std::vector<IRInstruction>{}));

				return ir_optimizer.get_constant_expression_result<double>(optimized_ir);
			})
			.transform_error([](const Errors& errors) {
				std::println("\033[1;31m Math expression is invalid. Reason:");
				for (const auto& it : errors) {
					if (it.defect_token.has_value()) {
						it.defect_token->print();
					}
					std::println("{}\033[0m\n", it.message);
				}
				return errors;
			});

		if (!program_pipeline.has_value()) continue;

		Console::print_to_center("5 EVALUATE IN GPU.\n", 0);
		CudaEvaluator evaluator;
		double result = evaluator.evaluate(std::to_string(program_pipeline.value()));

		Console::print_to_center("\033[32m 6. RESULT: ", 0);
		Console::print_to_center(std::string(std::format("{}\033[0m\n", result)), 0);
	}

	return 0;
}