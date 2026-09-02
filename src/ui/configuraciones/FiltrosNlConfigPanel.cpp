#include "FiltrosNlConfigPanel.h"
#include "../StringConvert.h"
#include <windows.h>

namespace ui::configuraciones {

FiltrosNlConfigPanel::FiltrosNlConfigPanel() {}

FiltrosNlConfigPanel::~FiltrosNlConfigPanel() {}

void FiltrosNlConfigPanel::Create(HWND hWndParent, const wchar_t* title, const wchar_t* subtitle) {
    ConfiguracionBasePanel::Create(hWndParent, title, subtitle);

    ShowWindow(hComboToken_, SW_HIDE);

    AddListColumn(0, L"No.", 50);
    AddListColumn(1, L"Palabra de Relleno / Ignorada (TOKEN_IGNORE)", 400);

    Refresh();
}

void FiltrosNlConfigPanel::Refresh() {
    ClearList();
    if (!jsonLoaded_) return;

    if (!jsonRoot_.contains("configuracion_lexica") ||
        !jsonRoot_["configuracion_lexica"].contains("filtros_nl") ||
        !jsonRoot_["configuracion_lexica"]["filtros_nl"].contains("TOKEN_IGNORE")) {
        return;
    }

    const auto& arr = jsonRoot_["configuracion_lexica"]["filtros_nl"]["TOKEN_IGNORE"];
    int row = 0;
    for (const auto& item : arr) {
        std::wstring itemStr = Utf8ToWide(item.get<std::string>());
        AddListItem(row, std::to_wstring(row + 1), itemStr);
        row++;
    }
}

void FiltrosNlConfigPanel::OnAdd() {
    std::wstring newItem = GetEditText(hEditNewItem_);
    if (newItem.empty()) {
        MessageBoxW(hWndParent_, L"Por favor ingrese una palabra para ignorar.", L"Advertencia", MB_OK | MB_ICONWARNING);
        return;
    }

    std::string newItemUtf8 = WideToUtf8(newItem);
    jsonRoot_["configuracion_lexica"]["filtros_nl"]["TOKEN_IGNORE"].push_back(newItemUtf8);
    SetEditText(hEditNewItem_, L"");
    Refresh();
}

void FiltrosNlConfigPanel::OnDelete() {
    int selIdx = ListView_GetNextItem(hListItems_, -1, LVNI_SELECTED);
    if (selIdx < 0) {
        MessageBoxW(hWndParent_, L"Seleccione un elemento de la lista para eliminar.", L"Advertencia", MB_OK | MB_ICONWARNING);
        return;
    }

    auto& arr = jsonRoot_["configuracion_lexica"]["filtros_nl"]["TOKEN_IGNORE"];
    if (selIdx < (int)arr.size()) {
        arr.erase(arr.begin() + selIdx);
    }
    Refresh();
}

void FiltrosNlConfigPanel::OnSave() {
    if (SaveSchemaJson()) {
        MessageBoxW(hWndParent_, L"Filtros NL guardados exitosamente en config/schema_master.json", L"Exito", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBoxW(hWndParent_, L"Error al guardar los cambios en config/schema_master.json", L"Error", MB_OK | MB_ICONERROR);
    }
}

}
