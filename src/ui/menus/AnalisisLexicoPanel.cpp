#include "../../../framework.h"
#include "../../../Resource.h"
#include "AnalisisLexicoPanel.h"
#include "TablaSimbolosPanel.h"
#include "../MainWindow.h"
#include "../StringConvert.h"
#include "../configuraciones/ConfiguracionesManager.h"
#include "../../compiler/lexer/LexicalAnalyzer.h"
#include "../../compiler/syntax/Parser.h"
#include "../../compiler/symbols/SymbolTable.h"
#include "../../compiler/semantic/SemanticAnalyzer.h"
#include "../../nl2sql/SchemaMapper.h"
#include "../../nl2sql/SlotFiller.h"
#include "../../errors/ErrorDictionary.h"
#include "../ErrorPanel.h"
#include <string>
#include <vector>

namespace ui::analisislexico {

namespace {

HWND hLblNlInput    = nullptr;
HWND hEditNlInput   = nullptr;
HWND hChkModoToggle = nullptr;
HWND hLblNlOutput   = nullptr;
HWND hEditNlOutput  = nullptr;
HWND hBtnEjecutar   = nullptr;
bool g_panelNaturalVisible = false;
bool g_modoCotidiano       = true;

nl2sql::SchemaMapper GetSchemaMapper(bool* outLoadedOk = nullptr)
{
    nl2sql::SchemaMapper mapper;
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    std::wstring exeDir(exePath);
    size_t slash = exeDir.find_last_of(L"\\/");
    exeDir = (slash != std::wstring::npos) ? exeDir.substr(0, slash) : L".";

    std::string candidate1 = WideToUtf8(exeDir) + "\\config\\schema_master.json";
    bool loadedOk = mapper.LoadFromFile(candidate1) || mapper.LoadFromFile("config/schema_master.json");
    if (outLoadedOk) *outLoadedOk = loadedOk;
    return mapper;
}

}

void CreatePanelNatural(HWND hWnd)
{
    hLblNlInput = CreateWindowExW(0, L"STATIC", L"Escribe que quieres hacer",
        WS_CHILD | SS_LEFT,
        0, 0, 0, 0, hWnd, (HMENU)(INT_PTR)IDC_LBL_NL_INPUT, hInst, nullptr);

    hEditNlInput = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL,
        0, 0, 0, 0, hWnd, (HMENU)(INT_PTR)IDC_EDIT_NL_INPUT, hInst, nullptr);

    hChkModoToggle = CreateWindowExW(0, L"BUTTON", L"Modo Cotidiano",
        WS_CHILD | BS_AUTOCHECKBOX,
        0, 0, 0, 0, hWnd, (HMENU)(INT_PTR)IDC_CHK_MODO_TOGGLE, hInst, nullptr);
    CheckDlgButton(hWnd, IDC_CHK_MODO_TOGGLE, g_modoCotidiano ? BST_CHECKED : BST_UNCHECKED);

    hLblNlOutput = CreateWindowExW(0, L"STATIC", L"Resultado obtenido",
        WS_CHILD | SS_LEFT,
        0, 0, 0, 0, hWnd, (HMENU)(INT_PTR)IDC_LBL_NL_OUTPUT, hInst, nullptr);

