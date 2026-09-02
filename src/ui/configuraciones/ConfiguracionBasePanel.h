#pragma once

#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include "../../../libs/json/json.hpp"

namespace ui::configuraciones {

using json = nlohmann::json;

class ConfiguracionBasePanel {
public:
    ConfiguracionBasePanel();
    virtual ~ConfiguracionBasePanel();

    virtual void Create(HWND hWndParent, const wchar_t* title, const wchar_t* subtitle);
    virtual void Layout(RECT rc);
    virtual void Show(bool show);
    bool IsVisible() const { return isVisible_; }

    virtual void Refresh() = 0;
    virtual void OnAdd() = 0;
    virtual void OnDelete() = 0;
    virtual void OnSave() = 0;
    virtual void OnComboSelect() {}

    bool ProcessCommand(WORD id, WORD code);

protected:
    bool LoadSchemaJson();
    bool SaveSchemaJson();

    void ClearList();
    void AddListColumn(int colIdx, const wchar_t* text, int width);
    void AddListItem(int index, const std::wstring& col1Text, const std::wstring& col2Text = L"");

    std::wstring GetEditText(HWND hEdit);
    void SetEditText(HWND hEdit, const std::wstring& text);

    std::string GetSchemaPath() const;

    HWND hWndParent_ = nullptr;
    HWND hLblTitle_ = nullptr;
    HWND hLblSubtitle_ = nullptr;
    HWND hComboToken_ = nullptr;
    HWND hListItems_ = nullptr;
    HWND hEditNewItem_ = nullptr;
    HWND hBtnAdd_ = nullptr;
    HWND hBtnDel_ = nullptr;
    HWND hBtnSave_ = nullptr;

    bool isVisible_ = false;
    json jsonRoot_;
    bool jsonLoaded_ = false;
};

}
