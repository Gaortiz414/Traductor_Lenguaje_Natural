#include "OperadoresNlConfigPanel.h"
#include "../StringConvert.h"
#include <windows.h>

namespace ui::configuraciones {

OperadoresNlConfigPanel::OperadoresNlConfigPanel() {}

OperadoresNlConfigPanel::~OperadoresNlConfigPanel() {}

void OperadoresNlConfigPanel::Create(HWND hWndParent, const wchar_t* title, const wchar_t* subtitle) {
    ConfiguracionBasePanel::Create(hWndParent, title, subtitle);

    SendMessageW(hComboToken_, CB_ADDSTRING, 0, (LPARAM)L"TOKEN_MAYOR_QUE");
    SendMessageW(hComboToken_, CB_ADDSTRING, 0, (LPARAM)L"TOKEN_MAYOR_IGUAL_QUE");
    SendMessageW(hComboToken_, CB_ADDSTRING, 0, (LPARAM)L"TOKEN_MENOR_QUE");
    SendMessageW(hComboToken_, CB_ADDSTRING, 0, (LPARAM)L"TOKEN_MENOR_IGUAL_QUE");
    SendMessageW(hComboToken_, CB_ADDSTRING, 0, (LPARAM)L"TOKEN_IGUAL_QUE");
    SendMessageW(hComboToken_, CB_ADDSTRING, 0, (LPARAM)L"TOKEN_DISTINTO_QUE");
    SendMessageW(hComboToken_, CB_ADDSTRING, 0, (LPARAM)L"TOKEN_LIKE");
    SendMessageW(hComboToken_, CB_ADDSTRING, 0, (LPARAM)L"TOKEN_EMPIEZA_POR");
    SendMessageW(hComboToken_, CB_ADDSTRING, 0, (LPARAM)L"TOKEN_TERMINA_CON");
    SendMessageW(hComboToken_, CB_ADDSTRING, 0, (LPARAM)L"TOKEN_IN");
    SendMessageW(hComboToken_, CB_ADDSTRING, 0, (LPARAM)L"TOKEN_NOT_IN");
    SendMessageW(hComboToken_, CB_SETCURSEL, 0, 0);

    AddListColumn(0, L"No.", 50);
    AddListColumn(1, L"Simbolo / Frase de Operador (Operadores NL)", 400);

    Refresh();
}

std::string OperadoresNlConfigPanel::GetSelectedToken() const {
    int idx = (int)SendMessageW(hComboToken_, CB_GETCURSEL, 0, 0);
    if (idx == CB_ERR) return "TOKEN_MAYOR_QUE";
    wchar_t buf[64];
    SendMessageW(hComboToken_, CB_GETLBTEXT, idx, (LPARAM)buf);
    return WideToUtf8(buf);
}

void OperadoresNlConfigPanel::Refresh() {
    ClearList();
    if (!jsonLoaded_) return;

    std::string token = GetSelectedToken();
    if (!jsonRoot_.contains("configuracion_lexica") ||
        !jsonRoot_["configuracion_lexica"].contains("operadores_nl") ||
        !jsonRoot_["configuracion_lexica"]["operadores_nl"].contains(token)) {
        return;
    }

    const auto& arr = jsonRoot_["configuracion_lexica"]["operadores_nl"][token];
    int row = 0;
    for (const auto& item : arr) {
        std::wstring itemStr = Utf8ToWide(item.get<std::string>());
        AddListItem(row, std::to_wstring(row + 1), itemStr);
        row++;
    }
}

void OperadoresNlConfigPanel::OnComboSelect() {
    Refresh();
}

void OperadoresNlConfigPanel::OnAdd() {
    std::wstring newItem = GetEditText(hEditNewItem_);
    if (newItem.empty()) {
        MessageBoxW(hWndParent_, L"Por favor ingrese un operador o frase valida.", L"Advertencia", MB_OK | MB_ICONWARNING);
        return;
    }

    std::string token = GetSelectedToken();
    std::string newItemUtf8 = WideToUtf8(newItem);

    jsonRoot_["configuracion_lexica"]["operadores_nl"][token].push_back(newItemUtf8);
    SetEditText(hEditNewItem_, L"");
    Refresh();
}

void OperadoresNlConfigPanel::OnDelete() {
    int selIdx = ListView_GetNextItem(hListItems_, -1, LVNI_SELECTED);
    if (selIdx < 0) {
        MessageBoxW(hWndParent_, L"Seleccione un elemento de la lista para eliminar.", L"Advertencia", MB_OK | MB_ICONWARNING);
        return;
    }

    std::string token = GetSelectedToken();
    auto& arr = jsonRoot_["configuracion_lexica"]["operadores_nl"][token];
    if (selIdx < (int)arr.size()) {
        arr.erase(arr.begin() + selIdx);
    }
    Refresh();
}

void OperadoresNlConfigPanel::OnSave() {
    if (SaveSchemaJson()) {
        MessageBoxW(hWndParent_, L"Operadores NL guardados exitosamente en config/schema_master.json", L"Exito", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBoxW(hWndParent_, L"Error al guardar los cambios en config/schema_master.json", L"Error", MB_OK | MB_ICONERROR);
    }
}

}
