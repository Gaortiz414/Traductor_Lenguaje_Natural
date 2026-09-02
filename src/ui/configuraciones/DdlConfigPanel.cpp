#include "DdlConfigPanel.h"
#include "../StringConvert.h"
#include <windows.h>

namespace ui::configuraciones {

DdlConfigPanel::DdlConfigPanel() {}

DdlConfigPanel::~DdlConfigPanel() {}

void DdlConfigPanel::Create(HWND hWndParent, const wchar_t* title, const wchar_t* subtitle) {
    ConfiguracionBasePanel::Create(hWndParent, title, subtitle);

    SendMessageW(hComboToken_, CB_ADDSTRING, 0, (LPARAM)L"TOKEN_CREATE");
    SendMessageW(hComboToken_, CB_SETCURSEL, 0, 0);

    AddListColumn(0, L"No.", 50);
    AddListColumn(1, L"Sinonimo / Alias DDL", 400);

    Refresh();
}

std::string DdlConfigPanel::GetSelectedToken() const {
    int idx = (int)SendMessageW(hComboToken_, CB_GETCURSEL, 0, 0);
    if (idx == CB_ERR) return "TOKEN_CREATE";
    wchar_t buf[64];
    SendMessageW(hComboToken_, CB_GETLBTEXT, idx, (LPARAM)buf);
    return WideToUtf8(buf);
}

void DdlConfigPanel::Refresh() {
    ClearList();
    if (!jsonLoaded_) return;

    std::string token = GetSelectedToken();
    if (!jsonRoot_.contains("configuracion_lexica") ||
        !jsonRoot_["configuracion_lexica"].contains("palabras_reservadas") ||
        !jsonRoot_["configuracion_lexica"]["palabras_reservadas"].contains("DDL") ||
        !jsonRoot_["configuracion_lexica"]["palabras_reservadas"]["DDL"].contains(token)) {
        return;
    }

    const auto& arr = jsonRoot_["configuracion_lexica"]["palabras_reservadas"]["DDL"][token];
    int row = 0;
    for (const auto& item : arr) {
        std::wstring itemStr = Utf8ToWide(item.get<std::string>());
        AddListItem(row, std::to_wstring(row + 1), itemStr);
        row++;
    }
}

void DdlConfigPanel::OnComboSelect() {
    Refresh();
}

void DdlConfigPanel::OnAdd() {
    std::wstring newItem = GetEditText(hEditNewItem_);
    if (newItem.empty()) {
        MessageBoxW(hWndParent_, L"Por favor ingrese un sinonimo DDL valido.", L"Advertencia", MB_OK | MB_ICONWARNING);
        return;
    }

    std::string token = GetSelectedToken();
    std::string newItemUtf8 = WideToUtf8(newItem);

    jsonRoot_["configuracion_lexica"]["palabras_reservadas"]["DDL"][token].push_back(newItemUtf8);
    SetEditText(hEditNewItem_, L"");
    Refresh();
}

void DdlConfigPanel::OnDelete() {
    int selIdx = ListView_GetNextItem(hListItems_, -1, LVNI_SELECTED);
    if (selIdx < 0) {
        MessageBoxW(hWndParent_, L"Seleccione un elemento de la lista para eliminar.", L"Advertencia", MB_OK | MB_ICONWARNING);
        return;
    }

    std::string token = GetSelectedToken();
    auto& arr = jsonRoot_["configuracion_lexica"]["palabras_reservadas"]["DDL"][token];
    if (selIdx < (int)arr.size()) {
        arr.erase(arr.begin() + selIdx);
    }
    Refresh();
}

void DdlConfigPanel::OnSave() {
    if (SaveSchemaJson()) {
        MessageBoxW(hWndParent_, L"Configuracion DDL guardada exitosamente en config/schema_master.json", L"Exito", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBoxW(hWndParent_, L"Error al guardar los cambios en config/schema_master.json", L"Error", MB_OK | MB_ICONERROR);
    }
}

}
