/*
 * @Author: qinXiong
 * @Date: 2026-05-14 20:08:30
 * @LastEditors: Qxiong&&2307975018@qq.com
 * @LastEditTime: 2026-05-19 11:50:10
 * @Description: 
 */
// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "framework.h"

// BootLoader Level 1 Security Access Key Calculation
// Key1[i] = Seed[i] ^ Mask[i]
// Key2[i] = (Seed[i] << 1) ^ Mask[i]
// Key[i]  = Key1[i] + Key2[i]  (8-bit, discard carry)
// Mask = {0xA9, 0xC6, 0x13, 0x91}
extern "C" __declspec(dllexport) void __stdcall ComputeKeyLevel1(const unsigned char* seed, unsigned char* key)
{
    const unsigned char mask[4] = { 0xA9, 0xC6, 0x13, 0x91 };
    for (int i = 0; i < 4; i++)
    {
        unsigned char seed1 = (seed[i] << 1) & 0xFF;
        unsigned char key1 = seed[i] ^ mask[i];
        unsigned char key2 = seed1 ^ mask[i];
        key[i] = (key1 + key2) & 0xFF;
    }
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

