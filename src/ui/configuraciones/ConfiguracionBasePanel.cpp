#include "ConfiguracionBasePanel.h"
#include "../StringConvert.h"
#include "../../../Resource.h"
#include <fstream>
#include <iomanip>

namespace ui::configuraciones {

ConfiguracionBasePanel::ConfiguracionBasePanel() {}

ConfiguracionBasePanel::~ConfiguracionBasePanel() {}

std::string ConfiguracionBasePanel::GetSchemaPath() const {
    return "config/schema_master.json";
}

bool ConfiguracionBasePanel::LoadSchemaJson() {
    std::ifstream file(GetSchemaPath());
    if (!file.is_open()) {
        jsonLoaded_ = false;
        return false;
    }
    try {
        file >> jsonRoot_;
        jsonLoaded_ = true;
    } catch (...) {
        jsonLoaded_ = false;
        return false;
    }
    return true;
}

bool ConfiguracionBasePanel::SaveSchemaJson() {
    bool ok = false;
    std::ofstream file(GetSchemaPath());
    if (file.is_open()) {
        try {
            file << std::setw(2) << jsonRoot_ << std::endl;
            ok = true;
        } catch (...) {}
    }

    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    std::wstring exeDir(exePath);
    size_t slash = exeDir.find_last_of(L"\\/");
    if (slash != std::wstring::npos) {
        std::string candidate1 = WideToUtf8(exeDir.substr(0, slash)) + "\\config\\schema_master.json";
        std::ofstream file2(candidate1);
        if (file2.is_open()) {
            try {
                file2 << std::setw(2) << jsonRoot_ << std::endl;
                ok = true;
            } catch (...) {}
        }
    }
    return ok;
}

void ConfiguracionBasePanel::Create(HWND hWndParent, const wchar_t* title, const wchar_t* subtitle) {
    hWndParent_ = hWndParent;
    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtrW(hWndParent, GWLP_HINSTANCE);

    hLblTitle_ = CreateWindowExW(0, L"STATIC", title,
        WS_CHILD | SS_LEFT, 0, 0, 0, 0, hWndParent, (HMENU)(INT_PTR)IDC_LBL_CFG_TITLE, hInst, nullptr);

    hLblSubtitle_ = CreateWindowExW(0, L"STATIC", subtitle,
        WS_CHILD | SS_LEFT, 0, 0, 0, 0, hWndParent, (HMENU)(INT_PTR)IDC_LBL_CFG_SUBTITLE, hInst, nullptr);

    hComboToken_ = CreateWindowExW(0, L"COMBOBOX", L"",
        WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL, 0, 0, 0, 0, hWndParent, (HMENU)(INT_PTR)IDC_COMBO_CFG_TOKEN, hInst, nullptr);

    hListItems_ = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEW, L"",
        WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
        0, 0, 0, 0, hWndParent, (HMENU)(INT_PTR)IDC_LV_CFG_ITEMS, hInst, nullptr);

    ListView_SetExtendedListViewStyle(hListItems_, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    hEditNewItem_ = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWndParent, (HMENU)(INT_PTR)IDC_EDIT_CFG_NEW_ITEM, hInst, nullptr);

    hBtnAdd_ = CreateWindowExW(0, L"BUTTON", L"Agregar",
        WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hWndParent, (HMENU)(INT_PTR)IDC_BTN_CFG_ADD, hInst, nullptr);

    hBtnDel_ = CreateWindowExW(0, L"BUTTON", L"Eliminar",
        WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hWndParent, (HMENU)(INT_PTR)IDC_BTN_CFG_DEL, hInst, nullptr);

