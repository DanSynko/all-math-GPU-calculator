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
#else
import std;
#endif



namespace TextFormatter {

	int get_screen_width() {
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
	uint32_t index = 0;
	TypeOfToken type = TypeOfToken::Default;
	std::string value;

	void setter(int num, TypeOfToken current_type, auto iterator_value) {
		type = current_type;
		index = num;
		value = iterator_value;
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
	InvalidFloatingPoint
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




using bitmask = uint8_t;

constexpr bitmask MASK_NUMBER = 1 << 0;
constexpr bitmask MASK_OPERATOR = 1 << 1;
constexpr bitmask MASK_PARENTHESIS = 1 << 2;
constexpr bitmask MASK_FLOATING_POINT = 1 << 3;
constexpr bitmask MASK_SPACE = 1 << 4;

constexpr std::array<bitmask, 256> lookup_table_fill() {
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

	symbols['('] = MASK_PARENTHESIS;
	symbols[')'] = MASK_PARENTHESIS;

	symbols[' '] = MASK_SPACE;

	return symbols;
}


class Lexer {
	static constexpr std::array<bitmask, 256> ascii_symbols = lookup_table_fill();
	bitmask math_operators = MASK_OPERATOR | MASK_PARENTHESIS;
	bitmask mask_negativesign = MASK_OPERATOR | MASK_PARENTHESIS | MASK_SPACE;

	ErrorHandler error_handler;

	std::string_view expr;
	std::string_view::const_iterator it;
	int i;
	bitmask current_symbol_type;
	uint8_t current_symbol_value;

	std::expected<Token, TypeOfError> number_tokenization() {
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

	SafeTokens tokenization() {
		std::vector<Token> tokens;

		for (; it != expr.end(); ++it) {
			current_symbol_value = static_cast<bitmask>(*it);
			if (ascii_symbols[current_symbol_value] == 0) {
				Token defect_token;
				defect_token.setter(i, TypeOfToken::Default, current_symbol_value);
				return std::unexpected(TypeOfError::UnknownSymbol);
			}

			current_symbol_type = ascii_symbols[current_symbol_value];
			if (current_symbol_type & MASK_SPACE) continue;

			if (current_symbol_type & math_operators) {
				Token a_operator;
				switch (current_symbol_value) {
				case '+':
					a_operator.setter(i, TypeOfToken::Plus, current_symbol_value);
					tokens.push_back(a_operator);
					break;
				case '-':
					if (it == expr.begin() || (ascii_symbols[*(std::prev(it))] & mask_negativesign)) {
						a_operator.setter(i, TypeOfToken::NegativeSign, '~');
						tokens.push_back(a_operator);
					}
					else {
						a_operator.setter(i, TypeOfToken::Minus, current_symbol_value);
						tokens.push_back(a_operator);
					}
					break;
				case '*':
					a_operator.setter(i, TypeOfToken::MultiplicationSign, current_symbol_value);
					tokens.push_back(a_operator);
					break;
				case '/':
					a_operator.setter(i, TypeOfToken::DivisionSign, current_symbol_value);
					tokens.push_back(a_operator);
					break;
				case '(':
					a_operator.setter(i, TypeOfToken::OpenParenthesis, current_symbol_value);
					tokens.push_back(a_operator);
					break;
				case ')':
					a_operator.setter(i, TypeOfToken::CloseParenthesis, current_symbol_value);
					tokens.push_back(a_operator);
					break;
				case '%':
					a_operator.setter(i, TypeOfToken::PercentSign, current_symbol_value);
					tokens.push_back(a_operator);
					break;
				case '^':
					a_operator.setter(i, TypeOfToken::PowerSign, current_symbol_value);
					tokens.push_back(a_operator);
					break;
				default:
					break;
				}
			}
			else if (current_symbol_type & MASK_NUMBER) {
				std::expected<Token, TypeOfError> number = number_tokenization();
				if (!number.has_value()) {
					Token defect_number;
					defect_number.setter(i, TypeOfToken::Default, current_symbol_value);
					error_handler.errors_list.push_back(ErrorMessage(defect_number, error_handler.error_message_templates(TypeOfError::InvalidFloatingPoint)));
					return std::unexpected(TypeOfError::InvalidFloatingPoint);
				}
				number->index = i;
				number->type = TypeOfToken::Number;
				tokens.push_back(number.value());
				--it;
			}
			++i;
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

	Lexer(std::string_view expr) : expr(expr), it(expr.begin()), i(0) {
		current_symbol_type = 0;
		current_symbol_value = 0;
	}

	LexerResult get_lexer_result() {
		SafeTokens tokens;
		tokens = tokenization();
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
	std::vector<std::string> node_data;
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



class ExpressionConverter {
	AbstractSyntaxTree_SoA ast;
	int32_t index_field;
	std::string latex_text_field;
	std::string unicode_text_field;

	bool parent_is_power = false;
	bool parent_is_stronger = false;

	std::string get_unicode_power_exponent(std::string& exponent) {
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
			expr_converter(--index_field, latex_text, unicode_text);
			latex_text.push_back(" \\frac{");
			break;
		case NodeTags::NegativeNum:
			expr_converter(--index_field, latex_text, unicode_text);
			latex_text.push_back("−");
			unicode_text.push_back("−");
			break;
		case NodeTags::Percent:
			expr_converter(--index_field, latex_text, unicode_text);
			latex_text.push_back(" \\,\\%");
			unicode_text.push_back("% ");
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
	ExpressionConverter(ParserResult& ast) {
		this->ast = (std::get<AbstractSyntaxTree_SoA>(ast));
		index_field = this->ast.child_start.size() - 1;
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
	neg
};

enum class IRInstructionType {
	i32,
	f32,
	f64
};

struct IRInstruction {
	OpCode op_code;
	int32_t ssa_index;
	int32_t operands_start;
	int32_t operands_count;
	std::variant<int32_t, float, double> payload;
	IRInstructionType type;
};

class IRGenerator {
	AbstractSyntaxTree_SoA ast;

	std::vector<IRInstruction> ir_instructions;
	std::vector<SafeUint32t> operands_pool;

	std::variant<int32_t, float, double> string_to_number(std::string_view string_const) {
		std::variant<int32_t, float, double> to_number;
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
			std::from_chars(string_const.data(), string_const.data() + string_const.size(), val);
			}, to_number);

		return to_number;
	}

	void ssaindex_to_string(int32_t ssaindex, std::string& str) {
		constexpr size_t max_chars = std::numeric_limits<int32_t>::digits10 + 2;
		char buffer[max_chars];
		auto [ptr, ec] = std::to_chars(buffer, buffer + max_chars, ssaindex);
		if (ec == std::errc{}) {
			str.append(buffer, ptr - buffer);
		}
	}

	IRInstructionType get_type(const std::variant<int32_t, float, double>& numbered_payload) {
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

	IRInstructionType get_type(IRInstructionType& left_child_type, IRInstructionType& right_child_type) {
		if (left_child_type != right_child_type) {
			if (right_child_type == IRInstructionType::i32) {
				right_child_type = left_child_type;
				return left_child_type;
			}
			else if (left_child_type == IRInstructionType::f64) {
				right_child_type = IRInstructionType::f64;
				return IRInstructionType::f64;
			}
			else if (left_child_type == IRInstructionType::i32) {
				left_child_type = right_child_type;
				return right_child_type;
			}
			else if (right_child_type == IRInstructionType::f64) {
				left_child_type = IRInstructionType::f64;
				return IRInstructionType::f64;
			}
		}
		else {
			return right_child_type;
		}
	}

	OpCode get_opcode(NodeTags node_tage) const {
		switch (node_tage) {
		case NodeTags::Add: return OpCode::add;
		case NodeTags::Subtract: return OpCode::sub;
		case NodeTags::Multiply: return OpCode::mul;
		case NodeTags::Divide: return OpCode::div;
		case NodeTags::Power: return OpCode::pow;
		case NodeTags::NegativeNum: return OpCode::neg;
		case NodeTags::Percent: return OpCode::pct;
		}
	}
public:
	IRGenerator(ParserResult& ast) {
		this->ast = std::get<AbstractSyntaxTree_SoA>(ast);
		operands_pool = std::move(std::get<AbstractSyntaxTree_SoA>(ast).child_relationships);
	}

	void IR_generate() {
		for (int i = 0; i < ast.node_tags.size(); ++i) {
			switch (ast.node_tags[i]) {
			case NodeTags::Add:
			case NodeTags::Subtract:
			case NodeTags::Multiply:
			case NodeTags::Divide:
			case NodeTags::Power: {
				ir_instructions.push_back(IRInstruction(get_opcode(ast.node_tags[i]), i, ast.child_start[i], 2));
				auto& current_ir = ir_instructions.back();
				IRInstructionType left_child_type = ir_instructions[operands_pool[current_ir.operands_start].value()].type;
				IRInstructionType right_child_type = ir_instructions[operands_pool[current_ir.operands_start + 1].value()].type;
				current_ir.type = get_type(left_child_type, right_child_type);
				continue;
			}
			case NodeTags::NegativeNum:
			case NodeTags::Percent: {
				ir_instructions.push_back(IRInstruction(get_opcode(ast.node_tags[i]), i, ast.child_start[i], 1));
				auto& current_ir = ir_instructions.back();
				current_ir.type = ir_instructions[operands_pool[current_ir.operands_start].value()].type;
				continue;
			}
			case NodeTags::Number: {
				auto numbered_payload = string_to_number(ast.node_data[i]);
				ir_instructions.push_back(IRInstruction(OpCode::ldc, i, -1, 0, numbered_payload, get_type(numbered_payload)));
				continue;
			}
			}
		}
	}
	void IR_print() {
		for (auto& i : ir_instructions) {
			std::print("v{} = {} {}", i.ssa_index, magic_enum::enum_name(i.op_code), magic_enum::enum_name(i.type));

			switch (i.operands_count) {
			case 0:
				std::visit([&](const auto& val) {
					std::println(" {}", val);
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


int main()
{
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
#endif

	std::println("Welcome to all-math-gpu-calculator! This project was created for performing calculations from basic arithmetic to advanced mathematics.)");
	std::println("Enter: \n 1) 'exit' to close the program. \n 2) 'help' to show the expression printing guide and supported operators and symbols.\n");
	std::println("The entered mathematical expression will be displayed in a clean mathematical format.\n");

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

		Lexer lexer(m_expr);
		LexerResult tokens = lexer.get_lexer_result();

		if (std::holds_alternative<std::vector<ErrorMessage>>(tokens)) {
			std::println("\033[1;31m Math expression is invalid. The unexpected tokens:");
			for (auto& it : std::get<std::vector<ErrorMessage>>(tokens)) {
				it.defect_token.print();
				std::println("{}\033[0m\n", it.message);
			}
			continue;
		}

		std::println("┌──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐");
		TextFormatter::print_to_center("1. TOKENS", 0);
		for (const Token& token : std::get<std::vector<Token>>(tokens)) {
			token.print();
		}
		std::println();

		TextFormatter::print_to_center("2. ABSTRACT SYNTAX TREE VALIDATION.", 0);
		PrattParser parser(std::get<std::vector<Token>>(tokens));
		parser.parse_expression(0);
		ParserResult soa_ast = parser.get_parser_result();

		if (std::holds_alternative<std::vector<ErrorMessage>>(soa_ast)) {
			std::println("\033[1;31m Math expression is invalid. The unexpected tokens:");
			for (auto& it : std::get<std::vector<ErrorMessage>>(soa_ast)) {
				it.defect_token.print();
				std::println("{}\033[0m\n\n", it.message);
			}
			continue;
		}
		else {
			std::println("\033[32m Math expression is valid!\033[0m");
		}
		TextFormatter::print_to_center("3. LATEX- AND UNICODE TEXT.\n", 0);
		ExpressionConverter converter(soa_ast);
		converter.expr_convert();
		std::println();

		TextFormatter::print_to_center("4. IR GENERATION.\n", 0);

		std::vector<IRInstruction> ir_instructions;

		IRGenerator ir_generator(soa_ast);
		ir_generator.IR_generate();
		ir_generator.IR_print();
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
			// General Math: ⁽ ⁾ ˣ ʸ ⁿ, √(and ³√, ⁴√, ⁵√), variables(𝑥, 𝑦), =, ≈, ƒ, log(and log₂, log₁₀), | (modules).
			// Trigonometry: °, π, sin, cos, tan, cotan, arcsin, arccos, arctan, atan2, arccotan, sec.
			// Calculus: lim, →, ℯ, ln, 𝒅, ′, ″, ∂, ∫, ∫∫, ∫∫∫, ∮, ₀ ₁ ₂ ₃ ₄ ₅ ₆ ₇ ₈ ₉ ₐ ₑ ₒ ₓ ₙ, Σ, ∏, ꝏ, ∞, ⁺, ₊, ⁻, ₋.
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


	TextFormatter::print_header(" [ 3. 📓 NOTICES. ] ");
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