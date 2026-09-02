#include "../../../framework.h"
#include "../../../Resource.h"
#include "TablaSimbolosPanel.h"
#include "AnalisisLexicoPanel.h"
#include "../MainWindow.h"
#include "../StringConvert.h"
#include "../../nl2sql/SchemaMapper.h"
#include "../../nl2sql/NlTokenizer.h"
#include "../configuraciones/ConfiguracionesManager.h"
#include <commctrl.h>
#include <string>
#include <vector>

namespace ui::tablasimbolos {

namespace {

HWND hLblTsInput   = nullptr;
HWND hEditTsInput  = nullptr;
HWND hBtnAnalizar  = nullptr;
HWND hListTokens   = nullptr;
bool g_panelTablaSimbolosVisible = false;

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

void CreatePanelTablaSimbolos(HWND hWnd)
{
    hLblTsInput = CreateWindowExW(0, L"STATIC", L"Escribe que quieres hacer",
        WS_CHILD | SS_LEFT,
        0, 0, 0, 0, hWnd, (HMENU)(INT_PTR)IDC_LBL_TS_INPUT, hInst, nullptr);

    hEditTsInput = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL,
        0, 0, 0, 0, hWnd, (HMENU)(INT_PTR)IDC_EDIT_TS_INPUT, hInst, nullptr);

    hBtnAnalizar = CreateWindowExW(0, L"BUTTON", L"Analizar",
        WS_CHILD | BS_PUSHBUTTON,
        0, 0, 0, 0, hWnd, (HMENU)(INT_PTR)IDC_BTN_TS_ANALIZAR, hInst, nullptr);

