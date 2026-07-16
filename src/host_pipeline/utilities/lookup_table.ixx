export module lookup_table;

import types;

export {
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
}