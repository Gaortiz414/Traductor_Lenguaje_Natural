#include "RelacionesConfigPanel.h"
#include "../StringConvert.h"
#include <windows.h>

namespace ui::configuraciones {

RelacionesConfigPanel::RelacionesConfigPanel() {}

RelacionesConfigPanel::~RelacionesConfigPanel() {}

void RelacionesConfigPanel::Create(HWND hWndParent, const wchar_t* title, const wchar_t* subtitle) {
    ConfiguracionBasePanel::Create(hWndParent, title, subtitle);

    ShowWindow(hComboToken_, SW_HIDE);

    AddListColumn(0, L"Origen -> Destino", 160);
    AddListColumn(1, L"Condicion JOIN", 280);

    Refresh();
}

void RelacionesConfigPanel::Refresh() {
    ClearList();
    if (!jsonLoaded_) return;

    if (!jsonRoot_.contains("relaciones")) {
        return;
    }

    const auto& rels = jsonRoot_["relaciones"];
    int row = 0;
    for (const auto& r : rels) {
        std::string origen = r.value("origen", "");
        std::string destino = r.value("destino", "");
        std::string cond = r.value("condicion_join", "");

        std::wstring col1 = Utf8ToWide(origen + " -> " + destino);
        std::wstring col2 = Utf8ToWide(cond);
        AddListItem(row, col1, col2);
        row++;
    }
}

void RelacionesConfigPanel::OnAdd() {
    std::wstring newItem = GetEditText(hEditNewItem_);
    if (newItem.empty()) {
        MessageBoxW(hWndParent_, L"Por favor ingrese una nueva relacion en formato 'tabla1.col1 = tabla2.col2'.", L"Advertencia", MB_OK | MB_ICONWARNING);
        return;
    }

    std::string text = WideToUtf8(newItem);
    json newRel;
    newRel["origen"] = "ventas";
    newRel["destino"] = "clientes";
    newRel["condicion_join"] = text;
    newRel["tipo_default"] = "INNER JOIN";

    jsonRoot_["relaciones"].push_back(newRel);
    SetEditText(hEditNewItem_, L"");
    Refresh();
}

void RelacionesConfigPanel::OnDelete() {
    int selIdx = ListView_GetNextItem(hListItems_, -1, LVNI_SELECTED);
    if (selIdx < 0) {
        MessageBoxW(hWndParent_, L"Seleccione un elemento de la lista para eliminar.", L"Advertencia", MB_OK | MB_ICONWARNING);
        return;
    }

    auto& rels = jsonRoot_["relaciones"];
    if (selIdx < (int)rels.size()) {
        rels.erase(rels.begin() + selIdx);
    }
    Refresh();
}

void RelacionesConfigPanel::OnSave() {
    if (SaveSchemaJson()) {
        MessageBoxW(hWndParent_, L"Relaciones guardadas exitosamente en config/schema_master.json", L"Exito", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBoxW(hWndParent_, L"Error al guardar los cambios en config/schema_master.json", L"Error", MB_OK | MB_ICONERROR);
    }
}

}
