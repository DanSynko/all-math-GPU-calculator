#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <string_view>
//#include <concepts>
#include <algorithm>
#include <expected>
#include <variant>
#include <utility>

#include "include/magic_enum-master/include/magic_enum/magic_enum.hpp"

#include <iomanip>

#ifdef _WIN32
#include <Windows.h>
#endif




void help_command();

void print_to_center(std::string_view text, int shift) {
#ifdef _WIN32
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	int width;

	if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
		width = csbi.dwSize.X;
	}
	else {
		width = 120;
	}

	if (text.length() < width) {
		int spaces = ((width - text.length()) / 2) - shift;
		std::cout << std::string(spaces, ' ');
	}
	std::cout << text;
#endif
}




enum class TypeOfToken {
	Default,
	Number,
	Plus,
	Minus,
	MultiplicationSign,
	DivisionSign,
	NegativeSign,
	OpenParenthesis,
	CloseParenthesis,
	PowerSign,
	PercentSign,
	EndOfFile
};

struct Token {
	uint32_t index = 0;
	TypeOfToken type = TypeOfToken::Default;
	std::string value;

	void setter(int num, TypeOfToken current_type, auto iterator_value) {
		type = current_type;
		index = num;
		value = iterator_value;
	}

	void print() const {
		std::cout << std::left;
		std::cout
			<< " [index: " << std::setw(13) << std::string(std::to_string(index) + ";")
			<< " type: " << std::setw(23) << (std::string(magic_enum::enum_name(type)) + ";")
			<< " value: " << std::setw(1) << value
			<< " ] \n";
	}
};



	enum class TypeOfError {
		NoInput,
		UnknownSymbol,
		ParenthesesImbalance,
		ExtraOperator,
		InvalidOperator,
		UnexpectedEnd,
		InvalidPrefixOperator
	};

	struct ErrorMessage {
		Token defect_token;
		std::string message;
	};


struct ErrorHandler {
	std::vector<ErrorMessage> errors_list;

	std::string error_message_templates(TypeOfError error_type) {
		switch (error_type) {
		case TypeOfError::NoInput:
			return "You haven't entered anything. Was this accidental?\n";
		case TypeOfError::UnknownSymbol:
			return "The unknown symbol has been detected. It isn't math symbol or don't supported by this program. The list of supported symbols you can check by 'help' command\n";
		case TypeOfError::ExtraOperator:
			return "An extra operator was detected.\n";
		case TypeOfError::UnexpectedEnd:
			return "The end of the expression or the subexpression has suddenly detected.\n";
		case TypeOfError::InvalidOperator:
			return "Invalid operator was detected: A binary-operator have less than two operands or a postfix-operator is before the operand.\n";
		case TypeOfError::ParenthesesImbalance:
			return "The count of open- and close parentheses isn't same.\n";
		case TypeOfError::InvalidPrefixOperator:
			return "The prefix-operator has detected after the operand. It must be before an operand.\n";
		default:
			return "Unknown error\n";
		}
	}

	std::vector<ErrorMessage> panic_mode(std::vector<Token>& tokens, int i, TypeOfError left_error, int defect_index) {
		errors_list.push_back(ErrorMessage(tokens[defect_index], error_message_templates(left_error)));
		while (tokens[i].type != TypeOfToken::EndOfFile) {
			i++;
		}
		return errors_list;
	}
};


using SafeTokens = std::expected<std::vector<Token>, TypeOfError>;
using LexerResult = std::variant<std::vector<Token>, std::vector<ErrorMessage>>;

class Lexer {

	std::string expr;
	
	SafeTokens tokens;

	ErrorHandler error_handler;

