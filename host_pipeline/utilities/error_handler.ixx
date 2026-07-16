export module error_handler;

#ifdef __INTELLISENSE__
#include <string>
#else
import std;
#endif

import types;

export class ErrorHandler {
public:
	void register_token(const Token& defect_token, ErrorType error_type) {
		messages.emplace_back(get_message(error_type), defect_token);
	}

	void register_semantic(ErrorType error_type) {
		messages.emplace_back(get_message(error_type));
	}

	[[nodiscard]] Errors panic_mode(const std::vector<Token>& tokens, int i, ErrorType left_error, int defect_index) {
		register_token(tokens[defect_index], left_error);
		while (tokens[i].type != TypeOfToken::EndOfFile) {
			i++;
		}
		return messages;
	}

	Errors messages;
private:
	[[nodiscard]] std::string_view get_message(ErrorType type) const noexcept {
		switch (type) {
		case ErrorType::NoInput:
			return "You haven't entered anything. Was this accidental?";
		case ErrorType::UnknownSymbol:
			return "The unknown symbol has been detected. It isn't math symbol or don't supported by this program. The list of supported symbols you can check by 'help' command";
		case ErrorType::UnexpectedEnd:
			return "The end of the expression or the subexpression has suddenly detected.";
		case ErrorType::UnexpectedNumber:
			return "Unexpected number detected. Numbers cannot be separated by spaces; use operators or merge them.";
		case ErrorType::InvalidOperator:
			return "Invalid operator was detected: A binary-operator have less than two operands or a postfix-operator is before the operand.";
		case ErrorType::ParenthesesImbalance:
			return "The count of open- and close parentheses isn't same.";
		case ErrorType::InvalidPrefixOperator:
			return "The prefix-operator was detected after the operand. It must be before an operand.";
		case ErrorType::InvalidFloatingPoint:
			return "An unexpected floating point was detected outside the operand, or there were two or more of them. It must be a single one and located inside the operand.";
		case ErrorType::DivByZero:
			return "Division by zero is undefined.";
		case ErrorType::UnsupportedComplex:
			return "Complex numbers are not supported in the program at the moment. Apologize for the inconvenience.";
		}
	}
};