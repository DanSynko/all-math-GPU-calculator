export module lexer;

#ifdef __INTELLISENSE__
#include <string>
#else
import std;
#endif

import types;
import lookup_table;
import error_handler;

export class Lexer {
public:
	explicit Lexer(std::string_view expression) : expression(expression), it(expression.begin()) {}

	[[nodiscard]] ExpectedTokens tokenize() {
		if (expression.empty()) {
			error_handler.register_semantic(ErrorType::NoInput);
			return std::unexpected(std::move(error_handler.messages));
		}

		std::vector<Token> tokens;

		std::string_view current_symbol_value_it;
		for (; it != expression.end(); ++it) {
			current_symbol_value = static_cast<bitmask>(*it);
			if (ascii_symbols[current_symbol_value] == 0) {
				Token defect_token(i, TypeOfToken::Default, current_symbol_value_it);
				error_handler.register_token(defect_token, ErrorType::UnknownSymbol);
				break;
			}

			current_symbol_value_it = std::string_view(it, it + 1);
			current_symbol_type = ascii_symbols[current_symbol_value];

			if (current_symbol_type & MASK_SPACE) continue;

			if (current_symbol_type & (MASK_OPERATOR | mask_parentheses)) {
				switch (current_symbol_value) {
				case '+':
					tokens.emplace_back(i, TypeOfToken::Plus, current_symbol_value_it);
					break;
				case '-':
					tokens.emplace_back(i, (is_negationsign(tokens)) ? TypeOfToken::NegationSign : TypeOfToken::Minus, current_symbol_value_it);
					break;
				case '*':
					tokens.emplace_back(i, TypeOfToken::MultiplicationSign, current_symbol_value_it);
					break;
				case '/':
					tokens.emplace_back(i, TypeOfToken::DivisionSign, current_symbol_value_it);
					break;
				case '(':
					tokens.emplace_back(i, TypeOfToken::OpenParenthesis, current_symbol_value_it);
					break;
				case ')':
					tokens.emplace_back(i, TypeOfToken::CloseParenthesis, current_symbol_value_it);
					break;
				case '%':
					tokens.emplace_back(i, TypeOfToken::PercentSign, current_symbol_value_it);
					break;
				case '^':
					tokens.emplace_back(i, TypeOfToken::PowerSign, current_symbol_value_it);
					break;
				}
			}
			else if (current_symbol_type & MASK_NUMBER) {
				std::expected<Token, ErrorType> number = tokenize_number();
				if (!number.has_value()) {
					Token defect_number(i, TypeOfToken::Default, current_symbol_value_it);
					error_handler.register_token(defect_number, number.error());
					break;
				}
				tokens.push_back(number.value());
			}
			++i;
		}

		if (!error_handler.messages.empty()) {
			return std::unexpected(std::move(error_handler.messages));
		}

		tokens.emplace_back(i, TypeOfToken::EndOfFile, std::string_view{});

		return tokens;
	}
private:
	[[nodiscard]] std::expected<Token, ErrorType> tokenize_number() noexcept {
		bitmask in_number_symbols = MASK_NUMBER | MASK_FLOATING_POINT;
		bool has_point = false;

		auto num_start = it;
		size_t num_length = 0;
		while (current_symbol_type & in_number_symbols) {
			if (current_symbol_type & MASK_FLOATING_POINT) {
				if (has_point) {
					return std::unexpected(ErrorType::InvalidFloatingPoint);
				}
				has_point = true;
			}

			++it; ++num_length;

			if (it == expression.end()) {
				break;
			}
			current_symbol_type = ascii_symbols[*it];
			current_symbol_value = *it;
		}

		bitmask num_end_symbol = static_cast<bitmask>(ascii_symbols[*(--it)]);
		if (num_end_symbol & MASK_FLOATING_POINT) {
			return std::unexpected(ErrorType::InvalidFloatingPoint);
		}

		return Token(i, TypeOfToken::Number, std::string_view(num_start, num_start + num_length));
	}

	[[nodiscard]] bool is_negationsign(const std::vector<Token>& tokens) const noexcept {
		if (tokens.empty()) {
			return true;
		}
		auto prev_symbol = tokens.back().value.data();
		bool is_prev_token_an_operator = (ascii_symbols[*prev_symbol] & (MASK_OPERATOR | MASK_OPEN_PARENTHESIS));
		return is_prev_token_an_operator;
	}


	static constexpr std::array ascii_symbols = lookup_table_fill();
	bitmask mask_parentheses = MASK_OPEN_PARENTHESIS | MASK_CLOSE_PARENTHESIS;

	ErrorHandler error_handler;

	std::string_view expression;
	std::string_view::const_iterator it;
	int i = 0;
	bitmask current_symbol_type = 0;
	uint8_t current_symbol_value = 0;
};