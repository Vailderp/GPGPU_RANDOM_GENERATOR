#pragma once
#include "pch.h"
#define _SILENCE_AMP_DEPRECATION_WARNINGS
#include <amp.h>
#include <iostream>
#include <fstream>
#include <iostream>
#include <type_traits>

#ifdef GPGPURAND_EXPORTS
	#define GPGPURAND_API __declspec(dllexport)
#else
	#define GPGPURAND_API __declspec(dllimport)
#endif //GPGPURAND_EXPORTS

using namespace concurrency;

inline int __fastcall rand(const int min, const int max, unsigned int seed) restrict(amp, cpu)
{
    seed = 8253729 * seed + 2396403;
    int rand = (8253729 * seed + 2396403) % (max + min + 1) - min;
    while (rand < min || rand > max)
    {
        rand = (8253729 * seed + 2396403) % (max + min) - min;
    }
    return rand;
}

typedef int rank_t;
template<typename _Value_ptr_type> using Ptr = _Value_ptr_type*;
template<typename _Value_ptr_type, rank_t _Rank>

struct PtrInfStr
{
    typedef typename  PtrInfStr<Ptr<_Value_ptr_type>, _Rank - 1>::value value;
};

template< typename _Value_ptr_type>
struct PtrInfStr<_Value_ptr_type, NULL>
{
    typedef _Value_ptr_type value;
};

template<typename _Value_ptr_type, rank_t _Rank>
using PtrInf = typename PtrInfStr<_Value_ptr_type, _Rank>::value;

template <typename _Type>
static inline Ptr<std::allocator<_Type>> allocator = nullptr;

template <typename _Type>
void FillCPU(PtrInf<Ptr<void>, 1> data, const size_t length, const int min, const int max)
{
    if (allocator<_Type> == nullptr)
    {
        allocator<_Type> = new std::allocator<_Type>;
    }

    *data = reinterpret_cast<PtrInf<void, 1>>(allocator<_Type>->allocate(length));
    for (size_t i = 0; i < length; i++)
    {
        static_cast<Ptr<_Type>>(*data)[i] = rand(min, max, i * 132);
    }
}

typedef void (*FillArr)(PtrInf<Ptr<void>, 1> data, size_t, int, int);

inline FillArr FillRandomArrayCPU;

constexpr void TypeOfByteCountCPU(const BYTE byte_count)
{
    if (byte_count == 1)
    {
        FillRandomArrayCPU = &FillCPU<char>;
        return;
    }
    if (byte_count == 2)
    {
        FillRandomArrayCPU = &FillCPU<short>;
        return;
    }
    if (byte_count == 4)
    {
        FillRandomArrayCPU = &FillCPU<int>;
        return;
    }
    if (byte_count == 8)
    {
        FillRandomArrayCPU = &FillCPU<long long>;
        return;
    }
}



template <typename _Type>
void FillGPU(PtrInf<Ptr<void>, 1> data, const size_t length, const int min, const int max, bool async = false)
{
    if (allocator<_Type> == nullptr)
    {
        allocator<_Type> = new std::allocator<_Type>;
    }
    *data = reinterpret_cast<PtrInf<void, 1>>(allocator<_Type>->allocate(length));
    array_view<int, 1> video_data(length, reinterpret_cast<int*>(*data));
    video_data.discard_data();
    array<unsigned int, 1> rand_out_data(1);
    parallel_for_each(
        video_data.extent,
        [=, &rand_out_data](index<1> idx) restrict(amp)
        {
            video_data[idx] = rand(min, max, idx[1]);
            rand_out_data[0]+= 324;
        }
    );
    if (!async)
    {
        video_data.synchronize();
    }
    *data = reinterpret_cast<PtrInf<void, 1>>(video_data.data());
}

typedef void (*FillArrGPU)(PtrInf<Ptr<void>, 1> data, size_t, int, int, bool);

inline FillArrGPU FillRandomArrayGPU;

constexpr void TypeOfByteCountGPU(const BYTE byte_count)
{
    if (byte_count == 1)
    {
        FillRandomArrayGPU = &FillGPU<char>;
        return;
    }
    if (byte_count == 2)
    {
        FillRandomArrayGPU = &FillGPU<short>;
        return;
    }
    if (byte_count == 4)
    {
        FillRandomArrayGPU = &FillGPU<int>;
        return;
    }
    if (byte_count == 8)
    {
        FillRandomArrayGPU = &FillGPU<long long>;
        return;
    }
}


extern "C" GPGPURAND_API void __cdecl FillArrayRandCPU
(
    PtrInf<Ptr<void>, 1> data, 
    const BYTE byte_count, 
    const size_t length, 
    const int min, 
    const int max
);

extern "C" GPGPURAND_API void __cdecl FillArrayRandGPU
(
    PtrInf<Ptr<void>, 1> data, 
    const BYTE byte_count, 
    const size_t length, 
    const int min, 
    const int max, 
    const bool async = false
);