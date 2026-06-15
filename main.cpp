#include <iostream>
#include <array>
#include <stack>
#include <queue>
#include <string>
#include <string_view>
#include <concepts>
#include <algorithm>
#include <numeric>

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

// First level of protection against incorrect input(Symbol existence checker).
void safe_input(std::string& expr) {
	bool correct_expr;
	do {
		std::getline(std::cin, expr);

		if (expr == "help" || expr == "exit") {
			return;
		}

		// this list in string_view will change.

		// TODO:
			// General Math: ⋅, ⁰ ¹ ² ³ ⁴ ⁵ ⁶ ⁷ ⁸ ⁹ ˣ ʸ ⁿ, √(and ³√, ⁴√, ⁵√), variables(𝑥, 𝑦), =, ≈, ƒ, log(and log₂, log₁₀), | (modules).
			// Trigonomethry: °, π, sin, cos, tan.
			// Calculus: lim, →, ℯ, ln, 𝒅, ′, ″, ∂, ∫, ∫∫, ∫∫∫, ∮, ₀ ₁ ₂ ₃ ₄ ₅ ₆ ₇ ₈ ₉ ₐ ₑ ₒ ₓ ₙ, Σ, ∏, ꝏ, ∞, ⁺, ₊, ⁻, ₋.

		correct_expr = true;
		if (!std::all_of(expr.begin(), expr.end(), [](char c) {
			return std::string_view("0123456789+-*/.,()%^ ").find(c) != std::string_view::npos;
			})) {
			correct_expr = false;
			std::cout << "Error: invalid math expression. Try again.\n";
		}

	} while (!correct_expr);
}

// second level of protection against incorrect input(Syntax checker).
template<typename T>
concept mathexpr = std::same_as<T, std::string> || std::same_as<T, std::vector<std::string>>;
bool math_expression_validation(const mathexpr auto& expr) {
	int operators_count = 0;
	int operands_count = 0;
	int open_parentheses_count = 0;
	int close_parentheses_count = 0;
	for (auto it = expr.begin(); it != expr.end(); it++) {
		if (std::string_view("+-*/^").find(*it) != std::string_view::npos) {
			if constexpr (std::is_same_v<decltype(expr), const std::string&>) {
				if (*it == '-' && (it == expr.begin() || *(std::prev(it)) == '(')) {
					continue;
				}
			}
			operators_count++;
		}
		else if (std::string_view("()").find(*it) != std::string_view::npos) {
			if (std::string_view("(").find(*it) != std::string_view::npos) {
				open_parentheses_count++;
			}
			else if (std::string_view(")").find(*it) != std::string_view::npos) {
				close_parentheses_count++;
			}
		}
		else if (std::string_view("%").find(*it) != std::string_view::npos) {
			if constexpr (std::is_same_v<decltype(expr), const std::string&>) {
				if (it == expr.begin()) {
					return false;
				}

				auto prev = it;
				prev--;
				if (std::string_view("+-*/ ").find(*prev) != std::string_view::npos) {
					return false;
				}

				auto next = it;
				next++;
				if (next != expr.end() && std::string_view("+-*/) ").find(*next) == std::string_view::npos) {
					return false;
				}
			}
		}
		else {
			operands_count++;
		}
	}
	if (operands_count == 0) {
		return false;
	}
	else if (operands_count <= operators_count) {
		return false;
	}
	else if (open_parentheses_count != close_parentheses_count) {
		return false;
	}

	return true;
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
	PercentSign
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

std::vector<Token> lexer(std::string_view expr) {
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
				continue;
			}
		}
		else {
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
		i++;
	}
	return tokens;
}

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
struct AbstractSyntaxTree_SoA {
	std::vector<NodeTags> node_tags;
	std::vector<std::string> node_data;
	std::vector<uint32_t> child_relationships;
	std::vector<int32_t> child_start;
	std::vector<uint32_t> child_count;

	uint32_t add_node(Token& some_token) {
		node_tags.push_back(typeoftoken_to_nodetags(some_token.type));
		node_data.push_back(some_token.value);
		child_start.push_back(-1);
		child_count.push_back(0);
		return node_tags.size() - 1;
	}

	uint32_t add_node(Token& some_token, uint32_t right_child_index) {
		node_tags.push_back(typeoftoken_to_nodetags(some_token.type));
		node_data.push_back(some_token.value);
		child_start.push_back(child_relationships.size());
		child_count.push_back(1);
		child_relationships.push_back(right_child_index);
		return node_tags.size() - 1;
	}

	uint32_t add_node(Token& some_token, uint32_t left_child_index, uint32_t right_child_index) {
		node_tags.push_back(typeoftoken_to_nodetags(some_token.type));
		node_data.push_back(some_token.value);
		child_start.push_back(child_relationships.size());
		child_count.push_back(2);
		child_relationships.push_back(left_child_index);
		child_relationships.push_back(right_child_index);
		return node_tags.size() - 1;
	}
};

class PrattParser {
	std::vector<Token> tokens;
	int i = 0;
	AbstractSyntaxTree_SoA ast;

	int get_lbp(TypeOfToken token_type) const {
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

	int lookahead_lbp() const {
		if (i >= tokens.size()) return 0;
		return get_lbp(tokens[i].type);
	}

	uint32_t NUD(Token& token) {
		if (token.type == TypeOfToken::NegativeSign) {
			uint32_t operand = parse_expression(3);
			return ast.add_node(token, operand);
		}
		else if (token.type == TypeOfToken::OpenParenthesis) {
			uint32_t open_p = parse_expression(0);
			if (i < tokens.size() && tokens[i].type == TypeOfToken::CloseParenthesis) {
				i++;
			}
			return open_p;
		}
		return ast.add_node(token);
	}

	uint32_t LED(Token& token, int left) {
		int right;
		switch (token.type) {
		case TypeOfToken::PercentSign:
			return ast.add_node(token, left);
		case TypeOfToken::PowerSign:
			right = parse_expression(get_lbp(token.type) - 1);
			return ast.add_node(token, left, right);
		default:
			right = parse_expression(get_lbp(token.type));
			return ast.add_node(token, left, right);
		}
	}

public:
	PrattParser(std::vector<Token>& some_tokens) : tokens(some_tokens) {}

	uint32_t parse_expression(int rbp) {
		if (i >= tokens.size()) return 0;

		Token current_token = tokens[i];

		i++;
		uint32_t left = NUD(current_token);

		while (rbp < lookahead_lbp()) {
			Token op_token = tokens[i];
			i++;

			left = LED(op_token, left);
		}

		return left;
	}

	AbstractSyntaxTree_SoA get_ast() const {
		return ast;
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

			std::string m_expr;
			safe_input(m_expr);

			if (m_expr == "help") {
				help_command();
				continue;
			}
			else if (m_expr == "exit") {
				break;
			}

			std::vector<Token> tokens = lexer(m_expr);

			std::cout << "┌──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐\n";
			print_to_center("1. TOKENS\n", 0);
			for (const Token& token : tokens) {
				token.print();
			}
			std::cout << "\n";
			PrattParser parser(tokens);
			parser.parse_expression(0);
			AbstractSyntaxTree_SoA soa_ast = parser.get_ast();
		}

		return 0;
	}






	void help_command() {
		std::cout << "\n╔══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗\n";
		std::cout << "1. SUPPORTED OPERATORS AND SYMBOLS: \n \n";

		std::cout << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n";
		std::cout << "FORMAT: [math object]                -             [name] \n";
		std::cout << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n \n";

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