    hBtnSave_ = CreateWindowExW(0, L"BUTTON", L"Guardar Cambios",
        WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hWndParent, (HMENU)(INT_PTR)IDC_BTN_CFG_SAVE, hInst, nullptr);

    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessageW(hLblTitle_, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessageW(hLblSubtitle_, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessageW(hComboToken_, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessageW(hListItems_, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessageW(hEditNewItem_, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessageW(hBtnAdd_, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessageW(hBtnDel_, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessageW(hBtnSave_, WM_SETFONT, (WPARAM)hFont, TRUE);

    LoadSchemaJson();
}

void ConfiguracionBasePanel::Layout(RECT rc) {
    if (!isVisible_) return;

    const int margin = 16;
    const int width = (rc.right - rc.left) - (margin * 2);
    if (width <= 0) return;

    int y = margin;

    MoveWindow(hLblTitle_, margin, y, width, 22, TRUE);
    y += 24;

    MoveWindow(hLblSubtitle_, margin, y, width, 18, TRUE);
    y += 24;

    if (IsWindowVisible(hComboToken_)) {
        MoveWindow(hComboToken_, margin, y, 240, 200, TRUE);
        y += 32;
    }

    const int btnHeight = 28;
    const int editHeight = 26;
    const int bottomControlsHeight = editHeight + margin + btnHeight + margin;

    int listHeight = (rc.bottom - rc.top) - y - bottomControlsHeight;
    if (listHeight < 100) listHeight = 100;

    MoveWindow(hListItems_, margin, y, width, listHeight, TRUE);
    y += listHeight + margin;

    const int addBtnWidth = 100;
    const int editWidth = width - addBtnWidth - margin;
    MoveWindow(hEditNewItem_, margin, y, editWidth, editHeight, TRUE);
    MoveWindow(hBtnAdd_, margin + editWidth + margin, y - 1, addBtnWidth, editHeight + 2, TRUE);
    y += editHeight + margin;

    const int actionBtnWidth = 140;
    MoveWindow(hBtnDel_, margin, y, actionBtnWidth, btnHeight, TRUE);
    MoveWindow(hBtnSave_, margin + actionBtnWidth + margin, y, actionBtnWidth + 20, btnHeight, TRUE);
}

void ConfiguracionBasePanel::Show(bool show) {
    isVisible_ = show;
    int cmd = show ? SW_SHOW : SW_HIDE;
    if (hLblTitle_) ShowWindow(hLblTitle_, cmd);
    if (hLblSubtitle_) ShowWindow(hLblSubtitle_, cmd);
    if (hComboToken_) ShowWindow(hComboToken_, cmd);
    if (hListItems_) ShowWindow(hListItems_, cmd);
    if (hEditNewItem_) ShowWindow(hEditNewItem_, cmd);
    if (hBtnAdd_) ShowWindow(hBtnAdd_, cmd);
    if (hBtnDel_) ShowWindow(hBtnDel_, cmd);
    if (hBtnSave_) ShowWindow(hBtnSave_, cmd);

    if (show) {
        LoadSchemaJson();
        Refresh();
    }
}

bool ConfiguracionBasePanel::ProcessCommand(WORD id, WORD code) {
    if (!isVisible_) return false;

    if (id == IDC_COMBO_CFG_TOKEN && code == CBN_SELCHANGE) {
        OnComboSelect();
        return true;
    }
    if (id == IDC_BTN_CFG_ADD && code == BN_CLICKED) {
        OnAdd();
        return true;
    }
    if (id == IDC_BTN_CFG_DEL && code == BN_CLICKED) {
        OnDelete();
        return true;
    }
    if (id == IDC_BTN_CFG_SAVE && code == BN_CLICKED) {
        OnSave();
        return true;
    }
    return false;
}

void ConfiguracionBasePanel::ClearList() {
    ListView_DeleteAllItems(hListItems_);
}

void ConfiguracionBasePanel::AddListColumn(int colIdx, const wchar_t* text, int width) {
    LVCOLUMNW lvc = { 0 };
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    lvc.iSubItem = colIdx;
    lvc.pszText = (LPWSTR)text;
    lvc.cx = width;
    ListView_InsertColumn(hListItems_, colIdx, &lvc);
}

void ConfiguracionBasePanel::AddListItem(int index, const std::wstring& col1Text, const std::wstring& col2Text) {
    LVITEMW lvi = { 0 };
    lvi.mask = LVIF_TEXT;
    lvi.iItem = index;
    lvi.iSubItem = 0;
    lvi.pszText = (LPWSTR)col1Text.c_str();
    ListView_InsertItem(hListItems_, &lvi);

    if (!col2Text.empty()) {
        ListView_SetItemText(hListItems_, index, 1, (LPWSTR)col2Text.c_str());
    }
}

std::wstring ConfiguracionBasePanel::GetEditText(HWND hEdit) {
    int len = GetWindowTextLengthW(hEdit);
    if (len <= 0) return L"";
    std::wstring buf(len + 1, L'\0');
    GetWindowTextW(hEdit, &buf[0], len + 1);
    buf.resize(len);
    return buf;
}

void ConfiguracionBasePanel::SetEditText(HWND hEdit, const std::wstring& text) {
    SetWindowTextW(hEdit, text.c_str());
}

}
