#include "pch.h"
#include "gpu.h"


void __cdecl FillArrayRandCPU(PtrInf<Ptr<void>, 1> data, const BYTE byte_count, const size_t length, const int min, const int max)
{
	TypeOfByteCountCPU(byte_count);
	FillRandomArrayCPU(data, length, min, max);
}

void __cdecl FillArrayRandGPU(PtrInf<Ptr<void>, 1> data, const BYTE byte_count, const size_t length, const int min, const int max, const bool async)
{
	TypeOfByteCountGPU(byte_count);
	FillRandomArrayGPU(data, length, min, max, async);
}
