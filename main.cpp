#include <iostream>
#include <array>
#include <stack>
#include <queue>
#include <string>
#include <string_view>
#include <concepts>
#include <algorithm>

#ifdef _WIN32
#include <Windows.h>
#endif

void safe_input(std::string& expr) {
    bool correct_expr;
    do {
        std::getline(std::cin, expr);

        if (expr == "help" || expr == "exit") {
            return;
        }

        // this list in string_view will change.

        // TODO:
            // General Math: ⋅, ⁰ ¹ ² ³ ⁴ ⁵ ⁶ ⁷ ⁸ ⁹ ˣ ʸ ⁿ, √(and ³√, ⁴√, ⁵√), variables(𝑥, 𝑦), =, ≈, ƒ, log(and log₂, log₁₀), | (modules).
            // Trigonomethry: °, π, sin, cos, tan.
            // Calculus: lim, →, ℯ, ln, 𝒅, ′, ″, ∫, ∫∫, ∫∫∫, ∮, ₀ ₁ ₂ ₃ ₄ ₅ ₆ ₇ ₈ ₉ ₐ ₑ ₒ ₓ ₙ, Σ, ∏, ꝏ, ∞, ⁺, ₊, ⁻, ₋.

        correct_expr = true;
        if (!std::all_of(expr.begin(), expr.end(), [](char c) {
            return std::string_view("0123456789+-*/.,()%^").find(c) != std::string_view::npos;
            })) {
            correct_expr = false;
            std::cout << "Error: invalid math expression. Try again." << std::endl;
        }

    } while (!correct_expr);
}

template<typename T>
concept mathexpr = std::same_as<T, std::string> || std::same_as<T, std::vector<std::string>>;
bool math_expression_validation(const mathexpr auto& expr) {
    int operators_count = 0;
    int operands_count = 0;
    int open_parentheses_count = 0;
    int close_parentheses_count = 0;
    for (auto it = expr.begin(); it != expr.end(); it++) {
        if (std::string_view("+-*/^").find(*it) != std::string_view::npos) {
            if constexpr (std::is_same_v<decltype(expr), const std::string&>) {
                if (*it == '-' && (it == expr.begin() || *(std::prev(it)) == '(')) {
                    continue;
                }
            }
            operators_count++;
        }
        else if (std::string_view("()").find(*it) != std::string_view::npos) {
            if (std::string_view("(").find(*it) != std::string_view::npos) {
                open_parentheses_count++;
            }
            else if (std::string_view(")").find(*it) != std::string_view::npos) {
                close_parentheses_count++;
            }
        }
        else if (std::string_view("%").find(*it) != std::string_view::npos) {
            if constexpr (std::is_same_v<decltype(expr), const std::string&>) {
                if (it == expr.begin()) {
                    return false;
                }

                auto prev = it;
                prev--;
                if (std::string_view("+-*/ ").find(*prev) != std::string_view::npos) {
                    return false;
                }

                auto next = it;
                next++;
                if (next != expr.end()) {
                    if (std::string_view("+-*/) ").find(*next) == std::string_view::npos) {
                        return false;
                    }
                }
            }
        }
        else {
            operands_count++;
        }
    }
    if (operands_count == 0) {
        return false;
    }
    else if (operands_count <= operators_count) {
        return false;
    }
    else if (open_parentheses_count != close_parentheses_count) {
        return false;
    }

    return true;
}

