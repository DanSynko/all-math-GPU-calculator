#include "include/magic_enum-master/include/magic_enum/magic_enum.hpp"

#ifdef _WIN32
#include <Windows.h>
#endif

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

using bitmask = uint8_t;

struct Settings {
	enum Bitmask : bitmask {
		fast_math = 1 << 0
	};

	bitmask load_settings() {
		std::ifstream out("config.txt");
		bitmask loaded_settings = 0;
		const bitmask default_settings = 0;

		if (!out.is_open()) {
			return default_settings;
		}

		std::string line;
		while (std::getline(out, line)) {
			if (line.contains("fast_math=true")) {
				loaded_settings |= Bitmask::fast_math;
			}
		}

		return loaded_settings;
	}
};


namespace Console {

	int get_screen_width() noexcept {
#ifdef _WIN32
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		return (GetConsoleScreenBufferInfo(hConsole, &csbi)) ? csbi.dwSize.X : 120;
#endif
	}

	void print_divider(std::string_view text, int width = get_screen_width()) {
		for (int i = 2; i < width; ++i) {
			std::print("{}", text);
		}
	}

	void print_header(std::string_view text, std::string_view header_element = "█", std::string_view color = "\033[33m") {
		std::print("{}", color);
		Console::print_divider(header_element, Console::get_screen_width() / 3);
		std::print("{}", text);
		Console::print_divider(header_element, Console::get_screen_width() / 3);
		std::println("\033[0m \n");
	}

	void print_to_center(std::string_view text, int shift) {
#ifdef _WIN32
		int width = get_screen_width();

		int text_len = static_cast<int>(text.length());

		if (text_len < width) {
			int spaces = ((width - text_len) / 2) - shift;
			if (spaces < 0) spaces = 0;

			std::println("{:>{}}{}", "", spaces, text);
		}
		else {
			std::println("{}", text);
		}
#endif
	}
}



void print_help();



enum class TypeOfToken {
	Default,
	Number,
	Plus,
	Minus,
	MultiplicationSign,
	DivisionSign,
	NegationSign,
	OpenParenthesis,
	CloseParenthesis,
	PowerSign,
	PercentSign,
	EndOfFile
};

struct Token {
	int32_t index;
	TypeOfToken type;
	std::string value;

	Token() = default;

	Token(int num, TypeOfToken type, std::string_view value)
		: index(num)
		, type(type)
		, value(value)
	{

	}

	Token(int num, TypeOfToken type, uint8_t single_char)
		: index(num)
		, type(type)
		, value(1, static_cast<char>(single_char)) 
	{

	}

	void print() const {
		std::println(" [index: {:<13} type: {:<23} value: {:<1} ]",
			std::to_string(index) + ";",
			std::string(magic_enum::enum_name(type)) + ";",
			value
		);
	}
};



enum class TypeOfError {
	NoInput,
	UnknownSymbol,
	ParenthesesImbalance,
	ExtraOperator,
	InvalidOperator,
	UnexpectedEnd,
	InvalidPrefixOperator,
	InvalidFloatingPoint,
	DivByZero
};

struct ErrorMessage {
	std::string message;
	std::optional<Token> defect_token;
};


using Errors = std::vector<ErrorMessage>;


struct ErrorHandler {
	Errors messages;

	[[nodiscard]] std::string get_message(TypeOfError type) const noexcept {
		switch (type) {
		case TypeOfError::NoInput:
			return "You haven't entered anything. Was this accidental?";
		case TypeOfError::UnknownSymbol:
			return "The unknown symbol has been detected. It isn't math symbol or don't supported by this program. The list of supported symbols you can check by 'help' command";
		case TypeOfError::ExtraOperator:
			return "An extra operator was detected.";
		case TypeOfError::UnexpectedEnd:
			return "The end of the expression or the subexpression has suddenly detected.";
		case TypeOfError::InvalidOperator:
			return "Invalid operator was detected: A binary-operator have less than two operands or a postfix-operator is before the operand.";
		case TypeOfError::ParenthesesImbalance:
			return "The count of open- and close parentheses isn't same.";
		case TypeOfError::InvalidPrefixOperator:
			return "The prefix-operator was detected after the operand. It must be before an operand.";
		case TypeOfError::InvalidFloatingPoint:
			return "An unexpected floating point was detected outside the operand, or there were two or more of them. It must be a single one and located inside the operand.";
		case TypeOfError::DivByZero:
			return "You cannot divide by zero.";
		}
	}

	void register_token(const Token& defect_token, TypeOfError error_type) {
		messages.emplace_back(get_message(error_type), defect_token);
	}

	void register_semantic(TypeOfError error_type) {
		messages.emplace_back(get_message(error_type));
	}

	[[nodiscard]] Errors panic_mode(const std::vector<Token>& tokens, int i, TypeOfError left_error, int defect_index) {
		register_token(tokens[defect_index], left_error);
		while (tokens[i].type != TypeOfToken::EndOfFile) {
			i++;
		}
		return messages;
	}
};