    hEditNlOutput = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"Aqui se mostrara un mensaje de ejemplo (no funcional).",
        WS_CHILD | WS_BORDER | ES_MULTILINE | ES_READONLY,
        0, 0, 0, 0, hWnd, (HMENU)(INT_PTR)IDC_EDIT_NL_OUTPUT, hInst, nullptr);

    hBtnEjecutar = CreateWindowExW(0, L"BUTTON", L"Ejecutar",
        WS_CHILD | BS_PUSHBUTTON,
        0, 0, 0, 0, hWnd, (HMENU)(INT_PTR)IDC_BTN_EJECUTAR, hInst, nullptr);

    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessageW(hLblNlInput,    WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessageW(hEditNlInput,   WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessageW(hChkModoToggle, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessageW(hLblNlOutput,   WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessageW(hEditNlOutput,  WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessageW(hBtnEjecutar,   WM_SETFONT, (WPARAM)hFont, TRUE);
}

void LayoutPanelNatural(HWND hWnd)
{
    if (!hEditNlInput || !g_panelNaturalVisible)
        return;

    RECT rc;
    GetClientRect(hWnd, &rc);
    const int margin = 12;
    const int width  = (rc.right - rc.left) - (margin * 2);
    if (width <= 0)
        return;

    const int labelHeight  = 18;
    const int inputHeight  = (int)((rc.bottom - rc.top) * 0.40);
    const int chkHeight    = 24;
    const int chkWidth     = 220;

    int y = margin;
    MoveWindow(hLblNlInput, margin, y, width, labelHeight, TRUE);
    y += labelHeight;

    MoveWindow(hEditNlInput, margin, y, width, inputHeight, TRUE);
    y += inputHeight + margin;

    const int btnWidth     = 140;
    const int errBtnWidth  = 110;
    const int btnHeight    = 28;
    int btnX = margin + chkWidth + margin;
    MoveWindow(hChkModoToggle, margin, y, chkWidth, chkHeight, TRUE);
    MoveWindow(hBtnEjecutar, btnX, y - 2, btnWidth, btnHeight, TRUE);
    ui::errorpanel::MoveErrorButton(btnX + btnWidth + margin, y - 2, errBtnWidth, btnHeight);
    y += chkHeight + margin;

    MoveWindow(hLblNlOutput, margin, y, width, labelHeight, TRUE);
    y += labelHeight;

    int outputHeight = (rc.bottom - rc.top) - y - margin;
    if (outputHeight < 40) outputHeight = 40;
    MoveWindow(hEditNlOutput, margin, y, width, outputHeight, TRUE);
}

void HidePanelNatural(HWND hWnd)
{
    UNREFERENCED_PARAMETER(hWnd);
    if (!g_panelNaturalVisible)
        return;

    g_panelNaturalVisible = false;
    ShowWindow(hLblNlInput,    SW_HIDE);
    ShowWindow(hEditNlInput,   SW_HIDE);
    ShowWindow(hChkModoToggle, SW_HIDE);
    ShowWindow(hBtnEjecutar,   SW_HIDE);
    ShowWindow(hLblNlOutput,   SW_HIDE);
    ShowWindow(hEditNlOutput,  SW_HIDE);

    ui::errorpanel::SetErrorIconVisible(false);
}

void TogglePanelNatural(HWND hWnd)
{
    if (!g_panelNaturalVisible) {
        ui::tablasimbolos::HidePanelTablaSimbolos(hWnd);
        ui::configuraciones::HideAllConfiguracionPanels();
        g_panelNaturalVisible = true;
    } else {
        g_panelNaturalVisible = false;
    }

    int cmdShow = g_panelNaturalVisible ? SW_SHOW : SW_HIDE;

    ShowWindow(hLblNlInput,    cmdShow);
    ShowWindow(hEditNlInput,   cmdShow);
    ShowWindow(hChkModoToggle, cmdShow);
    ShowWindow(hBtnEjecutar,   cmdShow);
    ShowWindow(hLblNlOutput,   cmdShow);
    ShowWindow(hEditNlOutput,  cmdShow);

    ui::errorpanel::SetErrorIconVisible(g_panelNaturalVisible);

    if (g_panelNaturalVisible)
        LayoutPanelNatural(hWnd);
}

void OnModoToggleChanged(HWND hWnd)
{
    g_modoCotidiano = (IsDlgButtonChecked(hWnd, IDC_CHK_MODO_TOGGLE) == BST_CHECKED);
    SetWindowTextW(hChkModoToggle, g_modoCotidiano ? L"Modo Cotidiano" : L"Modo Lenguaje Formal");
}

void EjecutarAnalisisLexico(HWND hWnd)
{
    UNREFERENCED_PARAMETER(hWnd);

    int len = GetWindowTextLengthW(hEditNlInput);
    std::wstring wideInput(len, L'\0');
    if (len > 0) GetWindowTextW(hEditNlInput, wideInput.data(), len + 1);
    std::string source = WideToUtf8(wideInput);

    ui::errorpanel::ClearErrors();

    std::wstring errors;

    if (g_modoCotidiano) {
        bool schemaLoadedOk = false;
        const nl2sql::SchemaMapper& schema = GetSchemaMapper(&schemaLoadedOk);
        if (!schemaLoadedOk) {
            SetWindowTextW(hEditNlOutput, L"No se pudo cargar config\\schema_master.json (JSON maestro NL2SQL).");
            return;
        }
        nl2sql::SlotFiller slotFiller(schema);
        nl2sql::SlotFillResult fillResult = slotFiller.Fill(source);

        if (!fillResult.success) {
            for (const auto& err : fillResult.errors) {
                std::string msg = errors::GetErrorDictionary().Resolve(err.code, err.detail);
                ui::errorpanel::AddError(err.line, msg);
            }
            SetWindowTextW(hEditNlOutput, Utf8ToWide(errors::GetErrorDictionary().Resolve("GEN001")).c_str());
            return;
        }

        source = fillResult.query.ToSqlString();
    }

    std::wstring sqlResult = Utf8ToWide(source);

    compiler::lexer::LexicalAnalyzer lexer(source);
    std::vector<compiler::lexer::Token> tokens = lexer.Tokenize();

    bool hasErrors = false;

    if (!lexer.Errors().empty()) {
        for (const auto& err : lexer.Errors()) {
            std::string msg = errors::GetErrorDictionary().Resolve(err.code, err.detail);
            ui::errorpanel::AddError(err.line, msg);
            hasErrors = true;
        }
    }

    bool hasTokens = false;
    for (const auto& tok : tokens) {
        if (tok.type != compiler::lexer::TokenType::TOKEN_EOF) { hasTokens = true; break; }
    }
    if (!hasTokens) {
        if (hasErrors) {
            SetWindowTextW(hEditNlOutput, Utf8ToWide(errors::GetErrorDictionary().Resolve("GEN001")).c_str());
        } else {
            SetWindowTextW(hEditNlOutput, L"(sin resultado)");
        }
        return;
    }

    compiler::symbols::SymbolTable scriptSymbols;
    {
        bool baseSchemaLoadedOk = false;
        const nl2sql::SchemaMapper& baseSchema = GetSchemaMapper(&baseSchemaLoadedOk);
        if (baseSchemaLoadedOk) {
            for (const auto& table : baseSchema.Tables()) {
                std::vector<compiler::symbols::Symbol> columns;
                columns.reserve(table.columnas.size());
                for (const auto& col : table.columnas) {
                    columns.push_back(compiler::symbols::Symbol{
                        compiler::symbols::SymbolTable::Normalize(col.idCanonico),
                        compiler::symbols::SymbolKind::Column,
                        col.tipoDato });
                }
                scriptSymbols.InjectTable(table.idCanonico, columns);
            }
        }
    }

    compiler::syntax::Parser parser(tokens);
    size_t reportedParserErrors = 0;

    while (!parser.IsAtEnd()) {
        size_t errorsBefore = parser.Errors().size();
        auto statement = parser.ParseStatement();

        if (parser.Errors().size() > errorsBefore) {
            for (size_t i = reportedParserErrors; i < parser.Errors().size(); i++) {
                const auto& err = parser.Errors()[i];
                std::string msg = errors::GetErrorDictionary().Resolve(err.code, err.detail);
                ui::errorpanel::AddError(err.line, msg);
                hasErrors = true;
            }
            reportedParserErrors = parser.Errors().size();
            parser.Synchronize();
            continue;
        }

        if (statement) {
            compiler::semantic::SemanticAnalyzer semantic(scriptSymbols);
            semantic.Analyze(statement.get());

            for (const auto& err : semantic.Errors()) {
                std::string msg = errors::GetErrorDictionary().Resolve(err.code, err.detail);
                ui::errorpanel::AddError(0, msg);
                hasErrors = true;
            }
        }
    }

    if (hasErrors) {
        SetWindowTextW(hEditNlOutput, Utf8ToWide(errors::GetErrorDictionary().Resolve("GEN001")).c_str());
        return;
    }

    if (sqlResult.empty()) sqlResult = L"(sin resultado)";
    SetWindowTextW(hEditNlOutput, sqlResult.c_str());
}

}
