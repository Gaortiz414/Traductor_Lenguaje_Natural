#pragma once

#include <windows.h>
#include <memory>
#include "ConfiguracionBasePanel.h"
#include "DmlConfigPanel.h"
#include "DdlConfigPanel.h"
#include "FiltrosNlConfigPanel.h"
#include "OperadoresNlConfigPanel.h"
#include "EsquemaBaseConfigPanel.h"
#include "RelacionesConfigPanel.h"

namespace ui::configuraciones {

enum class ConfiguracionPanelType {
    None,
    DML,
    DDL,
    FiltrosNL,
    OperadoresNL,
    EsquemaBase,
    Relaciones
};

void InitConfiguraciones(HWND hWndParent);
void LayoutActiveConfiguracion(HWND hWndParent);
void ShowConfiguracionPanel(HWND hWndParent, ConfiguracionPanelType type);
void HideAllConfiguracionPanels();
bool ProcessConfiguracionCommand(WORD id, WORD code);
bool IsAnyConfiguracionVisible();

}