    hListTokens = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
        WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS,
        0, 0, 0, 0, hWnd, (HMENU)(INT_PTR)IDC_LV_TS_TOKENS, hInst, nullptr);
    ListView_SetExtendedListViewStyle(hListTokens, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    LVCOLUMNW col{};
    col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    wchar_t colLexema[] = L"Lexema";
    col.pszText = colLexema; col.cx = 120; col.iSubItem = 0;
    ListView_InsertColumn(hListTokens, 0, &col);

    wchar_t colToken[] = L"Token";
    col.pszText = colToken; col.cx = 120; col.iSubItem = 1;
    ListView_InsertColumn(hListTokens, 1, &col);

    wchar_t colDescripcion[] = L"Descripcion";
    col.pszText = colDescripcion; col.cx = 300; col.iSubItem = 2;
    ListView_InsertColumn(hListTokens, 2, &col);

    wchar_t colReservada[] = L"Reservada";
    col.pszText = colReservada; col.cx = 90; col.iSubItem = 3;
    ListView_InsertColumn(hListTokens, 3, &col);

    wchar_t colLinea[] = L"Linea";
    col.pszText = colLinea; col.cx = 70; col.iSubItem = 4;
    ListView_InsertColumn(hListTokens, 4, &col);

    wchar_t colColumna[] = L"Columna";
    col.pszText = colColumna; col.cx = 70; col.iSubItem = 5;
    ListView_InsertColumn(hListTokens, 5, &col);

    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessageW(hLblTsInput,  WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessageW(hEditTsInput, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessageW(hBtnAnalizar, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessageW(hListTokens,  WM_SETFONT, (WPARAM)hFont, TRUE);
}

void LayoutPanelTablaSimbolos(HWND hWnd)
{
    if (!hEditTsInput || !g_panelTablaSimbolosVisible)
        return;

    RECT rc;
    GetClientRect(hWnd, &rc);
    const int margin = 12;
    const int width  = (rc.right - rc.left) - (margin * 2);
    if (width <= 0)
        return;

    const int labelHeight = 18;
    const int inputHeight = (int)((rc.bottom - rc.top) * 0.25);
    const int btnHeight   = 28;
    const int btnWidth    = 140;

    int y = margin;
    MoveWindow(hLblTsInput, margin, y, width, labelHeight, TRUE);
    y += labelHeight;

    MoveWindow(hEditTsInput, margin, y, width, inputHeight, TRUE);
    y += inputHeight + margin;

    MoveWindow(hBtnAnalizar, margin, y, btnWidth, btnHeight, TRUE);
    y += btnHeight + margin;

    int listHeight = (rc.bottom - rc.top) - y - margin;
    if (listHeight < 60) listHeight = 60;
    MoveWindow(hListTokens, margin, y, width, listHeight, TRUE);

    if (width > 0) {
        int wLexema = (int)(width * 0.16);
        int wToken = (int)(width * 0.15);
        int wReservada = (int)(width * 0.12);
        int wLinea = (int)(width * 0.10);
        int wColumna = (int)(width * 0.10);
        int wDescripcion = width - wLexema - wToken - wReservada - wLinea - wColumna - 4;
        if (wDescripcion < 100) wDescripcion = 100;
        ListView_SetColumnWidth(hListTokens, 0, wLexema);
        ListView_SetColumnWidth(hListTokens, 1, wToken);
        ListView_SetColumnWidth(hListTokens, 2, wDescripcion);
        ListView_SetColumnWidth(hListTokens, 3, wReservada);
        ListView_SetColumnWidth(hListTokens, 4, wLinea);
        ListView_SetColumnWidth(hListTokens, 5, wColumna);
    }
}

void HidePanelTablaSimbolos(HWND hWnd)
{
    UNREFERENCED_PARAMETER(hWnd);
    if (!g_panelTablaSimbolosVisible)
        return;

    g_panelTablaSimbolosVisible = false;
    ShowWindow(hLblTsInput,  SW_HIDE);
    ShowWindow(hEditTsInput, SW_HIDE);
    ShowWindow(hBtnAnalizar, SW_HIDE);
    ShowWindow(hListTokens,  SW_HIDE);
}

void TogglePanelTablaSimbolos(HWND hWnd)
{
    if (!g_panelTablaSimbolosVisible) {
        ui::analisislexico::HidePanelNatural(hWnd);
        ui::configuraciones::HideAllConfiguracionPanels();
        g_panelTablaSimbolosVisible = true;
    } else {
        g_panelTablaSimbolosVisible = false;
    }

    int cmdShow = g_panelTablaSimbolosVisible ? SW_SHOW : SW_HIDE;

    ShowWindow(hLblTsInput,  cmdShow);
    ShowWindow(hEditTsInput, cmdShow);
    ShowWindow(hBtnAnalizar, cmdShow);
    ShowWindow(hListTokens,  cmdShow);

    if (g_panelTablaSimbolosVisible)
        LayoutPanelTablaSimbolos(hWnd);
}

void OnAnalizarTablaSimbolos(HWND hWnd)
{
    UNREFERENCED_PARAMETER(hWnd);

    int len = GetWindowTextLengthW(hEditTsInput);
    std::wstring wideInput(len, L'\0');
    if (len > 0) GetWindowTextW(hEditTsInput, wideInput.data(), len + 1);
    std::string source = WideToUtf8(wideInput);

    ListView_DeleteAllItems(hListTokens);

    bool schemaLoadedOk = false;
    const nl2sql::SchemaMapper& schema = GetSchemaMapper(&schemaLoadedOk);
    if (!schemaLoadedOk) {
        LVITEMW item{};
        item.mask = LVIF_TEXT;
        item.iItem = 0;
        item.iSubItem = 0;
        wchar_t msg[] = L"No se pudo cargar config\\schema_master.json";
        item.pszText = msg;
        ListView_InsertItem(hListTokens, &item);
        return;
    }

    nl2sql::NlTokenizer tokenizer(schema);
    std::vector<nl2sql::NlToken> tokens = tokenizer.Tokenize(source);

    for (size_t i = 0; i < tokens.size(); i++) {
        const nl2sql::NlToken& tok = tokens[i];

        std::wstring wTokenName = Utf8ToWide(tok.tokenName);
        std::wstring wLexema    = Utf8ToWide(tok.lexema);
        std::wstring wPatron    = Utf8ToWide(tok.patron);
        std::wstring wReservada = tok.esPalabraReservada ? L"Si" : L"No";
        std::wstring wLinea     = std::to_wstring(tok.linea);
        std::wstring wColumna   = std::to_wstring(tok.columna);

        LVITEMW item{};
        item.mask = LVIF_TEXT;
        item.iItem = (int)i;
        item.iSubItem = 0;
        item.pszText = const_cast<LPWSTR>(wLexema.c_str());
        int idx = ListView_InsertItem(hListTokens, &item);

        ListView_SetItemText(hListTokens, idx, 1, const_cast<LPWSTR>(wTokenName.c_str()));
        ListView_SetItemText(hListTokens, idx, 2, const_cast<LPWSTR>(wPatron.c_str()));
        ListView_SetItemText(hListTokens, idx, 3, const_cast<LPWSTR>(wReservada.c_str()));
        ListView_SetItemText(hListTokens, idx, 4, const_cast<LPWSTR>(wLinea.c_str()));
        ListView_SetItemText(hListTokens, idx, 5, const_cast<LPWSTR>(wColumna.c_str()));
    }
}

void OnTablaSimbolos(HWND hWnd)
{
    TogglePanelTablaSimbolos(hWnd);
}

}
