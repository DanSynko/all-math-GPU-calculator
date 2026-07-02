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


namespace TextFormatter {

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
		TextFormatter::print_divider(header_element, TextFormatter::get_screen_width() / 3);
		std::print("{}", text);
		TextFormatter::print_divider(header_element, TextFormatter::get_screen_width() / 3);
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



void help_command();



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


struct ErrorHandler {
	std::vector<ErrorMessage> errors_list;

	[[nodiscard]] std::string error_message_templates(TypeOfError error_type) const noexcept {
		switch (error_type) {
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
		default:
			return "Unknown error\n";
		}
	}

	void token_error_register(const Token& defect_token, TypeOfError error_type) {
		errors_list.emplace_back(error_message_templates(error_type), defect_token);
	}

	void semantic_error_register(TypeOfError error_type) {
		errors_list.emplace_back(error_message_templates(error_type));
	}

	[[nodiscard]] std::vector<ErrorMessage> panic_mode(const std::vector<Token>& tokens, int i, TypeOfError left_error, int defect_index) {
		token_error_register(tokens[defect_index], left_error);
		while (tokens[i].type != TypeOfToken::EndOfFile) {
			i++;
		}
		return errors_list;
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


using SafeTokens = std::expected<std::vector<Token>, TypeOfError>;
using LexerResult = std::expected<std::vector<Token>, std::vector<ErrorMessage>>;

class Lexer {
	static constexpr std::array<bitmask, 256> ascii_symbols = lookup_table_fill();
	bitmask math_operators = MASK_OPERATOR;
	bitmask mask_parentheses = MASK_OPEN_PARENTHESIS | MASK_CLOSE_PARENTHESIS;

	ErrorHandler error_handler;

	std::string_view expr;
	std::string_view::const_iterator it;
	int i = 0;
	bitmask current_symbol_type = 0;
	uint8_t current_symbol_value = 0;

	std::vector<Token> tokens;

	[[nodiscard]] std::expected<Token, TypeOfError> number_tokenization() {
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

	[[nodiscard]] bool is_negativesign() const noexcept {
		if (tokens.empty()) {
			return true;
		}
		const char prev_symbol = tokens[tokens.size() - 1].value[0];
		const bool is_prev_token_an_operator = (ascii_symbols[prev_symbol] & (math_operators | MASK_OPEN_PARENTHESIS));
		if (is_prev_token_an_operator) {
			return true;
		}
		return false;
	}

	SafeTokens tokenization() {
		for (; it != expr.end(); ++it) {
			current_symbol_value = static_cast<bitmask>(*it);
			if (ascii_symbols[current_symbol_value] == 0) {
				Token defect_token(i, TypeOfToken::Default, current_symbol_value);
				error_handler.token_error_register(defect_token, TypeOfError::UnknownSymbol);
				return std::unexpected(TypeOfError::UnknownSymbol);
			}

			current_symbol_type = ascii_symbols[current_symbol_value];
			if (current_symbol_type & MASK_SPACE) continue;

			if (current_symbol_type & (math_operators | mask_parentheses)) {
				switch (current_symbol_value) {
				case '+':
					tokens.emplace_back(i, TypeOfToken::Plus, current_symbol_value);
					break;
				case '-':
					if (is_negativesign()) {
						tokens.emplace_back(i, TypeOfToken::NegativeSign, '~');
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
				default:
					break;
				}
			}
			else if (current_symbol_type & MASK_NUMBER) {
				std::expected<Token, TypeOfError> number = number_tokenization();
				if (!number.has_value()) {
					Token defect_number(i, TypeOfToken::Default, current_symbol_value);
					error_handler.token_error_register(defect_number, TypeOfError::InvalidFloatingPoint);
					return std::unexpected(TypeOfError::InvalidFloatingPoint);
				}
				number->index = i;
				number->type = TypeOfToken::Number;
				tokens.push_back(number.value());
				--it;
			}
			++i;
		}

		tokens.emplace_back(i, TypeOfToken::EndOfFile, '\0');

		if (tokens[0].type == TypeOfToken::EndOfFile) {
			error_handler.token_error_register(tokens[0], TypeOfError::NoInput);
			return std::unexpected(TypeOfError::NoInput);
		}

		return tokens;
	}
public:
	Lexer(std::string_view expr) : expr(expr), it(expr.begin()) {}

	[[nodiscard]] LexerResult get_lexer_result() {
		SafeTokens tokens = tokenization();
		if (!tokens.has_value()) {
			return std::unexpected(std::move(error_handler.errors_list));
		}
		return tokens.value();
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

[[nodiscard]] NodeTags typeoftoken_to_nodetags(TypeOfToken token_type) noexcept {
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



using SafeInt32t = std::expected<int32_t, TypeOfError>;


struct AbstractSyntaxTree_SoA {
	std::vector<NodeTags> node_tags;
	std::vector<std::string> node_data;
	std::vector<SafeInt32t> child_relationships;
	std::vector<int32_t> child_start;
	std::vector<int32_t> child_count;

	[[nodiscard]] int32_t add_node(const Token& some_token) {
		node_tags.push_back(typeoftoken_to_nodetags(some_token.type));
		node_data.push_back(some_token.value);
		child_start.push_back(-1);
		child_count.push_back(0);
		return node_tags.size() - 1;
	}

	[[nodiscard]] int32_t add_node(const Token& some_token, const SafeInt32t& right_child_index) {
		node_tags.push_back(typeoftoken_to_nodetags(some_token.type));
		node_data.push_back(some_token.value);
		child_start.push_back(child_relationships.size());
		child_count.push_back(1);
		child_relationships.push_back(right_child_index);
		return node_tags.size() - 1;
	}

	[[nodiscard]] int32_t add_node(const Token& some_token, const SafeInt32t& left_child_index, const SafeInt32t& right_child_index) {
		node_tags.push_back(typeoftoken_to_nodetags(some_token.type));
		node_data.push_back(some_token.value);
		child_start.push_back(child_relationships.size());
		child_count.push_back(2);
		child_relationships.push_back(left_child_index);
		child_relationships.push_back(right_child_index);
		return node_tags.size() - 1;
	}
};


using ParserResult = std::expected<AbstractSyntaxTree_SoA, std::vector<ErrorMessage>>;

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

	[[nodiscard]] SafeInt32t NUD(const Token& token) {
		if (token.type == TypeOfToken::NegativeSign) {
			SafeInt32t operand = parse_expression(3);
			return ast.value().add_node(token, operand);
		}
		else if (token.type == TypeOfToken::OpenParenthesis) {
			int current_openp_index = i;
			SafeInt32t open_p = parse_expression(0);
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

	[[nodiscard]] SafeInt32t LED(const Token& token, const SafeInt32t& left) {
		SafeInt32t right;
		switch (token.type) {
		case TypeOfToken::PercentSign:
			return ast.value().add_node(token, left);
		case TypeOfToken::PowerSign:
			right = parse_expression(lookahead_lbp(token.type) - 1);
			return ast.value().add_node(token, left, right);
		case TypeOfToken::NegativeSign:
			return std::unexpected(TypeOfError::InvalidPrefixOperator);
		default:
			right = parse_expression(lookahead_lbp(token.type));
			return ast.value().add_node(token, left, right);
		}
	}

public:
	PrattParser(const std::vector<Token>& some_tokens) : tokens(some_tokens) {
		error_handler.errors_list.reserve(expr_strings_count);
	}

	SafeInt32t parse_expression(int rbp) {
		if (i == tokens.size()) return 0;

		Token current_token = tokens[i];

		i++;

		auto left = NUD(current_token);
		if (!left.has_value()) {
			i--;
			if (error_handler.errors_list.size() < expr_strings_count) {
				int defect_index = (left.error() == TypeOfError::ParenthesesImbalance) ? openp_index : i;
				std::vector<ErrorMessage> parse_errors = error_handler.panic_mode(tokens, i, left.error(), defect_index);
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

	[[nodiscard]] ParserResult get_parser_result() noexcept {
		SafeInt32t parser_result = parse_expression(0);
		if (i != tokens.size() - 1) {
			error_handler.token_error_register(tokens[i], TypeOfError::UnexpectedEnd);
		}
		if (!error_handler.errors_list.empty()) {
			return std::unexpected(error_handler.errors_list);
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
		switch (ast.node_tags[index_field]) {
		case NodeTags::Add:
			expr_converter(--index_field, latex_text, unicode_text);
			latex_text.push_back(" + ");
			unicode_text.push_back(parent_is_power ? "⁺" : " + ");
			expr_converter(--index_field, latex_text, unicode_text);
			break;
		case NodeTags::Subtract:
			expr_converter(--index_field, latex_text, unicode_text);
			latex_text.push_back(" − ");
			unicode_text.push_back(parent_is_power ? "⁻" : " − ");
			expr_converter(--index_field, latex_text, unicode_text);
			break;
		case NodeTags::Multiply:
			expr_converter(--index_field, latex_text, unicode_text);
			latex_text.push_back(" \\cdot ");
			unicode_text.push_back(" ⋅ ");
			expr_converter(--index_field, latex_text, unicode_text);
			break;
		case NodeTags::Divide:
			latex_text.push_back("}");
			expr_converter(--index_field, latex_text, unicode_text);
			latex_text.push_back("}{");
			unicode_text.push_back(" / ");
			expr_converter(--index_field, latex_text, unicode_text);
			latex_text.push_back("\\frac{");
			break;
		case NodeTags::NegativeNum:
			expr_converter(--index_field, latex_text, unicode_text);
			latex_text.push_back("−");
			unicode_text.push_back("−");
			break;
		case NodeTags::Percent:
			latex_text.push_back("\\,\\%");
			unicode_text.push_back("%");
			expr_converter(--index_field, latex_text, unicode_text);
			break;
		case NodeTags::Power:
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
		TextFormatter::print_to_center(latex_text_field, 0);
		TextFormatter::print_to_center(unicode_text_field, 0);
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
	OpCode op_code;
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
	std::vector<SafeInt32t> operands_pool;
	int i = 0;
	int32_t ssa_offset_from_casts = 0;

	[[nodiscard]] PayloadType string_to_number(std::string_view string_const) const noexcept {
		PayloadType to_number;
		if (string_const.contains('.')) {
			if (string_const.size() <= 7) {
				to_number.emplace<float>();
			}
			else {
				to_number.emplace<double>();
			}
		}
		else {
			to_number.emplace<int32_t>();
		}

		std::visit([&string_const](auto& val) {
			using T = std::decay_t<decltype(val)>;

			if constexpr (std::is_same_v<T, std::monostate>) {
				return;
			}
			else {
				std::from_chars(string_const.data(), string_const.data() + string_const.size(), val);
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

	[[nodiscard]] IRInstructionType get_type(const PayloadType& numbered_payload) const noexcept {
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
			}, numbered_payload);

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

	[[nodiscard]] OpCode get_opcode(NodeTags node_tage) const noexcept {
		switch (node_tage) {
		case NodeTags::Add:			return OpCode::add;
		case NodeTags::Subtract:	return OpCode::sub;
		case NodeTags::Multiply:	return OpCode::mul;
		case NodeTags::Divide:		return OpCode::div;
		case NodeTags::Power:		return OpCode::pow;
		case NodeTags::NegativeNum: return OpCode::neg;
		case NodeTags::Percent:		return OpCode::pct;
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
		return static_cast<int32_t>(ir_instructions.size() - 1);
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
				get_opcode(ast.node_tags[i]),
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

	[[nodiscard]] std::vector<SafeInt32t> get_operands_pool() const noexcept {
		return operands_pool;
	}

	[[nodiscard]] std::vector<IRInstruction> IR_generate() {
		for (; i < ast.node_tags.size(); ++i) {
			switch (ast.node_tags[i]) {
			case NodeTags::Add:
			case NodeTags::Subtract:
			case NodeTags::Multiply:
			case NodeTags::Divide:
			case NodeTags::Power: {
				IRInstruction left_child = ir_instructions[operands_pool[ast.child_start[i]].value()];
				IRInstruction right_child = ir_instructions[operands_pool[ast.child_start[i] + 1].value()];
				binary_op_IR_generate(left_child, right_child);
				continue;
			}
			case NodeTags::NegativeNum:
			case NodeTags::Percent: {
				ir_instructions.push_back(
					IRInstruction(
						std::monostate{},
						get_opcode(ast.node_tags[i]), 
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
			case NodeTags::Number: {
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
	void IR_print(const std::vector<IRInstruction>& ir_instructions) const {
		for (const auto& i : ir_instructions) {
			if (i.op_code == OpCode::RET) {
				std::println("{} v{}",
					magic_enum::enum_name(i.op_code),
					(ir_instructions.rbegin() + 1)->ssa_index
				);
				return;
			}
			else if (i.op_code == OpCode::itofp) {
				std::println("v{} = itofp i32 v{} to {}", i.ssa_index, i.operands_start, magic_enum::enum_name(i.type));
				continue;
			}
			else if (i.op_code == OpCode::fpext){
				std::println("v{} = fpext f32 v{} to f64", i.ssa_index, i.operands_start);
				continue;
			}

			std::print("v{} = {} {}", i.ssa_index, magic_enum::enum_name(i.op_code), magic_enum::enum_name(i.type));
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
	IROptimizer(const std::vector<IRInstruction>& ir_instructions, const std::vector<SafeInt32t>& instructions_operands_pool, bitmask settings)
		: instructions(ir_instructions)
		, operands_pool(instructions_operands_pool)
		, current_settings(settings)
	{

	}

	using OptimizerResult = std::expected<std::vector<IRInstruction>, TypeOfError>;
	[[nodiscard]] OptimizerResult ir_optimize() {
		for (auto& i : instructions) {
			if (ascii_symbols[get_opcode_symbol(i.op_code)] & MASK_OPERATOR) {
				switch (i.op_code) {
				case OpCode::add:
				case OpCode::sub:
				case OpCode::mul:
				case OpCode::div:
				case OpCode::pow: {
					auto correct_result = std::visit([this, &i](const auto& l, const auto& r) -> std::expected<PayloadType, TypeOfError> {
						using LeftT = std::decay_t<decltype(l)>;
						using RightT = std::decay_t<decltype(r)>;

						if constexpr (ConstantType<LeftT> && std::same_as<LeftT, RightT>) {
							return fold_constants<LeftT>(i.op_code, l, r);
						}

						return l;
						}, instructions[operands_pool[i.operands_start].value()].payload, instructions[operands_pool[i.operands_start + 1].value()].payload);
					
					if (!correct_result.has_value()) return std::unexpected(correct_result.error());

					instructions[i.ssa_index].payload = correct_result.value();
					instructions[i.ssa_index].op_code = OpCode::ldc;
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
							return this->fold_constants<RightT>(i.op_code, r);
						}

						return r;
						}, instructions[operands_pool[i.operands_start].value()].payload);

					instructions[i.ssa_index].payload = correct_result.value();
					instructions[i.ssa_index].op_code = OpCode::ldc;
					instructions[i.ssa_index].operands_count = 0;
					instructions[operands_pool[i.operands_start].value()].use_count = 0;
					break;
				}
				}
			}
			else if (!(ascii_symbols[get_opcode_symbol(i.op_code)] & MASK_NUMBER)) {
				switch (i.op_code) {
				case OpCode::itofp: {
					auto& to_cast_payload = instructions[i.operands_start];
					instructions[i.ssa_index].payload = static_cast<float>(std::get<int32_t>(to_cast_payload.payload));
					to_cast_payload.use_count = 0;
					instructions[i.ssa_index].op_code = OpCode::ldc;
					instructions[i.ssa_index].operands_count = 0;
					break;
				}
				case OpCode::fpext: {
					auto& to_cast_payload = instructions[i.operands_start];
					instructions[i.ssa_index].payload = static_cast<double>(std::get<float>(to_cast_payload.payload));
					to_cast_payload.use_count = 0;
					instructions[i.ssa_index].op_code = OpCode::ldc;
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
	[[nodiscard]] std::expected <T, std::vector<ErrorMessage>> get_constant_expression_result(const OptimizerResult& optimized_ir) {
		if (!optimized_ir.has_value()) {
			error_handler.semantic_error_register(optimized_ir.error());
			return std::unexpected(std::move(error_handler.errors_list));
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
	std::vector<IRInstruction> optimized_instructions;
	std::vector<SafeInt32t> operands_pool;

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
			if (i->op_code != OpCode::RET && i->use_count == 0) {
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

		std::string m_expr;

		std::getline(std::cin, m_expr);

		if (m_expr == "help") {
			help_command();
			continue;
		}
		else if (m_expr == "exit") {
			break;
		}

		Lexer lexer(std::move(m_expr));

		std::expected<double, std::vector<ErrorMessage>> program_pipeline = lexer.get_lexer_result()
			.transform([](const std::vector<Token>& correct_tokens) {
				std::println("┌──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐");
				TextFormatter::print_to_center("1. TOKENS", 0);

				for (const Token& token : correct_tokens) {
					token.print();
				}
				std::println();

				return correct_tokens;
			})
			.and_then([](const std::vector<Token>& correct_tokens) {
				TextFormatter::print_to_center("2. ABSTRACT SYNTAX TREE VALIDATION.", 0);

				PrattParser parser(correct_tokens);

				return parser.get_parser_result();
			})
			.transform([](const AbstractSyntaxTree_SoA& soa_ast) {
				std::println("\033[32m Math expression is valid!\033[0m");

				TextFormatter::print_to_center("3. LATEX- AND UNICODE TEXT.\n", 0);
				ExpressionConverter converter(soa_ast);
				converter.expr_convert();
				std::println();

				return soa_ast;
			})
			.and_then([current_settings](const AbstractSyntaxTree_SoA& soa_ast) {
				TextFormatter::print_to_center("4. IR GENERATION.\n", 0);

				IRGenerator ir_generator(soa_ast);
				std::vector<IRInstruction> instructions = ir_generator.IR_generate();
				ir_generator.IR_print(instructions);
				std::println();

				TextFormatter::print_to_center("5. IR OPTIMIZATION.\n", 0);
				IROptimizer ir_optimizer(std::move(instructions), std::move(ir_generator.get_operands_pool()), current_settings);
				auto optimized_ir = ir_optimizer.ir_optimize();
				ir_generator.IR_print(std::move(optimized_ir).value_or(std::vector<IRInstruction>{}));

				return ir_optimizer.get_constant_expression_result<double>(optimized_ir);
			})
			.transform_error([](const std::vector<ErrorMessage>& errors) {
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

		TextFormatter::print_to_center("\033[32m 6. RESULT: ", 0);
		TextFormatter::print_to_center(std::string(std::format("{}\033[0m\n", program_pipeline.value())), 0);
	}

	return 0;
}




void help_command() {
	int width = TextFormatter::get_screen_width();

	std::print("╔");
	TextFormatter::print_divider("═");
	std::println("╗");
	TextFormatter::print_to_center("HELP", 0);
	std::print("╚");
	TextFormatter::print_divider("═");
	std::println("╝\n");


	TextFormatter::print_header(" [ 1. 🧮 SUPPORTED OPERATORS AND SYMBOLS. ] ");
	std::println("\033[36m FORMAT: [math object]          -        [name] \033[0m");
	TextFormatter::print_divider("-");
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



	TextFormatter::print_header(" [ 2. ✍️ MATH EXPRESSIONS PRINTING GUIDE. ] ");
	std::println(
		"ℹ️  \033[36m If you can't print Unicode math symbols (like 2⋅2)"
		"- you can enter them using the simple methods listed below, and they will be converted to Unicode.\n"
		"Here is the list of how to print symbols. \033[0m \n");

	std::println("\033[36m FORMAT: [Unicode symbol]        -        [simple method of printing] \033[0m");
	TextFormatter::print_divider("-");
	std::println();
	constexpr std::array guide = {
		"⋅            -          *",
		"𝑥ʸ           -          x^y",
	};
	for (const auto& i : guide) {
		std::println("{}", i);
	}
	std::println();



	TextFormatter::print_header(" [ 3. ⚙ SETTINGS. ] ");

	std::println("\033[36m FORMAT: [Settings name]        -        [value] \033[0m");
	TextFormatter::print_divider("-");
	std::println();
	constexpr std::array settings_list = {
		"Enable Fast Math mode            -          Apply algebraic simplifications using unsafe math optimizations (disregarding IEEE 754).",
		"𝑥ʸ           -          x^y",
	};
	for (const auto& i : guide) {
		std::println("{}", i);
	}
	std::println();



	TextFormatter::print_header(" [ 4. 📓 NOTICES. ] ");
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



	TextFormatter::print_to_center("Enjoy using the program!", 0);
	TextFormatter::print_divider("━");
	std::println();
}