﻿#ifndef APPLICATIOND3D_H
#define APPLICATIOND3D_H
/*
* The header of the module implementing the sample D3D12 application
* for JNP3 course at MIMUW.
*
* Author:   Przemys³aw Rutka
* Version:  1.0
* Data:     13.12.2023
*/

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif*/ /* WIN32_LEAN_AND_MEAN */
#include <windows.h>

/*
 * Initializes all Direct3D 12 components to run the application.
 */
void InitDirect3D(HWND hWnd);

/*
 * Initializes the animation timer.
 */
void InitTimer(HWND hwnd);

/*
 * Computes changes in 3D scene animation state.
 */
void OnTimer();

/*
 * Destroys the animation timer.
 */
void ReleaseTimer(HWND hwnd);

/*
 * Paints the content of application window.
 */
void OnPaint();

/*
 * Finishes the usage of Direct3D 12.
 */
void EndDirect3D();

#endif /* APPLICATIOND3D_H */