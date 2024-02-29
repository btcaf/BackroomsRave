#ifndef UTIL_H
#define UTIL_H

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>

/*
 * Helper function check HRESULT. Exits with code 1 if failed.
 */
void hr_check(HRESULT hr);


/*
 * Helper function to load a bitmap from a file.
 */
BYTE* load_bitmap(PCWSTR uri, UINT& width, UINT& height);


#endif /* UTIL_H */