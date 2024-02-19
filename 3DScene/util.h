#ifndef UTIL_H
#define UTIL_H

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>


void hr_check(HRESULT hr);

BYTE* LoadBitmapFromFile(PCWSTR uri, UINT& width, UINT& height);


#endif /* UTIL_H */