	SafeTokens tokenization(std::string_view expr) {
		std::vector<Token> tokens;
		int i = 0;
		for (auto it = expr.begin(); it != expr.end(); it++) {
			if (*it == ' ') continue;

			if (std::string_view("+-*/()%^").find(*it) != std::string_view::npos) {
				Token a_operator;
				switch (*it) {
				case '+':
					a_operator.setter(i, TypeOfToken::Plus, *it);
					tokens.push_back(a_operator);
					break;
				case '-':
					if (it == expr.begin() || *(std::prev(it)) == '(') {
						a_operator.setter(i, TypeOfToken::NegativeSign, '~');
						tokens.push_back(a_operator);
					}
					else {
						a_operator.setter(i, TypeOfToken::Minus, *it);
						tokens.push_back(a_operator);
					}
					break;
				case '*':
					a_operator.setter(i, TypeOfToken::MultiplicationSign, *it);
					tokens.push_back(a_operator);
					break;
				case '/':
					a_operator.setter(i, TypeOfToken::DivisionSign, *it);
					tokens.push_back(a_operator);
					break;
				case '(':
					a_operator.setter(i, TypeOfToken::OpenParenthesis, *it);
					tokens.push_back(a_operator);
					break;
				case ')':
					a_operator.setter(i, TypeOfToken::CloseParenthesis, *it);
					tokens.push_back(a_operator);
					break;
				case '%':
					a_operator.setter(i, TypeOfToken::PercentSign, *it);
					tokens.push_back(a_operator);
					break;
				case '^':
					a_operator.setter(i, TypeOfToken::PowerSign, *it);
					tokens.push_back(a_operator);
					break;
				default:
					break;
				}
			}
			else if (std::string_view("0123456789., ").find(*it) != std::string_view::npos) {
				Token number;
				bool has_point = false;
				while (it != expr.end() && (isdigit(*it) || *it == ' ' || *it == '.' || *it == ',')) {
					if (*it != ' ') {
						if ((*it == '.' || *it == ',')) {
							if (!has_point) {
								number.value.push_back('.');
								has_point = true;
							}
						}
						else {
							number.value.push_back(*it);
						}
					}
					it++;
				}
				number.index = i;
				number.type = TypeOfToken::Number;
				tokens.push_back(number);
				it--;
			}
			else {
				Token defect_token;
				defect_token.setter(i, TypeOfToken::Default, *it);
				error_handler.errors_list.push_back(ErrorMessage(defect_token, error_handler.error_message_templates(TypeOfError::UnknownSymbol)));
				return std::unexpected(TypeOfError::UnknownSymbol);
			}
			i++;
		}

		Token eof;
		eof.setter(i, TypeOfToken::EndOfFile, '\0');
		tokens.push_back(eof);

		if (tokens[0].type == TypeOfToken::EndOfFile) {
			error_handler.errors_list.push_back(ErrorMessage(tokens[0], error_handler.error_message_templates(TypeOfError::NoInput)));
			return std::unexpected(TypeOfError::NoInput);
		}

		return tokens;
	}
public:
	Lexer(std::string_view expr) : expr(expr) {}

	LexerResult get_lexer_result() {
		tokens = tokenization(expr);
		if (tokens.has_value()) {
			return std::move(tokens.value());
		}
		else {
			return std::move(error_handler.errors_list);
		}
	}
};

enum class NodeTags {
	Unknown,
	Number,
	Add,
	Subtract,
	Multiply,
	Divide,
	NegativeNum,
	Power,
	Percent
};

NodeTags typeoftoken_to_nodetags(TypeOfToken token_type) {
	switch (token_type) {
	case TypeOfToken::Number:
		return NodeTags::Number;
		break;
	case TypeOfToken::Plus:
		return NodeTags::Add;
		break;
	case TypeOfToken::Minus:
		return NodeTags::Subtract;
		break;
	case TypeOfToken::MultiplicationSign:
		return NodeTags::Multiply;
		break;
	case TypeOfToken::DivisionSign:
		return NodeTags::Divide;
		break;
	case TypeOfToken::NegativeSign:
		return NodeTags::NegativeNum;
		break;
	case TypeOfToken::PowerSign:
		return NodeTags::Power;
		break;
	case TypeOfToken::PercentSign:
		return NodeTags::Percent;
		break;
	default:
		return NodeTags::Unknown;
	}
}



using SafeUint32t = std::expected<uint32_t, TypeOfError>;


struct AbstractSyntaxTree_SoA {
	std::vector<NodeTags> node_tags;
	std::vector<std::string_view> node_data;
	std::vector<SafeUint32t> child_relationships;
	std::vector<int32_t> child_start;
	std::vector<uint32_t> child_count;

