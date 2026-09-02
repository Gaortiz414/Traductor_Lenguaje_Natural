#pragma once

#include "ConfiguracionBasePanel.h"

namespace ui::configuraciones {

class EsquemaBaseConfigPanel : public ConfiguracionBasePanel {
public:
    EsquemaBaseConfigPanel();
    ~EsquemaBaseConfigPanel() override;

    void Create(HWND hWndParent, const wchar_t* title, const wchar_t* subtitle) override;
    void Refresh() override;
    void OnAdd() override;
    void OnDelete() override;
    void OnSave() override;
    void OnComboSelect() override;

private:
    std::string GetSelectedTable() const;
};

}