constexpr bitmask MASK_NUMBER = 1 << 0;
constexpr bitmask MASK_OPERATOR = 1 << 1;
constexpr bitmask MASK_OPEN_PARENTHESIS = 1 << 2;
constexpr bitmask MASK_CLOSE_PARENTHESIS = 1 << 3;
constexpr bitmask MASK_FLOATING_POINT = 1 << 4;
constexpr bitmask MASK_SPACE = 1 << 5;

[[nodiscard]] constexpr std::array<bitmask, 256> lookup_table_fill() noexcept {
	std::array<bitmask, 256> symbols{};

	for (int i = '0'; i <= '9'; ++i) {
		symbols[i] = MASK_NUMBER;
	}

	symbols['+'] = MASK_OPERATOR;
	symbols['-'] = MASK_OPERATOR;
	symbols['*'] = MASK_OPERATOR;
	symbols['/'] = MASK_OPERATOR;

	symbols['^'] = MASK_OPERATOR;
	symbols['%'] = MASK_OPERATOR;

	symbols['.'] = MASK_FLOATING_POINT;
	symbols[','] = MASK_FLOATING_POINT;

	symbols['('] = MASK_OPEN_PARENTHESIS;
	symbols[')'] = MASK_CLOSE_PARENTHESIS;

	symbols[' '] = MASK_SPACE;

	return symbols;
}


using ExpectedTokens = std::expected<std::vector<Token>, Errors>;

class Lexer {
	static constexpr std::array<bitmask, 256> ascii_symbols = lookup_table_fill();
	bitmask mask_parentheses = MASK_OPEN_PARENTHESIS | MASK_CLOSE_PARENTHESIS;

	ErrorHandler error_handler;

	std::string_view expr;
	std::string_view::const_iterator it;
	int i = 0;
	bitmask current_symbol_type = 0;
	uint8_t current_symbol_value = 0;

	[[nodiscard]] std::expected<Token, TypeOfError> number_tokenize() {
		Token number;
		bool has_point = false;

		bitmask in_number_symbols = MASK_NUMBER | MASK_FLOATING_POINT | MASK_SPACE;

		while (current_symbol_type & in_number_symbols) {
			if (current_symbol_type & MASK_FLOATING_POINT) {
				if (has_point) {
					return std::unexpected(TypeOfError::InvalidFloatingPoint);
				}
				else {
					number.value.push_back('.');
					has_point = true;
				}
			}
			else if (!(current_symbol_type & MASK_SPACE)) {
				number.value.push_back(current_symbol_value);
			}

			++it;
			if (it == expr.end()) {
				break;
			}
			current_symbol_type = ascii_symbols[*it];
			current_symbol_value = *it;
		}

		if (static_cast<bitmask>(ascii_symbols[number.value.back()]) & MASK_FLOATING_POINT) {
			return std::unexpected(TypeOfError::InvalidFloatingPoint);
		}

		return number;
	}

	[[nodiscard]] bool is_negationsign(const std::vector<Token>& tokens) const noexcept {
		if (tokens.empty()) {
			return true;
		}
		char prev_symbol = tokens[tokens.back().index].value[0];
		bool is_prev_token_an_operator = (ascii_symbols[prev_symbol] & (MASK_OPERATOR | MASK_OPEN_PARENTHESIS));
		return is_prev_token_an_operator;
	}

public:
	Lexer(std::string_view expr) : expr(expr), it(expr.begin()) {}

	[[nodiscard]] ExpectedTokens tokenize() {
		std::vector<Token> tokens;

		for (; it != expr.end(); ++it) {
			current_symbol_value = static_cast<bitmask>(*it);
			if (ascii_symbols[current_symbol_value] == 0) {
				Token defect_token(i, TypeOfToken::Default, current_symbol_value);
				error_handler.register_token(defect_token, TypeOfError::UnknownSymbol);
				break;
			}

			current_symbol_type = ascii_symbols[current_symbol_value];
			if (current_symbol_type & MASK_SPACE) continue;

			if (current_symbol_type & (MASK_OPERATOR | mask_parentheses)) {
				switch (current_symbol_value) {
				case '+':
					tokens.emplace_back(i, TypeOfToken::Plus, current_symbol_value);
					break;
				case '-':
					if (is_negationsign(tokens)) {
						tokens.emplace_back(i, TypeOfToken::NegationSign, '~');
					}
					else {
						tokens.emplace_back(i, TypeOfToken::Minus, current_symbol_value);
					}
					break;
				case '*':
					tokens.emplace_back(i, TypeOfToken::MultiplicationSign, current_symbol_value);
					break;
				case '/':
					tokens.emplace_back(i, TypeOfToken::DivisionSign, current_symbol_value);
					break;
				case '(':
					tokens.emplace_back(i, TypeOfToken::OpenParenthesis, current_symbol_value);
					break;
				case ')':
					tokens.emplace_back(i, TypeOfToken::CloseParenthesis, current_symbol_value);
					break;
				case '%':
					tokens.emplace_back(i, TypeOfToken::PercentSign, current_symbol_value);
					break;
				case '^':
					tokens.emplace_back(i, TypeOfToken::PowerSign, current_symbol_value);
					break;
				}
			}
			else if (current_symbol_type & MASK_NUMBER) {
				std::expected<Token, TypeOfError> number = number_tokenize();
				if (!number.has_value()) {
					Token defect_number(i, TypeOfToken::Default, current_symbol_value);
					error_handler.register_token(defect_number, TypeOfError::InvalidFloatingPoint);
					break;
				}
				number->index = i;
				number->type = TypeOfToken::Number;
				tokens.push_back(number.value());
				--it;
			}
			++i;
		}

		if (!error_handler.messages.empty()) {
			return std::unexpected(std::move(error_handler.messages));
		}

		tokens.emplace_back(i, TypeOfToken::EndOfFile, '\0');

		if (tokens[0].type == TypeOfToken::EndOfFile) {
			error_handler.register_token(tokens[0], TypeOfError::NoInput);
			return std::unexpected(std::move(error_handler.messages));
		}


		return tokens;
	}	
};