	uint32_t add_node(Token& some_token) {
		node_tags.push_back(typeoftoken_to_nodetags(some_token.type));
		node_data.push_back(some_token.value);
		child_start.push_back(-1);
		child_count.push_back(0);
		return node_tags.size() - 1;
	}

	uint32_t add_node(Token& some_token, SafeUint32t right_child_index) {
		node_tags.push_back(typeoftoken_to_nodetags(some_token.type));
		node_data.push_back(some_token.value);
		child_start.push_back(child_relationships.size());
		child_count.push_back(1);
		child_relationships.push_back(right_child_index);
		return node_tags.size() - 1;
	}

	uint32_t add_node(Token& some_token, SafeUint32t left_child_index, SafeUint32t right_child_index) {
		node_tags.push_back(typeoftoken_to_nodetags(some_token.type));
		node_data.push_back(some_token.value);
		child_start.push_back(child_relationships.size());
		child_count.push_back(2);
		child_relationships.push_back(left_child_index);
		child_relationships.push_back(right_child_index);
		return node_tags.size() - 1;
	}
};


using ParserResult = std::variant<AbstractSyntaxTree_SoA, std::vector<ErrorMessage>>;

class PrattParser {

	std::vector<Token> tokens;
	int i = 0;
	int current_index;
	int openp_index;
	AbstractSyntaxTree_SoA ast;

	ErrorHandler error_handler;




