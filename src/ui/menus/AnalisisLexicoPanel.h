#pragma once

#include <windows.h>

namespace ui::analisislexico {

void CreatePanelNatural(HWND hWnd);

void LayoutPanelNatural(HWND hWnd);

void TogglePanelNatural(HWND hWnd);

void HidePanelNatural(HWND hWnd);

void EjecutarAnalisisLexico(HWND hWnd);

void OnModoToggleChanged(HWND hWnd);

}