std::vector<std::string> shunting_yard_algorithm(std::string_view expr) {
    std::vector<std::string> output_ast;
    std::stack<char> operators_stack;

    for (auto it = expr.begin(); it != expr.end(); it++) {
        if (*it == ' ') continue;

        if (std::string_view("+-*/()%^").find(*it) != std::string_view::npos) {
            if (*it == ')') {
                while (!operators_stack.empty() && (operators_stack.top() != '(')) {
                    output_ast.push_back(std::string(1, operators_stack.top()));
                    operators_stack.pop();
                }
                if (!operators_stack.empty()) {
                    operators_stack.pop();
                    continue;
                }
                else {
                    return output_ast;
                }
            }
            else if (*it == '/' || *it == '*') {
                while (!operators_stack.empty() && (operators_stack.top() == '*' || operators_stack.top() == '/')) {
                    output_ast.push_back(std::string(1, operators_stack.top()));
                    operators_stack.pop();
                }
            }
            else if (*it == '+') {
                while (!operators_stack.empty() && operators_stack.top() != '(') {
                    output_ast.push_back(std::string(1, operators_stack.top()));
                    operators_stack.pop();
                }
            }
            else if (*it == '-') {
                if (it == expr.begin() || *(std::prev(it)) == '(') {
                    operators_stack.push('~');
                    continue;
                }
                else {
                    while (!operators_stack.empty() && operators_stack.top() != '(') {
                        output_ast.push_back(std::string(1, operators_stack.top()));
                        operators_stack.pop();
                    }
                }
            }
            else if (*it == '%') {
                if (output_ast.empty()) {
                    return {};
                }

                output_ast.push_back("%");
                continue;
            }
            operators_stack.push(*it);
            continue;
        }
        else {
            std::string number;
            bool has_point = false;
            while (it != expr.end() && (isdigit(*it) || *it == ' ' || *it == '.' || *it == ',')) {
                if (*it != ' ') {
                    if ((*it == '.' || *it == ',')) {
                        if (!has_point) {
                            number.push_back('.');
                            has_point = true;
                        }
                    }
                    else {
                        number.push_back(*it);
                    }
                }
                it++;
            }
            output_ast.push_back(number);
            it--;
        }
    }

    while (!operators_stack.empty()) {
        output_ast.push_back(std::string(1, operators_stack.top()));
        operators_stack.pop();
    }

    return output_ast;
}



/*double tree_walking_interpretator(std::vector<std::string>& ast) {
        
}*/



int main()
{
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    #endif

    std::cout << "Welcome to all-math-gpu-calculator! This project was created for performing calculations from basic arithmetic to advanced mathematics." << std::endl;
    std::cout << "Enter: \n 1) 'exit' to close the program. \n 2) 'help' to show the expression printing guide and supported operators and symbols. \n" << std::endl;
    std::cout << "The entered mathematical expression will be displayed in a clean mathematical format.\n" << std::endl;

    while (true) {
        std::cout << "Enter any mathematical expression or command:" << std::endl;

        std::string m_expr;
        safe_input(m_expr);

        if (m_expr == "help") {
            std::cout << std::endl;
            std::cout << "--------------------------------------------------------------------------------------------------------" << std::endl;
            std::cout << "1. SUPPORTED OPERATORS AND SYMBOLS: \n" << std::endl;

            std::cout << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -" << std::endl;
            std::cout << "FORMAT: [math object]                -             [name] " << std::endl;
            std::cout << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n" << std::endl;

            std::array<std::string_view, 3> supported_symbols = {
                "+, -, ⋅, /         -         basic arithmetic operators\n",
                "𝑥ʸ                 -         exponentiation\n",
                "%                  -         percent\n"
            };
            for (const auto& i : supported_symbols) {
                std::cout << i << std::endl;
            }



            std::cout << "2. MATH EXPRESSIONS PRINTING GUIDE. \n" << std::endl;
            std::cout
                << "If you can't print Unicode math symbols (like ⋅)"
                << " - you can enter them using the simple methods listed below, and they will be converted to Unicode.\n"
                << "Here is the list of how to print symbols. \n" << std::endl;

            std::cout << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -" << std::endl;
            std::cout << "FORMAT: [Unicode symbol]        -        [simple method of printing] " << std::endl;
            std::cout << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n" << std::endl;

            std::array<std::string_view, 2> guide = {
                "⋅            -          *",
                "𝑥ʸ           -          x^(y) ",
            };
            for (const auto& i : guide) {
                std::cout << i << std::endl;
            }

            std::cout << std::endl;

            std::cout << "-------------------------------------------------------------------------------------------------------- \n \n" << std::endl;
        }
        else if (m_expr == "exit") {
            break;
        }
    }

    return 0;
}