	int lookahead_lbp(TypeOfToken token_type) const {
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

	SafeUint32t NUD(Token& token) {
		if (token.type == TypeOfToken::NegativeSign) {
			SafeUint32t operand = parse_expression(3);
			return ast.add_node(token, operand);
		}
		else if (token.type == TypeOfToken::OpenParenthesis) {
			int current_openp_index = i;
			SafeUint32t open_p = parse_expression(0);
			if (tokens[i].type == TypeOfToken::CloseParenthesis) {
				i++;
			}
			else {
				openp_index = current_openp_index;
				return std::unexpected(TypeOfError::ParenthesesImbalance);
			}
			return open_p;
		}
		else if (token.type == TypeOfToken::Number) {
			return ast.add_node(token);
		}
		else {
			return std::unexpected(TypeOfError::InvalidOperator);
		}
	}

	SafeUint32t LED(Token& token, SafeUint32t left) {
		SafeUint32t right;
		switch (token.type) {
		case TypeOfToken::PercentSign:
			return ast.add_node(token, left);
		case TypeOfToken::PowerSign:
			right = parse_expression(lookahead_lbp(token.type) - 1);
			return ast.add_node(token, left, right);
		case TypeOfToken::NegativeSign:
			return std::unexpected(TypeOfError::InvalidPrefixOperator);
		default:
			right = parse_expression(lookahead_lbp(token.type));
			return ast.add_node(token, left, right);
		}
	}

public:
	PrattParser(std::vector<Token>& some_tokens) : tokens(some_tokens), current_index(0), openp_index(0) {}

	SafeUint32t parse_expression(int rbp) {
		if (i == tokens.size()) return 0;

		Token current_token = tokens[i];

		i++;

		auto left = NUD(current_token);
		if (!left.has_value()) {
			i--;
			if (left.error() == TypeOfError::ParenthesesImbalance) {
				std::vector<ErrorMessage> parse_errors = error_handler.panic_mode(tokens, i, left.error(), openp_index);
			}
			else {
				std::vector<ErrorMessage> parse_errors = error_handler.panic_mode(tokens, i, left.error(), i);
			}
			return std::unexpected(left.error());
		}


		while (rbp < lookahead_lbp(tokens[i].type)) {
			Token op_token = tokens[i];
			i++;
			left = LED(op_token, left);
		}

		return left;
	}

	ParserResult get_parser_result() {
		if (!error_handler.errors_list.empty()) {
			return std::move(error_handler.errors_list);
		}
		else if (i != tokens.size() - 1) {
			error_handler.errors_list.push_back(ErrorMessage(tokens[i], error_handler.error_message_templates(TypeOfError::UnexpectedEnd)));
			return std::move(error_handler.errors_list);
		}
		else {
			return std::move(ast);
		}
	}
};

int main()
{
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
#endif

	std::cout << "Welcome to all-math-gpu-calculator! This project was created for performing calculations from basic arithmetic to advanced mathematics.\n";
	std::cout << "Enter: \n 1) 'exit' to close the program. \n 2) 'help' to show the expression printing guide and supported operators and symbols. \n \n";
	std::cout << "The entered mathematical expression will be displayed in a clean mathematical format.\n\n";

	while (true) {
		std::cout << "Enter any mathematical expression or command:\n";

		ErrorHandler error_handler;

		std::string m_expr;
		std::getline(std::cin, m_expr);

		if (m_expr == "help") {
			help_command();
			continue;
		}
		else if (m_expr == "exit") {
			break;
		}

		Lexer lexer(m_expr);
		LexerResult tokens = lexer.get_lexer_result();

		if (std::holds_alternative<std::vector<ErrorMessage>>(tokens)) {
			std::cout << "\033[1;31m Math expression is invalid. The unexpected tokens: \n";
			for (auto& it : std::get<std::vector<ErrorMessage>>(tokens)) {
				it.defect_token.print();
				std::cout << '\n' << it.message + "\033[0m" << '\n\n';
			}
			continue;
		}

		std::cout << "┌──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐\n";
		print_to_center("1. TOKENS\n", 0);
		for (const Token& token : std::get<std::vector<Token>>(tokens)) {
			token.print();
		}


		std::cout << "\n";
		PrattParser parser(std::get<std::vector<Token>>(tokens));
		parser.parse_expression(0);
		ParserResult soa_ast = parser.get_parser_result();

		if (std::holds_alternative<std::vector<ErrorMessage>>(soa_ast)) {
			std::cout << "\033[1;31m Math expression is invalid. The unexpected tokens: \n";
			for (auto& it : std::get<std::vector<ErrorMessage>>(soa_ast)) {
				it.defect_token.print();
				std::cout << '\n' << it.message + "\033[0m" << '\n\n';
			}
		}
		else {
			std::cout << "\033[32m Math expression is valid!\n" << "\033[0m";
			continue;
		}
	}

	return 0;
}






void help_command() {
	std::cout << "\n╔══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗\n";
	std::cout << "1. SUPPORTED OPERATORS AND SYMBOLS: \n \n";

	std::cout << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n";
	std::cout << "FORMAT: [math object]                -             [name] \n";
	std::cout << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n \n";


	// this list in std::array will change.

		// TODO:
			// General Math: ⋅, ⁰ ¹ ² ³ ⁴ ⁵ ⁶ ⁷ ⁸ ⁹ ˣ ʸ ⁿ, √(and ³√, ⁴√, ⁵√), variables(𝑥, 𝑦), =, ≈, ƒ, log(and log₂, log₁₀), | (modules).
			// Trigonomethry: °, π, sin, cos, tan.
			// Calculus: lim, →, ℯ, ln, 𝒅, ′, ″, ∂, ∫, ∫∫, ∫∫∫, ∮, ₀ ₁ ₂ ₃ ₄ ₅ ₆ ₇ ₈ ₉ ₐ ₑ ₒ ₓ ₙ, Σ, ∏, ꝏ, ∞, ⁺, ₊, ⁻, ₋.


	std::array<std::string_view, 3> supported_symbols = {
		"+, -, ⋅, /         -         basic arithmetic operators\n",
		"𝑥ʸ                 -         power\n",
		"%                  -         percent\n"
	};
	for (const auto& i : supported_symbols) {
		std::cout << i << "\n";
	}



	std::cout << "2. MATH EXPRESSIONS PRINTING GUIDE. \n \n";
	std::cout
		<< "If you can't print Unicode math symbols (like ⋅)"
		<< " - you can enter them using the simple methods listed below, and they will be converted to Unicode.\n"
		<< "Here is the list of how to print symbols. \n\n ";

	std::cout << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n";
	std::cout << "FORMAT: [Unicode symbol]        -        [simple method of printing] " << std::endl;
	std::cout << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n\n";

	std::array<std::string_view, 2> guide = {
		"⋅            -          *",
		"𝑥ʸ           -          x^(y) ",
	};
	for (const auto& i : guide) {
		std::cout << i << "\n";
	}

	std::cout << "\n";

	std::cout << "╚══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝ \n \n \n";
}