enum class TypeOfNode {
	Number,
	Addition,
	Subtraction,
	Multiplication,
	Division,
	Negation,
	Power,
	Percent
};

[[nodiscard]] TypeOfNode typeoftoken_to_typeofnode(TypeOfToken token_type) noexcept {
	switch (token_type) {
	case TypeOfToken::Number:
		return TypeOfNode::Number;
	case TypeOfToken::Plus:
		return TypeOfNode::Addition;
	case TypeOfToken::Minus:
		return TypeOfNode::Subtraction;
	case TypeOfToken::MultiplicationSign:
		return TypeOfNode::Multiplication;
	case TypeOfToken::DivisionSign:
		return TypeOfNode::Division;
	case TypeOfToken::NegationSign:
		return TypeOfNode::Negation;
	case TypeOfToken::PowerSign:
		return TypeOfNode::Power;
	case TypeOfToken::PercentSign:
		return TypeOfNode::Percent;
	}
}



using ExpectedIndex = std::expected<int32_t, TypeOfError>;


struct AbstractSyntaxTree_SoA {
	std::vector<TypeOfNode> node_types;
	std::vector<std::string> node_data;
	std::vector<ExpectedIndex> child_relationships;
	std::vector<int32_t> child_start;
	std::vector<int32_t> child_count;

	[[nodiscard]] int32_t add_node(const Token& token) {
		node_types.push_back(typeoftoken_to_typeofnode(token.type));
		node_data.push_back(token.value);
		child_start.push_back(-1);
		child_count.push_back(0);
		return node_types.size() - 1;
	}

	[[nodiscard]] int32_t add_node(const Token& token, const ExpectedIndex& right_child_index) {
		node_types.push_back(typeoftoken_to_typeofnode(token.type));
		node_data.push_back(token.value);
		child_start.push_back(child_relationships.size());
		child_count.push_back(1);
		child_relationships.push_back(right_child_index);
		return node_types.size() - 1;
	}

	[[nodiscard]] int32_t add_node(const Token& token, const ExpectedIndex& left_child_index, const ExpectedIndex& right_child_index) {
		node_types.push_back(typeoftoken_to_typeofnode(token.type));
		node_data.push_back(token.value);
		child_start.push_back(child_relationships.size());
		child_count.push_back(2);
		child_relationships.push_back(left_child_index);
		child_relationships.push_back(right_child_index);
		return node_types.size() - 1;
	}
};


using ParserResult = std::expected<AbstractSyntaxTree_SoA, Errors>;

class PrattParser {
	std::vector<Token> tokens;
	ParserResult ast;

	ErrorHandler error_handler;

	int i = 0;
	int current_index = 0;
	int openp_index = 0;
	int expr_strings_count = 1;
	int errors_count = 0;

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
			ExpectedIndex operand = start_pratt_parser(3);
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
				return std::unexpected(TypeOfError::ParenthesesImbalance);
			}
			return open_p;
		}
		else if (token.type == TypeOfToken::Number) {
			return ast.value().add_node(token);
		}
		else if (token.type == TypeOfToken::EndOfFile) {
			return std::unexpected(TypeOfError::UnexpectedEnd);
		}
		else {
			return std::unexpected(TypeOfError::InvalidOperator);
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
			return std::unexpected(TypeOfError::InvalidPrefixOperator);
		default:
			right = start_pratt_parser(lookahead_lbp(token.type));
			return ast.value().add_node(token, left, right);
		}
	}

