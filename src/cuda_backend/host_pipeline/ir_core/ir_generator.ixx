export module ir_generator;
#include "third-party/magic_enum.hpp"

#ifdef __INTELLISENSE__
#include <vector>
#include <print>
#include <string>
#else
import std;
#endif

import types;

export class IRGenerator {
public:
	explicit IRGenerator(const ParserResult& ast)
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
			case NodeType::Addition:
			case NodeType::Subtraction:
			case NodeType::Multiplication:
			case NodeType::Division:
			case NodeType::Power: {
				IRInstruction left_child = ir_instructions[operands_pool[ast.child_start[i]].value()];
				IRInstruction right_child = ir_instructions[operands_pool[ast.child_start[i] + 1].value()];
				binary_op_IR_generate(left_child, right_child);
				continue;
			}
			case NodeType::Negation:
			case NodeType::Percent: {
				ir_instructions.emplace_back(
					std::monostate{},
					get_opcode(ast.node_types[i]),
					IRInstructionType::f64,
					i,
					ast.child_start[i],
					1,
					0
				);
				auto& current_ir = ir_instructions.back();
				current_ir.type = ir_instructions[operands_pool[current_ir.operands_start].value()].type;
				continue;
			}
			case NodeType::Number: {
				auto numbered_payload = string_to_number(ast.node_data[i]);
				ir_instructions.emplace_back(numbered_payload, OpCode::ldc, IRInstructionType::f64, i, -1, 0, 0);
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
			else if (i.opcode == OpCode::fpext) {
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
private:
	[[nodiscard]] PayloadType string_to_number(std::string_view literal) const noexcept {
		PayloadType to_number;

		to_number.emplace<double>();

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

	[[nodiscard]] OpCode get_opcode(NodeType node_type) const noexcept {
		switch (node_type) {
		case NodeType::Addition:			return OpCode::add;
		case NodeType::Subtraction:		return OpCode::sub;
		case NodeType::Multiplication:	return OpCode::mul;
		case NodeType::Division:			return OpCode::div;
		case NodeType::Power:				return OpCode::pow;
		case NodeType::Negation:			return OpCode::neg;
		case NodeType::Percent:			return OpCode::pct;
		}
	}

	void binary_op_IR_generate(IRInstruction& left_child, IRInstruction& right_child) {
		ir_instructions.emplace_back(
			std::monostate{},
			get_opcode(ast.node_types[i]),
			IRInstructionType::f64,
			i,
			ast.child_start[i],
			2,
			0
		);
		auto& current_instr = ir_instructions.back();
		ir_instructions[operands_pool[current_instr.operands_start].value()].use_count++;
		ir_instructions[operands_pool[current_instr.operands_start + 1].value()].use_count++;
	}

	AbstractSyntaxTree_SoA ast;

	std::vector<IRInstruction> ir_instructions;
	std::vector<ExpectedIndex> operands_pool;
	int i = 0;
};