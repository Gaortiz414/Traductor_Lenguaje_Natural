#include "../../framework.h"
#include "../../Resource.h"
#include "ErrorPanel.h"
#include "MainWindow.h"
#include "StringConvert.h"
#include <commctrl.h>
#include <vector>
#include <utility>

namespace ui::errorpanel {

namespace {

HWND hBtnErrores = nullptr;
HWND hErrorWindow = nullptr;
HWND hListErrores = nullptr;
bool g_panelErroresVisible = false;
bool s_classRegistered = false;

const wchar_t* kErrorWndClass = L"ErrorPanelWindowClass";

std::vector<std::pair<size_t, std::string>> g_errores;

void PoblarListView() {
    if (!hListErrores) return;
    ListView_DeleteAllItems(hListErrores);
    for (size_t i = 0; i < g_errores.size(); i++) {
        const auto& [line, message] = g_errores[i];

        std::wstring wLine = std::to_wstring(line);
        std::wstring wMsg = Utf8ToWide(message);

        LVITEMW item{};
        item.mask = LVIF_TEXT;
        item.iItem = (int)i;
        item.iSubItem = 0;
        item.pszText = const_cast<LPWSTR>(wLine.c_str());
        int idx = ListView_InsertItem(hListErrores, &item);

        ListView_SetItemText(hListErrores, idx, 1, const_cast<LPWSTR>(wMsg.c_str()));
    }
}

LRESULT CALLBACK ErrorPanelWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_SIZE: {
        if (hListErrores) {
            RECT rc;
            GetClientRect(hWnd, &rc);
            const int margin = 8;
            int width = (rc.right - rc.left) - (margin * 2);
            int height = (rc.bottom - rc.top) - (margin * 2);
            if (width < 0) width = 0;
            if (height < 0) height = 0;
            MoveWindow(hListErrores, margin, margin, width, height, TRUE);

            if (width > 0) {
                int wLinea = (int)(width * 0.12);
                int wError = width - wLinea - 4;
                if (wError < 100) wError = 100;
                ListView_SetColumnWidth(hListErrores, 0, wLinea);
                ListView_SetColumnWidth(hListErrores, 1, wError);
            }
        }
        return 0;
    }
    case WM_CLOSE:
        ShowWindow(hWnd, SW_HIDE);
        g_panelErroresVisible = false;
        return 0;
    default:
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
}

}

void CreateErrorIcon(HWND hWnd) {
    hBtnErrores = CreateWindowExW(0, L"BUTTON", L"Errores",
        WS_CHILD | BS_PUSHBUTTON,
        0, 0, 90, 26, hWnd, (HMENU)(INT_PTR)IDC_BTN_ERRORES, hInst, nullptr);

    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessageW(hBtnErrores, WM_SETFONT, (WPARAM)hFont, TRUE);

    LayoutErrorIcon(hWnd);
    ShowWindow(hBtnErrores, SW_HIDE);
}

void SetErrorIconVisible(bool visible) {
    if (hBtnErrores) {
        ShowWindow(hBtnErrores, visible ? SW_SHOW : SW_HIDE);
    }
}

void CreatePanel(HWND hWndOwner) {
    if (!s_classRegistered) {
        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.lpfnWndProc = ErrorPanelWndProc;
        wc.hInstance = hInst;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.lpszClassName = kErrorWndClass;
        RegisterClassExW(&wc);
        s_classRegistered = true;
    }

    const int wndWidth = 700;
    const int wndHeight = 400;
    int posX = CW_USEDEFAULT;
    int posY = CW_USEDEFAULT;
    if (hWndOwner && IsWindow(hWndOwner)) {
        RECT ownerRc{};
        GetWindowRect(hWndOwner, &ownerRc);
        int ownerCenterX = (ownerRc.left + ownerRc.right) / 2;
        int ownerCenterY = (ownerRc.top + ownerRc.bottom) / 2;
        posX = ownerCenterX - wndWidth / 2;
        posY = ownerCenterY - wndHeight / 2;

        HMONITOR hMon = MonitorFromWindow(hWndOwner, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi{ sizeof(MONITORINFO) };
        if (GetMonitorInfoW(hMon, &mi)) {
            if (posX < mi.rcWork.left) posX = mi.rcWork.left;
            if (posY < mi.rcWork.top) posY = mi.rcWork.top;
            if (posX + wndWidth > mi.rcWork.right) posX = mi.rcWork.right - wndWidth;
            if (posY + wndHeight > mi.rcWork.bottom) posY = mi.rcWork.bottom - wndHeight;
        }
    }

    hErrorWindow = CreateWindowExW(WS_EX_DLGMODALFRAME, kErrorWndClass, L"Tabla de Errores",
        WS_POPUPWINDOW | WS_CAPTION,
        posX, posY, wndWidth, wndHeight,
        hWndOwner, nullptr, hInst, nullptr);

    if (!hErrorWindow) {
        OutputDebugStringW(L"[ErrorPanel] CreateWindowExW fallo al crear hErrorWindow (Tabla de Errores).\n");
        return;
    }

    hListErrores = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS,
        0, 0, 0, 0, hErrorWindow, (HMENU)(INT_PTR)IDC_LV_ERRORES, hInst, nullptr);

    if (!hListErrores) {
        OutputDebugStringW(L"[ErrorPanel] CreateWindowExW fallo al crear hListErrores (SysListView32).\n");
        return;
    }

    ListView_SetExtendedListViewStyle(hListErrores, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    LVCOLUMNW col{};
    col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    wchar_t colLinea[] = L"Linea";
    col.pszText = colLinea; col.cx = 80; col.iSubItem = 0;
    ListView_InsertColumn(hListErrores, 0, &col);

    wchar_t colError[] = L"Error";
    col.pszText = colError; col.cx = 500; col.iSubItem = 1;
    ListView_InsertColumn(hListErrores, 1, &col);

    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessageW(hListErrores, WM_SETFONT, (WPARAM)hFont, TRUE);

    RECT rcClient;
    GetClientRect(hErrorWindow, &rcClient);
    SendMessageW(hErrorWindow, WM_SIZE, SIZE_RESTORED,
        MAKELPARAM(rcClient.right - rcClient.left, rcClient.bottom - rcClient.top));
}

void LayoutErrorIcon(HWND hWnd) {
    UNREFERENCED_PARAMETER(hWnd);
}

void MoveErrorButton(int x, int y, int width, int height) {
    if (hBtnErrores) {
        MoveWindow(hBtnErrores, x, y, width, height, TRUE);
    }
}

void TogglePanel(HWND hWnd) {
    if (!hErrorWindow) return;

    g_panelErroresVisible = !g_panelErroresVisible;

    if (g_panelErroresVisible) {
        PoblarListView();
        ShowWindow(hErrorWindow, SW_SHOW);
        SetWindowPos(hErrorWindow, HWND_TOP, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
        BringWindowToTop(hErrorWindow);
        SetForegroundWindow(hErrorWindow);
    } else {
        ShowWindow(hErrorWindow, SW_HIDE);
    }
}

void ClearErrors() {
    g_errores.clear();
    if (hListErrores) ListView_DeleteAllItems(hListErrores);
}

void AddError(size_t line, const std::string& message) {
    g_errores.emplace_back(line, message);
    if (hListErrores && g_panelErroresVisible) {
        PoblarListView();
    }
}

}
