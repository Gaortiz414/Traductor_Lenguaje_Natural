#include "ConfiguracionesManager.h"
#include "../menus/AnalisisLexicoPanel.h"
#include "../menus/TablaSimbolosPanel.h"
#include "../ErrorPanel.h"

namespace ui::configuraciones {

namespace {
std::unique_ptr<DmlConfigPanel> g_pDmlPanel;
std::unique_ptr<DdlConfigPanel> g_pDdlPanel;
std::unique_ptr<FiltrosNlConfigPanel> g_pFiltrosPanel;
std::unique_ptr<OperadoresNlConfigPanel> g_pOperadoresPanel;
std::unique_ptr<EsquemaBaseConfigPanel> g_pEsquemaPanel;
std::unique_ptr<RelacionesConfigPanel> g_pRelacionesPanel;
ConfiguracionPanelType g_currentType = ConfiguracionPanelType::None;
}

void InitConfiguraciones(HWND hWndParent) {
    g_pDmlPanel = std::make_unique<DmlConfigPanel>();
    g_pDmlPanel->Create(hWndParent, L"Configuracion Palabras Reservadas DML", L"Permite agregar, modificar y eliminar alias de SELECT, FROM, WHERE, AND, OR.");

    g_pDdlPanel = std::make_unique<DdlConfigPanel>();
    g_pDdlPanel->Create(hWndParent, L"Configuracion Palabras Reservadas DDL", L"Permite agregar, modificar y eliminar alias de comandos DDL (CREATE, etc.).");

    g_pFiltrosPanel = std::make_unique<FiltrosNlConfigPanel>();
    g_pFiltrosPanel->Create(hWndParent, L"Configuracion Filtros NL (Palabras Ignoradas)", L"Gestion de palabras de relleno/ignoradas en lenguaje natural (TOKEN_IGNORE).");

    g_pOperadoresPanel = std::make_unique<OperadoresNlConfigPanel>();
    g_pOperadoresPanel->Create(hWndParent, L"Configuracion Operadores NL", L"Gestion de simbolos y frases comparativas en lenguaje natural (>, <, =, LIKE, etc.).");

    g_pEsquemaPanel = std::make_unique<EsquemaBaseConfigPanel>();
    g_pEsquemaPanel->Create(hWndParent, L"Configuracion Esquema Base", L"Gestion de tablas canonicas y columnas del esquema de base de datos.");

    g_pRelacionesPanel = std::make_unique<RelacionesConfigPanel>();
    g_pRelacionesPanel->Create(hWndParent, L"Configuracion Relaciones entre Tablas", L"Gestion de rutas de union (JOIN) e integridad referencial.");
}

void LayoutActiveConfiguracion(HWND hWndParent) {
    if (g_currentType == ConfiguracionPanelType::None) return;

    RECT rc;
    GetClientRect(hWndParent, &rc);

    switch (g_currentType) {
    case ConfiguracionPanelType::DML:
        if (g_pDmlPanel) g_pDmlPanel->Layout(rc);
        break;
    case ConfiguracionPanelType::DDL:
        if (g_pDdlPanel) g_pDdlPanel->Layout(rc);
        break;
    case ConfiguracionPanelType::FiltrosNL:
        if (g_pFiltrosPanel) g_pFiltrosPanel->Layout(rc);
        break;
    case ConfiguracionPanelType::OperadoresNL:
        if (g_pOperadoresPanel) g_pOperadoresPanel->Layout(rc);
        break;
    case ConfiguracionPanelType::EsquemaBase:
        if (g_pEsquemaPanel) g_pEsquemaPanel->Layout(rc);
        break;
    case ConfiguracionPanelType::Relaciones:
        if (g_pRelacionesPanel) g_pRelacionesPanel->Layout(rc);
        break;
    default:
        break;
    }
}

void HideAllConfiguracionPanels() {
    if (g_pDmlPanel) g_pDmlPanel->Show(false);
    if (g_pDdlPanel) g_pDdlPanel->Show(false);
    if (g_pFiltrosPanel) g_pFiltrosPanel->Show(false);
    if (g_pOperadoresPanel) g_pOperadoresPanel->Show(false);
    if (g_pEsquemaPanel) g_pEsquemaPanel->Show(false);
    if (g_pRelacionesPanel) g_pRelacionesPanel->Show(false);
    g_currentType = ConfiguracionPanelType::None;
}

void ShowConfiguracionPanel(HWND hWndParent, ConfiguracionPanelType type) {
    // Cross-hiding other main panels
    ui::analisislexico::HidePanelNatural(hWndParent);
    ui::tablasimbolos::HidePanelTablaSimbolos(hWndParent);
    ui::errorpanel::SetErrorIconVisible(false);

    HideAllConfiguracionPanels();
    g_currentType = type;

    RECT rc;
    GetClientRect(hWndParent, &rc);

    switch (type) {
    case ConfiguracionPanelType::DML:
        if (g_pDmlPanel) { g_pDmlPanel->Show(true); g_pDmlPanel->Layout(rc); }
        break;
    case ConfiguracionPanelType::DDL:
        if (g_pDdlPanel) { g_pDdlPanel->Show(true); g_pDdlPanel->Layout(rc); }
        break;
    case ConfiguracionPanelType::FiltrosNL:
        if (g_pFiltrosPanel) { g_pFiltrosPanel->Show(true); g_pFiltrosPanel->Layout(rc); }
        break;
    case ConfiguracionPanelType::OperadoresNL:
        if (g_pOperadoresPanel) { g_pOperadoresPanel->Show(true); g_pOperadoresPanel->Layout(rc); }
        break;
    case ConfiguracionPanelType::EsquemaBase:
        if (g_pEsquemaPanel) { g_pEsquemaPanel->Show(true); g_pEsquemaPanel->Layout(rc); }
        break;
    case ConfiguracionPanelType::Relaciones:
        if (g_pRelacionesPanel) { g_pRelacionesPanel->Show(true); g_pRelacionesPanel->Layout(rc); }
        break;
    default:
        break;
    }
}

bool ProcessConfiguracionCommand(WORD id, WORD code) {
    if (g_pDmlPanel && g_pDmlPanel->ProcessCommand(id, code)) return true;
    if (g_pDdlPanel && g_pDdlPanel->ProcessCommand(id, code)) return true;
    if (g_pFiltrosPanel && g_pFiltrosPanel->ProcessCommand(id, code)) return true;
    if (g_pOperadoresPanel && g_pOperadoresPanel->ProcessCommand(id, code)) return true;
    if (g_pEsquemaPanel && g_pEsquemaPanel->ProcessCommand(id, code)) return true;
    if (g_pRelacionesPanel && g_pRelacionesPanel->ProcessCommand(id, code)) return true;
    return false;
}

bool IsAnyConfiguracionVisible() {
    return g_currentType != ConfiguracionPanelType::None;
}

}
