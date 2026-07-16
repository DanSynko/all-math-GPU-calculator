export module pratt_parser;

#ifdef __INTELLISENSE__
#include <vector>
#else
import std;
#endif

import types;
import error_handler;

export class PrattParser {
public:
	explicit PrattParser(const std::vector<Token>& some_tokens) : tokens(some_tokens) {
		error_handler.messages.reserve(expr_strings_count);
	}

	[[nodiscard]] ParserResult parse() {
		ExpectedIndex result = start_pratt_parser(0);
		if (!result.has_value()) {
			error_handler.register_token(tokens[i], result.error());
			return std::unexpected(error_handler.messages);
		}
		if (!error_handler.messages.empty()) {
			return std::unexpected(error_handler.messages);
		}
		if (i != tokens.back().index) {
			error_handler.register_token(tokens[i], ErrorType::UnexpectedEnd);
			return std::unexpected(error_handler.messages);
		}
		return ast;
	}
private:
	[[nodiscard]] ExpectedIndex start_pratt_parser(int rbp) {
		if (i == tokens.size()) return 0;

		Token current_token = tokens[i];

		i++;

		ExpectedIndex left = NUD(current_token);
		if (!left.has_value()) {
			i--;
			if (error_handler.messages.size() < expr_strings_count) {
				int defect_index = (left.error() == ErrorType::ParenthesesImbalance) ? openp_index : i;
				Errors parse_errors = error_handler.panic_mode(tokens, i, left.error(), defect_index);
				errors_count++;
			}
			return std::unexpected(left.error());
		}

		if (i != tokens.size() && tokens[i].type == TypeOfToken::Number) {
			return std::unexpected(ErrorType::UnexpectedNumber);
		}

		while (rbp < lookahead_lbp(tokens[i].type)) {
			Token op_token = tokens[i];
			i++;
			left = LED(op_token, left);
		}

		return left;
	}

	[[nodiscard]] int lookahead_lbp(TypeOfToken token_type) const noexcept {
		switch (token_type) {
		case TypeOfToken::Plus:
		case TypeOfToken::Minus:
			return 1;
		case TypeOfToken::MultiplicationSign:
		case TypeOfToken::DivisionSign:
			return 2;
		case TypeOfToken::PowerSign:
			return 4;
		case TypeOfToken::PercentSign:
			return 5;
		default:
			return 0;
		}
	}

	[[nodiscard]] ExpectedIndex NUD(const Token& token) {
		if (token.type == TypeOfToken::NegationSign) {
			ExpectedIndex operand = start_pratt_parser(4);
			return ast.value().add_node(token, operand);
		}
		else if (token.type == TypeOfToken::OpenParenthesis) {
			int current_openp_index = i;
			ExpectedIndex open_p = start_pratt_parser(0);
			if (tokens[i].type == TypeOfToken::CloseParenthesis) {
				i++;
			}
			else {
				openp_index = current_openp_index;
				return std::unexpected(ErrorType::ParenthesesImbalance);
			}
			return open_p;
		}
		else if (token.type == TypeOfToken::Number) {
			return ast.value().add_node(token);
		}
		else if (token.type == TypeOfToken::EndOfFile) {
			return std::unexpected(ErrorType::UnexpectedEnd);
		}
		else {
			return std::unexpected(ErrorType::InvalidOperator);
		}
	}

	[[nodiscard]] ExpectedIndex LED(const Token& token, const ExpectedIndex& left) {
		ExpectedIndex right;
		switch (token.type) {
		case TypeOfToken::PercentSign:
			return ast.value().add_node(token, left);
		case TypeOfToken::PowerSign:
			right = start_pratt_parser(lookahead_lbp(token.type) - 1);
			return ast.value().add_node(token, left, right);
		case TypeOfToken::NegationSign:
			return std::unexpected(ErrorType::InvalidPrefixOperator);
		default:
			right = start_pratt_parser(lookahead_lbp(token.type));
			return ast.value().add_node(token, left, right);
		}
	}

	std::vector<Token> tokens;
	ParserResult ast;

	ErrorHandler error_handler;

	int i = 0;
	int current_index = 0;
	int openp_index = 0;
	int expr_strings_count = 1;
	int errors_count = 0;
};