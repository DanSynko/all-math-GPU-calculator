module;
#include "cuda.h"

#ifdef __INTELLISENSE__
#include <string>
#include <print>
#else
import std;
#endif

export module cuda_backend:driver_executor;

export struct CudaExecutor {
	double execute(const std::string& ptx) {
			cuInit(0);

			CUdevice cuDevice;
			cuDeviceGet(&cuDevice, 0);

			CUcontext cuContext;
			cuCtxCreate(&cuContext, 0, 0, cuDevice);

			CUmodule cuModule;
			cuModuleLoadData(&cuModule, ptx.c_str());

			CUfunction cuFunction;
			cuModuleGetFunction(&cuFunction, cuModule, "custom_kernel");

			CUdeviceptr d_out;

			cuMemAlloc(&d_out, sizeof(double));

			void* args[] = { &d_out };

			cuLaunchKernel(cuFunction,
				1, 1, 1,
				1, 1, 1,
				0,
				nullptr,
				args,
				nullptr);

			cuCtxSynchronize();

			double h_result = 0.0;

			cuMemcpyDtoH(&h_result, d_out, sizeof(double));

			return h_result;

			cuMemFree(d_out);
			cuModuleUnload(cuModule);
			cuCtxDestroy(cuContext);
		}
};