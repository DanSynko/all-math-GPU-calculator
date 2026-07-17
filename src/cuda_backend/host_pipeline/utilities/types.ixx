module;
#include "third-party/magic_enum.hpp"

export module types;

#ifdef __INTELLISENSE__
#include <cstdint>
#include <vector>
#include <string>
#include <optional>
#include <print>
#include <expected>
#include <variant>
#else
import std;
#endif

export {
	using bitmask = uint8_t;
	enum Bitmask : bitmask {
		fast_math_mode = 1 << 0
	};

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
		void print() const {
			std::println(" [index: {:<13} type: {:<23} value: {:<1} ]",
				std::to_string(index) + ";",
				std::string(magic_enum::enum_name(type)) + ";",
				value
			);
		}

		int32_t index = 0;
		TypeOfToken type{};
		std::string_view value;
	};

	enum class ErrorType {
		NoInput,
		UnknownSymbol,
		ParenthesesImbalance,
		InvalidOperator,
		UnexpectedEnd,
		UnexpectedNumber,
		InvalidPrefixOperator,
		InvalidFloatingPoint,
		DivByZero,
		UnsupportedComplex
	};
	struct ErrorMessage {
		ErrorType error_type;
		std::string_view message;
		std::optional<Token> defect_token;
	};

	using Errors = std::vector<ErrorMessage>;
	using ExpectedTokens = std::expected<std::vector<Token>, Errors>;
	using ExpectedIndex = std::expected<int32_t, ErrorType>;
	using PayloadType = std::variant<std::monostate, int32_t, float, double>;

	enum class NodeType {
		Number,
		Addition,
		Subtraction,
		Multiplication,
		Division,
		Negation,
		Power,
		Percent
	};

	class AbstractSyntaxTree_SoA {
	public:
		[[nodiscard]] int32_t add_node(const Token& token) {
			node_types.push_back(typeoftoken_to_typeofnode(token.type));
			node_data.emplace_back(token.value.data(), token.value.size());
			child_start.push_back(-1);
			child_count.push_back(0);
			return node_types.size() - 1;
		}

		[[nodiscard]] int32_t add_node(const Token& token, const ExpectedIndex& right_child_index) {
			node_types.push_back(typeoftoken_to_typeofnode(token.type));
			node_data.emplace_back(token.value.data(), token.value.size());
			child_start.push_back(child_relationships.size());
			child_count.push_back(1);
			child_relationships.push_back(right_child_index);
			return node_types.size() - 1;
		}

		[[nodiscard]] int32_t add_node(const Token& token, const ExpectedIndex& left_child_index, const ExpectedIndex& right_child_index) {
			node_types.push_back(typeoftoken_to_typeofnode(token.type));
			node_data.emplace_back(token.value.data(), token.value.size());
			child_start.push_back(child_relationships.size());
			child_count.push_back(2);
			child_relationships.push_back(left_child_index);
			child_relationships.push_back(right_child_index);
			return node_types.size() - 1;
		}

		std::vector<NodeType> node_types;
		std::vector<std::string_view> node_data;
		std::vector<ExpectedIndex> child_relationships;
		std::vector<int32_t> child_start;
		std::vector<int32_t> child_count;
	private:
		[[nodiscard]] NodeType typeoftoken_to_typeofnode(TypeOfToken token_type) const noexcept {
			switch (token_type) {
			case TypeOfToken::Number:
				return NodeType::Number;
			case TypeOfToken::Plus:
				return NodeType::Addition;
			case TypeOfToken::Minus:
				return NodeType::Subtraction;
			case TypeOfToken::MultiplicationSign:
				return NodeType::Multiplication;
			case TypeOfToken::DivisionSign:
				return NodeType::Division;
			case TypeOfToken::NegationSign:
				return NodeType::Negation;
			case TypeOfToken::PowerSign:
				return NodeType::Power;
			case TypeOfToken::PercentSign:
				return NodeType::Percent;
			}
		}
	};

	using ParserResult = std::expected<AbstractSyntaxTree_SoA, Errors>;

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

	struct IRInstruction {
		PayloadType payload;
		OpCode opcode{};
		IRInstructionType type{};
		int32_t ssa_index = 0;
		int32_t operands_start = 0;
		int32_t operands_count = 0;
		int32_t use_count = 0;
		bool is_higher_precision = false;
	};
}