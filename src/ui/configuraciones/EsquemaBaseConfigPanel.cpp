#include "EsquemaBaseConfigPanel.h"
#include "../StringConvert.h"
#include <windows.h>

namespace ui::configuraciones {

EsquemaBaseConfigPanel::EsquemaBaseConfigPanel() {}

EsquemaBaseConfigPanel::~EsquemaBaseConfigPanel() {}

void EsquemaBaseConfigPanel::Create(HWND hWndParent, const wchar_t* title, const wchar_t* subtitle) {
    ConfiguracionBasePanel::Create(hWndParent, title, subtitle);

    AddListColumn(0, L"Columna Canonica", 150);
    AddListColumn(1, L"Tipo de Dato", 100);

    Refresh();
}

std::string EsquemaBaseConfigPanel::GetSelectedTable() const {
    int idx = (int)SendMessageW(hComboToken_, CB_GETCURSEL, 0, 0);
    if (idx == CB_ERR) return "";
    wchar_t buf[64];
    SendMessageW(hComboToken_, CB_GETLBTEXT, idx, (LPARAM)buf);
    return WideToUtf8(buf);
}

void EsquemaBaseConfigPanel::Refresh() {
    ClearList();
    SendMessageW(hComboToken_, CB_RESETCONTENT, 0, 0);

    if (!jsonLoaded_) return;

    if (!jsonRoot_.contains("esquema_base") ||
        !jsonRoot_["esquema_base"].contains("tablas")) {
        return;
    }

    const auto& tablas = jsonRoot_["esquema_base"]["tablas"];
    for (const auto& t : tablas) {
        std::string idCanonico = t.value("id_canonico", "");
        if (!idCanonico.empty()) {
            SendMessageW(hComboToken_, CB_ADDSTRING, 0, (LPARAM)Utf8ToWide(idCanonico).c_str());
        }
    }

    if (SendMessageW(hComboToken_, CB_GETCOUNT, 0, 0) > 0) {
        SendMessageW(hComboToken_, CB_SETCURSEL, 0, 0);
    }

    OnComboSelect();
}

void EsquemaBaseConfigPanel::OnComboSelect() {
    ClearList();
    if (!jsonLoaded_) return;

    std::string selectedTable = GetSelectedTable();
    if (selectedTable.empty()) return;

    if (!jsonRoot_.contains("esquema_base") ||
        !jsonRoot_["esquema_base"].contains("tablas")) {
        return;
    }

    const auto& tablas = jsonRoot_["esquema_base"]["tablas"];
    for (const auto& t : tablas) {
        if (t.value("id_canonico", "") == selectedTable) {
            if (t.contains("columnas")) {
                int row = 0;
                for (const auto& col : t["columnas"]) {
                    std::string idCol = col.value("id_canonico", "");
                    std::string tipoCol = col.value("tipo_dato", "");
                    AddListItem(row, Utf8ToWide(idCol), Utf8ToWide(tipoCol));
                    row++;
                }
            }
            break;
        }
    }
}

void EsquemaBaseConfigPanel::OnAdd() {
    std::wstring newItem = GetEditText(hEditNewItem_);
    if (newItem.empty()) {
        MessageBoxW(hWndParent_, L"Por favor ingrese una nueva tabla o columna.", L"Advertencia", MB_OK | MB_ICONWARNING);
        return;
    }

    std::string selectedTable = GetSelectedTable();
    std::string newItemUtf8 = WideToUtf8(newItem);

    if (selectedTable.empty()) {
        json newTable;
        newTable["id_canonico"] = newItemUtf8;
        newTable["alias_nl"] = json::array();
        newTable["columnas"] = json::array();
        jsonRoot_["esquema_base"]["tablas"].push_back(newTable);
    } else {
        for (auto& t : jsonRoot_["esquema_base"]["tablas"]) {
            if (t.value("id_canonico", "") == selectedTable) {
                json newCol;
                newCol["id_canonico"] = newItemUtf8;
                newCol["alias_nl"] = json::array();
                newCol["tipo_dato"] = "varchar";
                t["columnas"].push_back(newCol);
                break;
            }
        }
    }

    SetEditText(hEditNewItem_, L"");
    Refresh();
}

void EsquemaBaseConfigPanel::OnDelete() {
    int selIdx = ListView_GetNextItem(hListItems_, -1, LVNI_SELECTED);
    std::string selectedTable = GetSelectedTable();

    if (selIdx >= 0 && !selectedTable.empty()) {
        for (auto& t : jsonRoot_["esquema_base"]["tablas"]) {
            if (t.value("id_canonico", "") == selectedTable) {
                if (t.contains("columnas") && selIdx < (int)t["columnas"].size()) {
                    t["columnas"].erase(t["columnas"].begin() + selIdx);
                }
                break;
            }
        }
        OnComboSelect();
    } else if (!selectedTable.empty()) {
        auto& tablas = jsonRoot_["esquema_base"]["tablas"];
        for (auto it = tablas.begin(); it != tablas.end(); ++it) {
            if ((*it).value("id_canonico", "") == selectedTable) {
                tablas.erase(it);
                break;
            }
        }
        Refresh();
    }
}

void EsquemaBaseConfigPanel::OnSave() {
    if (SaveSchemaJson()) {
        MessageBoxW(hWndParent_, L"Esquema Base guardado exitosamente en config/schema_master.json", L"Exito", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBoxW(hWndParent_, L"Error al guardar los cambios en config/schema_master.json", L"Error", MB_OK | MB_ICONERROR);
    }
}

}
