module;
#include "nvrtc.h"

#ifdef __INTELLISENSE__
#include <string>
#include <print>
#else
import std;
#endif

export module cuda_backend:nvrtc_compiler;

export struct CudaCompiler {
	std::string compile(const std::string& kernel_code) {
		nvrtcProgram prog;

		nvrtcCreateProgram(&prog,
			kernel_code.c_str(),
			"custom_kernel.cu",
			0, nullptr, nullptr);

		const char* opts[] = {
			"--gpu-architecture=compute_75"
		};

		nvrtcResult compile_result = nvrtcCompileProgram(prog, 1, opts);

		if (compile_result != NVRTC_SUCCESS) {
			size_t log_size;
			nvrtcGetProgramLogSize(prog, &log_size);

			std::string compile_log(log_size, '\0');

			nvrtcGetProgramLog(prog, &compile_log[0]);

			std::println("NVRTC Compilation failed: {}", compile_log);
		}

		size_t ptx_size;
		nvrtcGetPTXSize(prog, &ptx_size);

		std::string ptx_code(ptx_size, '\0');

		nvrtcGetPTX(prog, &ptx_code[0]);

		nvrtcDestroyProgram(&prog);

		std::println("{}", ptx_code.c_str());

		return ptx_code;
	}
};