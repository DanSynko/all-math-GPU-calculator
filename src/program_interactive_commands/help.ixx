export module help;

#ifdef __INTELLISENSE__
#include <print>
#include <array>
#else
import std;
#endif

import console_print;

export void print_help() {
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

	std::println("\033[36m FORMAT: [Settings name]    -     [value]     -    [flags] \033[0m");
	Console::print_divider("-");
	std::println();
	constexpr std::array settings_list = {
		"Enable Fast Math mode       -       Apply algebraic simplifications using unsafe math optimizations (disregarding IEEE 754)     -     -f, --fast-math."
	};
	for (const auto& i : settings_list) {
		std::println("{}", i);
	}
	std::println();



	Console::print_header(" [ 4. 📓 NOTICES. ] ");
	std::print("\033[36m");
	constexpr std::array notices = {
		"1) You can print negative numbers without parentheses even not at the beginning.",
		"2) You need to use parentheses if the exponent contains more than just a single constant or variable (for example, an operator).",
		"3) Unicode does not support nested exponents like LaTeX; it only supports one level(e. g. 2², but not 2²²).",
		"4) Programm does not support output LaTeX image in now.",
		"5) A 'Complex numbers are not supported at the moment' error may occur with the following mathematical expressions:\n",
		"	1. Square root of a negative number;",
		"	2. Negative base raised to a fractional exponent (e.g., -2 ^ 0.5 )."
	};
	for (const auto& i : notices) {
		std::println("{}", i);
	}
	std::println("\033[0m");



	Console::print_to_center("Enjoy using the program!", 0);
	Console::print_divider("━");
	std::println();
}