public:
	PrattParser(const std::vector<Token>& some_tokens) : tokens(some_tokens) {
		error_handler.messages.reserve(expr_strings_count);
	}

	ExpectedIndex start_pratt_parser(int rbp) {
		if (i == tokens.size()) return 0;

		Token current_token = tokens[i];

		i++;

		ExpectedIndex left = NUD(current_token);
		if (!left.has_value()) {
			i--;
			if (error_handler.messages.size() < expr_strings_count) {
				int defect_index = (left.error() == TypeOfError::ParenthesesImbalance) ? openp_index : i;
				Errors parse_errors = error_handler.panic_mode(tokens, i, left.error(), defect_index);
				errors_count++;
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

	[[nodiscard]] ParserResult parse() {
		start_pratt_parser(0);
		if (!error_handler.messages.empty()) {
			return std::unexpected(error_handler.messages);
		}
		if (i != tokens.back().index) {
			error_handler.register_token(tokens[i], TypeOfError::UnexpectedEnd);
			return std::unexpected(error_handler.messages);
		}
		return ast;
	}
};



class ExpressionConverter {
	AbstractSyntaxTree_SoA ast;
	int32_t index_field;
	std::string latex_text_field;
	std::string unicode_text_field;

	bool parent_is_power = false;

	[[nodiscard]] std::string get_unicode_power_exponent(const std::string& exponent) const {
		std::string exponent_expr;
		for (int i = 0; i < exponent.size(); ++i) {
			switch (exponent[i]) {
			case '0': exponent_expr += "⁰"; break;
			case '1': exponent_expr += "¹"; break;
			case '2': exponent_expr += "²"; break;
			case '3': exponent_expr += "³"; break;
			case '4': exponent_expr += "⁴"; break;
			case '5': exponent_expr += "⁵"; break;
			case '6': exponent_expr += "⁶"; break;
			case '7': exponent_expr += "⁷"; break;
			case '8': exponent_expr += "⁸"; break;
			case '9': exponent_expr += "⁹"; break;
			}
		}
		return exponent_expr;
	}

	void expr_converter(int32_t index, std::vector<std::string>& latex_text, std::vector<std::string>& unicode_text) {
		index_field = index;
		if (ast.child_start[index_field] == -1) {
			if (parent_is_power) {
				unicode_text.push_back(get_unicode_power_exponent(ast.node_data[index_field]));
			}
			else {
				unicode_text.push_back(ast.node_data[index_field]);
			}
			latex_text.push_back(ast.node_data[index_field]);
			return;
		}

		// Note: The SoA AST is built using post-order traversal, meaning the root node 
		// is located at the end of the vector. Thus, nodes are processed in reverse 
		// order (from end to start), causing the text to be assembled backwards. 
		// The resulting string must be flipped using std::reverse inside to_latex_and_unicode_text().
		switch (ast.node_types[index_field]) {
		case TypeOfNode::Addition:
			expr_converter(--index_field, latex_text, unicode_text);
			latex_text.push_back(" + ");
			unicode_text.push_back(parent_is_power ? "⁺" : " + ");
			expr_converter(--index_field, latex_text, unicode_text);
			break;
		case TypeOfNode::Subtraction:
			expr_converter(--index_field, latex_text, unicode_text);
			latex_text.push_back(" − ");
			unicode_text.push_back(parent_is_power ? "⁻" : " − ");
			expr_converter(--index_field, latex_text, unicode_text);
			break;
		case TypeOfNode::Multiplication:
			expr_converter(--index_field, latex_text, unicode_text);
			latex_text.push_back(" \\cdot ");
			unicode_text.push_back(" ⋅ ");
			expr_converter(--index_field, latex_text, unicode_text);
			break;
		case TypeOfNode::Division:
			latex_text.push_back("}");
			expr_converter(--index_field, latex_text, unicode_text);
			latex_text.push_back("}{");
			unicode_text.push_back(" / ");
			expr_converter(--index_field, latex_text, unicode_text);
			latex_text.push_back("\\frac{");
			break;
		case TypeOfNode::Negation:
			expr_converter(--index_field, latex_text, unicode_text);
			latex_text.push_back("−");
			unicode_text.push_back("−");
			break;
		case TypeOfNode::Percent:
			latex_text.push_back("\\,\\%");
			unicode_text.push_back("%");
			expr_converter(--index_field, latex_text, unicode_text);
			break;
		case TypeOfNode::Power:
			latex_text.push_back("}");
			parent_is_power = true;
			expr_converter(--index_field, latex_text, unicode_text);
			parent_is_power = false;
			latex_text.push_back("^{");
			expr_converter(--index_field, latex_text, unicode_text);
			break;
		}
	}
	void to_latex_and_unicode_text() {
		std::vector<std::string> latex_text_syntax;
		std::vector<std::string> unicode_text;

		latex_text_syntax.reserve(ast.node_data.size());
		unicode_text.reserve(ast.node_data.size());

		expr_converter(ast.child_start.size() - 1, latex_text_syntax, unicode_text);

		// Restore the correct character sequence for the backwards-built AST string.
		std::reverse(latex_text_syntax.begin(), latex_text_syntax.end());
		std::reverse(unicode_text.begin(), unicode_text.end());

		std::string latex_text_string;
		latex_text_string.reserve(latex_text_syntax.size());
		for (int i = 0; i < latex_text_syntax.size(); ++i) {
			latex_text_string += latex_text_syntax[i];
		}

		std::string unicode_text_string;
		unicode_text_string.reserve(unicode_text.size());
		for (int i = 0; i < unicode_text.size(); ++i) {
			unicode_text_string += unicode_text[i];
		}

		latex_text_field = latex_text_string;
		unicode_text_field = unicode_text_string;
	}
public:
	ExpressionConverter(const ParserResult& ast)
		: ast(ast.value())
		, index_field(this->ast.child_start.size() - 1)
	{

	}

	void expr_convert() {
		to_latex_and_unicode_text();
		Console::print_to_center(latex_text_field, 0);
		Console::print_to_center(unicode_text_field, 0);
	}
};



enum class OpCode {
	ldc,
	add,
	sub,
	mul,
	div,
	pow,
	pct,
	neg,
	itofp,
	fpext,
	RET
};

enum class IRInstructionType {
	i32,
	f32,
	f64
};

using PayloadType = std::variant<std::monostate, int32_t, float, double>;

struct IRInstruction {
	PayloadType payload;
	OpCode opcode;
	IRInstructionType type;
	int32_t ssa_index;
	int32_t operands_start;
	int32_t operands_count;
	int32_t use_count;
	bool is_higher_precision = false;
};

class IRGenerator {
	AbstractSyntaxTree_SoA ast;

	std::vector<IRInstruction> ir_instructions;
	std::vector<ExpectedIndex> operands_pool;
	int i = 0;
	int32_t ssa_offset_from_casts = 0;

	[[nodiscard]] PayloadType string_to_number(std::string_view literal) const noexcept {
		PayloadType to_number;
		if (literal.contains('.')) {
			if (literal.size() <= 7) {
				to_number.emplace<float>();
			}
			else {
				to_number.emplace<double>();
			}
		}
		else {
			to_number.emplace<int32_t>();
		}

		std::visit([&literal](auto& val) {
			using T = std::decay_t<decltype(val)>;

			if constexpr (std::is_same_v<T, std::monostate>) {
				return;
			}
			else {
				std::from_chars(literal.data(), literal.data() + literal.size(), val);
			}
			}, to_number);

		return to_number;
	}

	void ssaindex_to_string(int32_t ssaindex, std::string& str) const {
		constexpr size_t max_chars = std::numeric_limits<int32_t>::digits10 + 2;
		char buffer[max_chars];
		auto [ptr, ec] = std::to_chars(buffer, buffer + max_chars, ssaindex);
		if (ec == std::errc{}) {
			str.append(buffer, ptr - buffer);
		}
	}

	[[nodiscard]] IRInstructionType get_type(const PayloadType& payload_value) const noexcept {
		IRInstructionType type;
		std::visit([&](const auto& payload_type) {
			using T = std::decay_t<decltype(payload_type)>;

			if constexpr (std::is_same_v<T, int32_t>) {
				type = IRInstructionType::i32;
			}
			else if constexpr (std::is_same_v<T, float>) {
				type = IRInstructionType::f32;
			}
			else if constexpr (std::is_same_v<T, double>) {
				type = IRInstructionType::f64;
			}
			}, payload_value);

		return type;
	}

	[[nodiscard]] bool right_child_is_higher_precision(const IRInstruction& left_child, const IRInstruction& right_child) const noexcept {
		IRInstructionType left_child_type = left_child.type;
		IRInstructionType right_child_type = right_child.type;

		if (right_child_type == IRInstructionType::i32) {
			return false;
		}
		else if (left_child_type == IRInstructionType::f64) {
			return false;
		}
		else if (left_child_type == IRInstructionType::i32) {
			return true;
		}
		else if (right_child_type == IRInstructionType::f64) {
			return true;
		}
	}

	[[nodiscard]] OpCode get_opcode(TypeOfNode node_type) const noexcept {
		switch (node_type) {
		case TypeOfNode::Addition:			return OpCode::add;
		case TypeOfNode::Subtraction:		return OpCode::sub;
		case TypeOfNode::Multiplication:	return OpCode::mul;
		case TypeOfNode::Division:			return OpCode::div;
		case TypeOfNode::Power:				return OpCode::pow;
		case TypeOfNode::Negation:			return OpCode::neg;
		case TypeOfNode::Percent:			return OpCode::pct;
		}
	}

	int32_t type_cast_IR_generate(IRInstruction& left_child, IRInstruction& right_child) {
		if (right_child_is_higher_precision(left_child, right_child)) {
			right_child.is_higher_precision = true;
			OpCode opcode_of_cast = (left_child.type == IRInstructionType::i32) ? OpCode::itofp : OpCode::fpext;
			ir_instructions.emplace_back(
				std::monostate{},
				opcode_of_cast,
				right_child.type,
				i + ssa_offset_from_casts,
				left_child.ssa_index,
				1,
				left_child.use_count,
				true
			);
		}
		else {
			left_child.is_higher_precision = true;
			OpCode opcode_of_cast = (right_child.type == IRInstructionType::i32) ? OpCode::itofp : OpCode::fpext;
			ir_instructions.emplace_back(
				std::monostate{},
				opcode_of_cast,
				left_child.type,
				i + ssa_offset_from_casts,
				right_child.ssa_index,
				1,
				right_child.use_count,
				true
			);
		}
		ssa_offset_from_casts++;
		return static_cast<int32_t>(ir_instructions.back().ssa_index);
	}

	void binary_op_IR_generate(IRInstruction& left_child, IRInstruction& right_child) {
		if (left_child.type != right_child.type) {
			int32_t new_operand = type_cast_IR_generate(left_child, right_child);
			if (left_child.is_higher_precision) {
				operands_pool[ast.child_start[i] + 1] = new_operand;
				right_child.type = left_child.type;
			}
			else {
				operands_pool[ast.child_start[i]] = new_operand;
				left_child.type = right_child.type;
			}
		}
		ir_instructions.push_back(
			IRInstruction(
				std::monostate{},
				get_opcode(ast.node_types[i]),
				left_child.type,
				i + ssa_offset_from_casts,
				ast.child_start[i],
				2,
				0
			)
		);
		ir_instructions[ast.child_start[i]].use_count++;
		ir_instructions[ast.child_start[i] + 1].use_count++;
	}

public:
	IRGenerator(const ParserResult& ast)
		: ast(ast.value())
		, operands_pool(ast.value().child_relationships)
	{

	}

	[[nodiscard]] std::vector<ExpectedIndex> get_operands_pool() const noexcept {
		return operands_pool;
	}

	[[nodiscard]] std::vector<IRInstruction> generate() {
		for (; i < ast.node_types.size(); ++i) {
			switch (ast.node_types[i]) {
			case TypeOfNode::Addition:
			case TypeOfNode::Subtraction:
			case TypeOfNode::Multiplication:
			case TypeOfNode::Division:
			case TypeOfNode::Power: {
				IRInstruction left_child = ir_instructions[operands_pool[ast.child_start[i]].value()];
				IRInstruction right_child = ir_instructions[operands_pool[ast.child_start[i] + 1].value()];
				binary_op_IR_generate(left_child, right_child);
				continue;
			}
			case TypeOfNode::Negation:
			case TypeOfNode::Percent: {
				ir_instructions.push_back(
					IRInstruction(
						std::monostate{},
						get_opcode(ast.node_types[i]), 
						IRInstructionType::i32,
						i + ssa_offset_from_casts, 
						ast.child_start[i], 
						1
					)
				);
				ir_instructions[ast.child_start[i]].use_count++;

				auto& current_ir = ir_instructions.back();
				current_ir.type = ir_instructions[operands_pool[current_ir.operands_start].value()].type;
				continue;
			}
			case TypeOfNode::Number: {
				auto numbered_payload = string_to_number(ast.node_data[i]);
				ir_instructions.push_back(IRInstruction(numbered_payload, OpCode::ldc, get_type(numbered_payload), i + ssa_offset_from_casts, -1, 0, 0));
				continue;
			}
			}
		}

		IRInstruction target_ret = ir_instructions.back();
		ir_instructions.emplace_back(
			std::monostate{},
			OpCode::RET,
			target_ret.type,
			-1,
			target_ret.ssa_index,
			1,
			0
		);

		return ir_instructions;
	}
	void print(const std::vector<IRInstruction>& ir_instructions) const {
		for (const auto& i : ir_instructions) {
			if (i.opcode == OpCode::RET) {
				std::println("{} v{}",
					magic_enum::enum_name(i.opcode),
					(ir_instructions.rbegin() + 1)->ssa_index
				);
				return;
			}
			else if (i.opcode == OpCode::itofp) {
				std::println("v{} = itofp i32 v{} to {}", i.ssa_index, i.operands_start, magic_enum::enum_name(i.type));
				continue;
			}
			else if (i.opcode == OpCode::fpext){
				std::println("v{} = fpext f32 v{} to f64", i.ssa_index, i.operands_start);
				continue;
			}

			std::print("v{} = {} {}", i.ssa_index, magic_enum::enum_name(i.opcode), magic_enum::enum_name(i.type));
			switch (i.operands_count) {
			case 0:
				std::visit([&](const auto& val) {
					using T = std::decay_t<decltype(val)>;

					if constexpr (std::is_same_v<T, std::monostate>) {
						return;
					}
					else {
						std::println(" {}", val);
					}
					}, i.payload);
				break;
			default: {
				std::string one_instruction;

				for (int op_i = i.operands_start; op_i < i.operands_start + i.operands_count; ++op_i) {
					one_instruction += (op_i == i.operands_start) ? " v" : ", v";
					ssaindex_to_string(ir_instructions[operands_pool[op_i].value()].ssa_index, one_instruction);
				}

				std::println("{}", one_instruction);
				break;
			}
			}
		}
	}
};


template<typename T>
concept ConstantType = std::same_as<T, int32_t> ||
					   std::same_as<T, float>   ||
					   std::same_as<T, double>;
class IROptimizer {
public:
	IROptimizer(const std::vector<IRInstruction>& ir_instructions, const std::vector<ExpectedIndex>& instructions_operands_pool, bitmask settings)
		: instructions(ir_instructions)
		, operands_pool(instructions_operands_pool)
		, current_settings(settings)
	{

	}

	using OptimizerResult = std::expected<std::vector<IRInstruction>, TypeOfError>;
	[[nodiscard]] OptimizerResult optimize() {
		for (auto& i : instructions) {
			if (ascii_symbols[get_opcode_symbol(i.opcode)] & MASK_OPERATOR) {
				switch (i.opcode) {
				case OpCode::add:
				case OpCode::sub:
				case OpCode::mul:
				case OpCode::div:
				case OpCode::pow: {
					auto correct_result = std::visit([this, &i](const auto& l, const auto& r) -> std::expected<PayloadType, TypeOfError> {
						using LeftT = std::decay_t<decltype(l)>;
						using RightT = std::decay_t<decltype(r)>;

						if constexpr (ConstantType<LeftT> && std::same_as<LeftT, RightT>) {
							return fold_constants<LeftT>(i.opcode, l, r);
						}

						return l;
						}, instructions[operands_pool[i.operands_start].value()].payload, instructions[operands_pool[i.operands_start + 1].value()].payload);
					
					if (!correct_result.has_value()) return std::unexpected(correct_result.error());

					instructions[i.ssa_index].payload = correct_result.value();
					instructions[i.ssa_index].opcode = OpCode::ldc;
					instructions[i.ssa_index].operands_count = 0;
					instructions[operands_pool[i.operands_start].value()].use_count = 0;
					instructions[operands_pool[i.operands_start + 1].value()].use_count = 0;
					break;
				}
				case OpCode::neg:
				case OpCode::pct: {
					auto correct_result = std::visit([this, &i](const auto& r) -> std::expected<PayloadType, TypeOfError> {
						using RightT = std::decay_t<decltype(r)>;

						if constexpr (ConstantType<RightT>) {
							return this->fold_constants<RightT>(i.opcode, r);
						}

						return r;
						}, instructions[operands_pool[i.operands_start].value()].payload);

					instructions[i.ssa_index].payload = correct_result.value();
					instructions[i.ssa_index].opcode = OpCode::ldc;
					instructions[i.ssa_index].operands_count = 0;
					instructions[operands_pool[i.operands_start].value()].use_count = 0;
					break;
				}
				}
			}
			else if (!(ascii_symbols[get_opcode_symbol(i.opcode)] & MASK_NUMBER)) {
				switch (i.opcode) {
				case OpCode::itofp: {
					auto& to_cast_payload = instructions[i.operands_start];
					instructions[i.ssa_index].payload = static_cast<float>(std::get<int32_t>(to_cast_payload.payload));
					to_cast_payload.use_count = 0;
					instructions[i.ssa_index].opcode = OpCode::ldc;
					instructions[i.ssa_index].operands_count = 0;
					break;
				}
				case OpCode::fpext: {
					auto& to_cast_payload = instructions[i.operands_start];
					instructions[i.ssa_index].payload = static_cast<double>(std::get<float>(to_cast_payload.payload));
					to_cast_payload.use_count = 0;
					instructions[i.ssa_index].opcode = OpCode::ldc;
					instructions[i.ssa_index].operands_count = 0;
					break;
				}
				case OpCode::RET:
					instructions[i.operands_start].use_count = 1;
					break;
				}
			}
		}

		dead_code_elimination();

		return instructions;
	}

	template<typename T>
	[[nodiscard]] std::expected <T, Errors> get_constant_expression_result(const OptimizerResult& optimized_ir) {
		if (!optimized_ir.has_value()) {
			error_handler.register_semantic(optimized_ir.error());
			return std::unexpected(std::move(error_handler.messages));
		}

		return std::visit([](const auto& res) -> T {
			using result_t = std::decay_t<decltype(res)>;

			if constexpr (std::is_same_v<result_t, std::monostate>) {
				return T{};
			}
			else if constexpr (std::is_arithmetic_v<result_t>) {
				return static_cast<T>(res);
			}
			else {
				return T{};
			}
			}, (instructions.rbegin() + 1)->payload);
	}


private:
	static constexpr std::array<bitmask, 256> ascii_symbols = lookup_table_fill();

	bitmask current_settings;

	std::vector<IRInstruction> instructions;
	std::vector<ExpectedIndex> operands_pool;

	ErrorHandler error_handler;

	template<ConstantType T>
	[[nodiscard]] std::expected<PayloadType, TypeOfError> fold_constants(OpCode op, T left_child, T right_child) const noexcept {
		switch (op) {
		case OpCode::add:
			return left_child + right_child;
			break;
		case OpCode::sub:
			return left_child - right_child;
			break;
		case OpCode::mul:
			return left_child * right_child;
			break;
		case OpCode::div:
			if (right_child == 0.0) return std::unexpected(TypeOfError::DivByZero);
			return left_child / right_child;
		case OpCode::pow:
			return std::pow(left_child, right_child);
		}
	}

	template<ConstantType T>
	[[nodiscard]] PayloadType fold_constants(OpCode op, T right_child) const noexcept {
		switch (op) {
		case OpCode::neg:
			return -right_child;
		case OpCode::pct:
			return right_child / 100.0;
		}
	}

	void dead_code_elimination() {
		for (auto i = instructions.begin(); i != instructions.end(); ) {
			if (i->opcode != OpCode::RET && i->use_count == 0) {
				i = instructions.erase(i);
			}
			else {
				++i;
			}
		}
	}

	[[nodiscard]] uint8_t get_opcode_symbol(OpCode current_op) const noexcept {
		switch (current_op) {
		case OpCode::add: return '+';
		case OpCode::sub: case OpCode::neg: return '-';
		case OpCode::mul: return '*';
		case OpCode::div: return '/';
		case OpCode::pow: return '^';
		case OpCode::pct: return '%';
		}
	}
};


int main()
{
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
#endif

	std::println("Welcome to all-math-gpu-calculator! This project was created for performing calculations from basic arithmetic to advanced mathematics.)");
	std::println("Enter: \n 1) 'exit' to close the program. \n 2) 'help' to show the expression printing guide and supported operators and symbols.\n");
	std::println("The entered mathematical expression will be displayed in a clean mathematical format.\n");

	Settings settings;
	bitmask current_settings = settings.load_settings();

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
			.transform([](const AbstractSyntaxTree_SoA& soa_ast) {
				std::println("\033[32m Math expression is valid!\033[0m");

				Console::print_to_center("3. LATEX- AND UNICODE TEXT.\n", 0);
				ExpressionConverter converter(soa_ast);
				converter.expr_convert();
				std::println();

				return soa_ast;
			})
			.and_then([current_settings](const AbstractSyntaxTree_SoA& soa_ast) {
				Console::print_to_center("4. IR GENERATION.\n", 0);

				IRGenerator ir_generator(soa_ast);
				std::vector<IRInstruction> instructions = ir_generator.generate();
				ir_generator.print(instructions);
				std::println();

				Console::print_to_center("5. IR OPTIMIZATION.\n", 0);
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

		Console::print_to_center("\033[32m 6. RESULT: ", 0);
		Console::print_to_center(std::string(std::format("{}\033[0m\n", program_pipeline.value())), 0);
	}

	return 0;
}




void print_help() {
	int width = Console::get_screen_width();

	std::print("╔");
	Console::print_divider("═");
	std::println("╗");
	Console::print_to_center("HELP", 0);
	std::print("╚");
	Console::print_divider("═");
	std::println("╝\n");


	Console::print_header(" [ 1. 🧮 SUPPORTED OPERATORS AND SYMBOLS. ] ");
	std::println("\033[36m FORMAT: [math object]          -        [name] \033[0m");
	Console::print_divider("-");
	std::println();


	// this list in std::array will change.

		// TODO:
			// General Math: ⁽ ⁾ ˣ ʸ ⁿ, √(and ³√, ⁴√, ⁵√), variables(𝑥, 𝑦), =, ≈, ƒ, log(and log₂, log₁₀), | (modules), <, >, ≤, ≥, ≠.
			// Trigonometry: °, π, sin, cos, tan, cotan, arcsin, arccos, arctan, atan2, arccotan, sec.
			// Calculus: lim, →, ℯ, ln, 𝒅, ′, ″, ∂, ∫, ∫∫, ∫∫∫, ∮, ₀ ₁ ₂ ₃ ₄ ₅ ₆ ₇ ₈ ₉ ₐ ₑ ₒ ₓ ₙ, Σ, ∏, ꝏ, ∞, ⁺, ₊, ⁻, ₋, ∇, Δ, δ.
			// Complex numbers: 𝑧, 𝑖, 𝑤, 𝑗, ℛℯ, ℐ𝓂, z̄, arg.


	constexpr std::array supported_symbols = {
		"+, -, ⋅, /         -         basic arithmetic operators",
		"𝑥ʸ                 -         power",
		"%                  -         percent"
	};
	for (const auto& i : supported_symbols) {
		std::println("{}", i);
	}

	std::println();



	Console::print_header(" [ 2. ✍️ MATH EXPRESSIONS PRINTING GUIDE. ] ");
	std::println(
		"ℹ️  \033[36m If you can't print Unicode math symbols (like 2⋅2)"
		"- you can enter them using the simple methods listed below, and they will be converted to Unicode.\n"
		"Here is the list of how to print symbols. \033[0m \n");

	std::println("\033[36m FORMAT: [Unicode symbol]        -        [simple method of printing] \033[0m");
	Console::print_divider("-");
	std::println();
	constexpr std::array guide = {
		"⋅            -          *",
		"𝑥ʸ           -          x^y",
	};
	for (const auto& i : guide) {
		std::println("{}", i);
	}
	std::println();



	Console::print_header(" [ 3. ⚙ SETTINGS. ] ");

	std::println("\033[36m FORMAT: [Settings name]        -        [value] \033[0m");
	Console::print_divider("-");
	std::println();
	constexpr std::array settings_list = {
		"Enable Fast Math mode            -          Apply algebraic simplifications using unsafe math optimizations (disregarding IEEE 754).",
		"𝑥ʸ           -          x^y",
	};
	for (const auto& i : guide) {
		std::println("{}", i);
	}
	std::println();



	Console::print_header(" [ 4. 📓 NOTICES. ] ");
	std::print("\033[36m");
	constexpr std::array notices = {
		"1) You can print negative numbers without parentheses even not at the beginning.",
		"2) You need to use parentheses if the exponent contains more than just a single constant or variable (for example, an operator).",
		"3) Unicode does not support nested exponents like LaTeX; it only supports one level(e. g. 2², but not 2²²).",
		"4) Programm does not support output LaTeX image in now."
	};
	for (const auto& i : notices) {
		std::println("{}", i);
	}
	std::println("\033[0m");



	Console::print_to_center("Enjoy using the program!", 0);
	Console::print_divider("━");
	std::println();
}