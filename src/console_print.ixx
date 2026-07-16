module;
#ifdef  _WIN32
#include <Windows.h>
#endif

export module console_print;

#ifdef __INTELLISENSE__
#include <print>
#include <string>
#else
import std;
#endif


export namespace Console {
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