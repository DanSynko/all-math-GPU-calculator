#ifdef __INTELLISENSE__
#include <string>
#include <format>
#else
import std;
#endif

export module cuda_backend:kernel_generator;

export struct CudaGenerator {
	std::string generate(const std::string& coded_instructions) {
		std::string kernel_code = std::format(R"(
				extern "C" __global__ void custom_kernel(double* d_out) {{
					*d_out = {};
				}}
			)", coded_instructions);

		return kernel_code;
	}
};