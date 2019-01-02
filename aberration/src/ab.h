#pragma once
#include <stdio.h>
#ifdef AB_BUILD_DLL
#define AB_API __declspec(dllexport)
#else
#define AB_API __declspec(dllimport)
#endif

AB_API void ab();