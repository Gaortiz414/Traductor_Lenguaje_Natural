#include "DmlConfigPanel.h"
#include "../StringConvert.h"
#include <windows.h>

namespace ui::configuraciones {

DmlConfigPanel::DmlConfigPanel() {}

DmlConfigPanel::~DmlConfigPanel() {}

void DmlConfigPanel::Create(HWND hWndParent, const wchar_t* title, const wchar_t* subtitle) {
    ConfiguracionBasePanel::Create(hWndParent, title, subtitle);

    SendMessageW(hComboToken_, CB_ADDSTRING, 0, (LPARAM)L"TOKEN_SELECT");
    SendMessageW(hComboToken_, CB_ADDSTRING, 0, (LPARAM)L"TOKEN_FROM");
    SendMessageW(hComboToken_, CB_ADDSTRING, 0, (LPARAM)L"TOKEN_WHERE");
    SendMessageW(hComboToken_, CB_ADDSTRING, 0, (LPARAM)L"TOKEN_AND");
    SendMessageW(hComboToken_, CB_ADDSTRING, 0, (LPARAM)L"TOKEN_OR");
    SendMessageW(hComboToken_, CB_SETCURSEL, 0, 0);

    AddListColumn(0, L"No.", 50);
    AddListColumn(1, L"Sinonimo / Alias (Lexema NL)", 400);

    Refresh();
}

std::string DmlConfigPanel::GetSelectedToken() const {
    int idx = (int)SendMessageW(hComboToken_, CB_GETCURSEL, 0, 0);
    if (idx == CB_ERR) return "TOKEN_SELECT";
    wchar_t buf[64];
    SendMessageW(hComboToken_, CB_GETLBTEXT, idx, (LPARAM)buf);
    return WideToUtf8(buf);
}

void DmlConfigPanel::Refresh() {
    ClearList();
    if (!jsonLoaded_) return;

    std::string token = GetSelectedToken();
    if (!jsonRoot_.contains("configuracion_lexica") ||
        !jsonRoot_["configuracion_lexica"].contains("palabras_reservadas") ||
        !jsonRoot_["configuracion_lexica"]["palabras_reservadas"].contains("DML") ||
        !jsonRoot_["configuracion_lexica"]["palabras_reservadas"]["DML"].contains(token)) {
        return;
    }

    const auto& arr = jsonRoot_["configuracion_lexica"]["palabras_reservadas"]["DML"][token];
    int row = 0;
    for (const auto& item : arr) {
        std::wstring itemStr = Utf8ToWide(item.get<std::string>());
        AddListItem(row, std::to_wstring(row + 1), itemStr);
        row++;
    }
}

void DmlConfigPanel::OnComboSelect() {
    Refresh();
}

void DmlConfigPanel::OnAdd() {
    std::wstring newItem = GetEditText(hEditNewItem_);
    if (newItem.empty()) {
        MessageBoxW(hWndParent_, L"Por favor ingrese un sinonimo valido.", L"Advertencia", MB_OK | MB_ICONWARNING);
        return;
    }

    std::string token = GetSelectedToken();
    std::string newItemUtf8 = WideToUtf8(newItem);

    jsonRoot_["configuracion_lexica"]["palabras_reservadas"]["DML"][token].push_back(newItemUtf8);
    SetEditText(hEditNewItem_, L"");
    Refresh();
}

void DmlConfigPanel::OnDelete() {
    int selIdx = ListView_GetNextItem(hListItems_, -1, LVNI_SELECTED);
    if (selIdx < 0) {
        MessageBoxW(hWndParent_, L"Seleccione un elemento de la lista para eliminar.", L"Advertencia", MB_OK | MB_ICONWARNING);
        return;
    }

    std::string token = GetSelectedToken();
    auto& arr = jsonRoot_["configuracion_lexica"]["palabras_reservadas"]["DML"][token];
    if (selIdx < (int)arr.size()) {
        arr.erase(arr.begin() + selIdx);
    }
    Refresh();
}

void DmlConfigPanel::OnSave() {
    if (SaveSchemaJson()) {
        MessageBoxW(hWndParent_, L"Configuracion DML guardada exitosamente en config/schema_master.json", L"Exito", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBoxW(hWndParent_, L"Error al guardar los cambios en config/schema_master.json", L"Error", MB_OK | MB_ICONERROR);
    }
}

}
