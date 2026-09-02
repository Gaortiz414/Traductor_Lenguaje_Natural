#pragma once

#include "ConfiguracionBasePanel.h"

namespace ui::configuraciones {

class RelacionesConfigPanel : public ConfiguracionBasePanel {
public:
    RelacionesConfigPanel();
    ~RelacionesConfigPanel() override;

    void Create(HWND hWndParent, const wchar_t* title, const wchar_t* subtitle) override;
    void Refresh() override;
    void OnAdd() override;
    void OnDelete() override;
    void OnSave() override;
};

}
