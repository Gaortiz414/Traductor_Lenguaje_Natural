#pragma once

#include "../../framework.h"
#include <string>

namespace ui::errorpanel {

void CreateErrorIcon(HWND hWnd);

void CreatePanel(HWND hWndOwner);

void LayoutErrorIcon(HWND hWnd);

void MoveErrorButton(int x, int y, int width, int height);

void SetErrorIconVisible(bool visible);

void TogglePanel(HWND hWnd);

void ClearErrors();

void AddError(size_t line, const std::string& message);

}
