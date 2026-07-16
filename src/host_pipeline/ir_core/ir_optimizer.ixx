export module ir_optimizer;

#ifdef __INTELLISENSE__
#include <cstdint>
#include <vector>
#include <variant>
#include <print>
#else
import std.compat;
#endif

import types;
import lookup_table;
import error_handler;

template<typename T>
concept ConstantType = std::same_as<T, int32_t> ||
					   std::same_as<T, float>   ||
					   std::same_as<T, double>;
export class IROptimizer {
public:
	IROptimizer(const std::vector<IRInstruction>& ir_instructions, const std::vector<ExpectedIndex>& instructions_operands_pool, bitmask settings)
		: instructions(ir_instructions)
		, operands_pool(instructions_operands_pool)
		, current_settings(settings)
	{

	}

	using OptimizerResult = std::expected<std::vector<IRInstruction>, ErrorType>;
	[[nodiscard]] OptimizerResult optimize() {
		for (auto& i : instructions) {
			if (ascii_symbols[get_opcode_symbol(i.opcode)] & MASK_OPERATOR) {
				switch (i.opcode) {
				case OpCode::add:
				case OpCode::sub:
				case OpCode::mul:
				case OpCode::div:
				case OpCode::pow: {
					auto correct_result = std::visit([this, &i](const auto& l, const auto& r) -> std::expected<PayloadType, ErrorType> {
						using LeftT = std::decay_t<decltype(l)>;
						using RightT = std::decay_t<decltype(r)>;

						if constexpr (ConstantType<LeftT> && std::same_as<LeftT, RightT>) {
							return fold_constants<LeftT>(i.opcode, l, r);
						}
						else {
							std::println("{}", sizeof(LeftT));
							std::println("{}", sizeof(RightT));
							return l;
						}
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
					auto correct_result = std::visit([this, &i](const auto& r) -> std::expected<PayloadType, ErrorType> {
						using RightT = std::decay_t<decltype(r)>;

						if constexpr (ConstantType<RightT>) {
							return this->fold_constants<RightT>(i.opcode, r);
						}
						else {
							std::println("{}", sizeof(RightT));
							return r;
						}
						}, instructions[operands_pool[i.operands_start].value()].payload);

					if (!correct_result.has_value()) {
						error_handler.register_semantic(correct_result.error());
					}

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
	template<ConstantType T>
	[[nodiscard]] std::expected<PayloadType, ErrorType> fold_constants_pow(T base, T exponent) const noexcept {
		using base_T = std::decay_t<decltype(base)>;
		using exponent_T = std::decay_t<decltype(exponent)>;

		// Mathematically, a complex result occurs only when a fractional exponent has an even denominator. 
		// However, checking for an even denominator is impossible on a PC due to the binary nature of IEEE 754.
		// Every float/double fraction stored in memory always has a denominator that is a power of 2 (which is always even).
		// For example, 1.0 / 3.0 becomes 0.33333333333333331482961625624739, or 6004799503160661 / 18014398509481984.
		// The denominator is 2^55, which is even. 
		// Conclusion: any fractional exponent of a negative base is processed as an even-root extraction, leading to NaN.
		// Therefore, we simply check for the presence of any fractional part in the exponent (exponent_has_frac).
		bool exponent_has_frac = exponent != std::trunc(exponent);
		if (base < 0 && exponent_has_frac) {
			return std::unexpected(ErrorType::UnsupportedComplex);
		}
		return std::pow(base, exponent);
	}


	template<ConstantType T>
	[[nodiscard]] std::expected<PayloadType, ErrorType> fold_constants(OpCode op, T left_child, T right_child) const noexcept {
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
			if (right_child == 0.0) return std::unexpected(ErrorType::DivByZero);
			return left_child / right_child;
		case OpCode::pow:
			return fold_constants_pow(left_child, right_child);
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

	static constexpr std::array ascii_symbols = lookup_table_fill();

	bitmask current_settings;

	std::vector<IRInstruction> instructions;
	std::vector<ExpectedIndex> operands_pool;

	ErrorHandler error_handler;
};