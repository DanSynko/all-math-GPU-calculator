module;
#include "third-party/CLI11.hpp";

export module settings;

#ifdef __INTELLISENSE__
#include <expected>
#else
import std;
#endif

import types;
import help;

export class Settings {
public:
	[[nodiscard]] bitmask load_from_config() {
		std::ifstream in("config.txt");

		if (!in.is_open()) {
			return default_settings;
		}

		std::string line;
		while (std::getline(in, line)) {
			if (line == "fast_math=true") {
				loaded_settings |= Bitmask::fast_math_mode;
			}
		}

		return loaded_settings;
	}

	[[nodiscard]] std::expected<bitmask, int> load_from_arguments(int main_argc, char* main_argv[]) {
		CLI::App app(std::string{}, "all_math_GPU_calculator");
		app.set_help_flag(std::string{}, std::string{});

		bool enable_fast_math = false;
		bool needs_print_help = false;

		app.add_flag("-f,--fast-math", enable_fast_math, "Apply algebraic simplifications using unsafe math optimizations (disregarding IEEE 754)");
		app.add_flag("-h,--help", needs_print_help, "Get information about the program.");

		try {
			app.parse(main_argc, main_argv);
		}
		catch (const CLI::ParseError& e) {
			return std::unexpected(app.exit(e));
		}

		if (needs_print_help) {
			print_help();
			std::exit(0);
		}

		if (enable_fast_math) {
			loaded_settings |= Bitmask::fast_math_mode;
		}

		return loaded_settings;
	}
private:
	bitmask loaded_settings = 0;
	const bitmask default_settings = 0;
};