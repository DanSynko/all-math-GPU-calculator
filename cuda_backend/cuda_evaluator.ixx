#ifdef __INTELLISENSE__
#include <string>
#else
import std;
#endif

export module cuda_backend;

import :kernel_generator;
import :nvrtc_compiler;
import :driver_executor;

export struct CudaEvaluator {
	double evaluate(const std::string& instructions) {
		CudaGenerator generator;
		std::string kernel_code = generator.generate(instructions);

		CudaCompiler compiler;
		std::string ptx = compiler.compile(kernel_code);

		CudaExecutor executor;
		return executor.execute(ptx);
	}
};