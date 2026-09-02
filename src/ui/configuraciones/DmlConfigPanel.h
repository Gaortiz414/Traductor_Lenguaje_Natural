#pragma once

#include "ConfiguracionBasePanel.h"

namespace ui::configuraciones {

class DmlConfigPanel : public ConfiguracionBasePanel {
public:
    DmlConfigPanel();
    ~DmlConfigPanel() override;

    void Create(HWND hWndParent, const wchar_t* title, const wchar_t* subtitle) override;
    void Refresh() override;
    void OnAdd() override;
    void OnDelete() override;
    void OnSave() override;
    void OnComboSelect() override;

private:
    std::string GetSelectedToken() const;
};

}
