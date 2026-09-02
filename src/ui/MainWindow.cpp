#include "../../framework.h"
#include "../../Resource.h"
#include "MainWindow.h"
#include "menus/AnalisisLexicoPanel.h"
#include "menus/TablaSimbolosPanel.h"
#include "menus/InstruccionesPanel.h"
#include "configuraciones/ConfiguracionesManager.h"
#include "ErrorPanel.h"
#include <commctrl.h>
#pragma comment(lib, "Comctl32.lib")

HINSTANCE hInst;

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    extern WCHAR szWindowClass[];

    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TRADUCTORLENGUAJENATURAL));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TRADUCTORLENGUAJENATURAL);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance;

   INITCOMMONCONTROLSEX icc{};
   icc.dwSize = sizeof(icc);
   icc.dwICC = ICC_LISTVIEW_CLASSES;
   InitCommonControlsEx(&icc);

   extern WCHAR szTitle[];
   extern WCHAR szWindowClass[];

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        ui::analisislexico::CreatePanelNatural(hWnd);
        ui::tablasimbolos::CreatePanelTablaSimbolos(hWnd);
        ui::configuraciones::InitConfiguraciones(hWnd);
        ui::errorpanel::CreateErrorIcon(hWnd);
        ui::errorpanel::CreatePanel(hWnd);
        SetupMainMenu(hWnd);
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);

            switch (wmId)
            {
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case IDM_INSTRUCCIONES:
                ui::instrucciones::OnInstrucciones(hWnd);
                break;
            case IDM_TABLA_SIMBOLOS:
                ui::tablasimbolos::OnTablaSimbolos(hWnd);
                break;
            case IDM_PANEL_NATURAL:
                ui::analisislexico::TogglePanelNatural(hWnd);
                break;
            case IDM_CONFIG_DML:
                ui::configuraciones::ShowConfiguracionPanel(hWnd, ui::configuraciones::ConfiguracionPanelType::DML);
                break;
            case IDM_CONFIG_DDL:
                ui::configuraciones::ShowConfiguracionPanel(hWnd, ui::configuraciones::ConfiguracionPanelType::DDL);
                break;
            case IDM_CONFIG_FILTROS_NL:
                ui::configuraciones::ShowConfiguracionPanel(hWnd, ui::configuraciones::ConfiguracionPanelType::FiltrosNL);
                break;
            case IDM_CONFIG_OPERADORES_NL:
                ui::configuraciones::ShowConfiguracionPanel(hWnd, ui::configuraciones::ConfiguracionPanelType::OperadoresNL);
                break;
            case IDM_CONFIG_ESQUEMA_BASE:
                ui::configuraciones::ShowConfiguracionPanel(hWnd, ui::configuraciones::ConfiguracionPanelType::EsquemaBase);
                break;
            case IDM_CONFIG_RELACIONES:
                ui::configuraciones::ShowConfiguracionPanel(hWnd, ui::configuraciones::ConfiguracionPanelType::Relaciones);
                break;
            case IDC_CHK_MODO_TOGGLE:
                ui::analisislexico::OnModoToggleChanged(hWnd);
                break;
            case IDC_BTN_EJECUTAR:
                ui::analisislexico::EjecutarAnalisisLexico(hWnd);
                break;
            case IDC_BTN_TS_ANALIZAR:
                ui::tablasimbolos::OnAnalizarTablaSimbolos(hWnd);
                break;
            case IDC_BTN_ERRORES:
                ui::errorpanel::TogglePanel(hWnd);
                break;
            default:
                if (ui::configuraciones::ProcessConfiguracionCommand(wmId, HIWORD(wParam)))
                    break;
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_SIZE:
        ui::analisislexico::LayoutPanelNatural(hWnd);
        ui::tablasimbolos::LayoutPanelTablaSimbolos(hWnd);
        ui::configuraciones::LayoutActiveConfiguracion(hWnd);
        ui::errorpanel::LayoutErrorIcon(hWnd);
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void SetupMainMenu(HWND hWnd)
{
    HMENU hMenu = GetMenu(hWnd);
    if (!hMenu)
        return;

    while (GetMenuItemCount(hMenu) > 0)
        RemoveMenu(hMenu, 0, MF_BYPOSITION);

    HMENU hDefinicionesMenu = CreatePopupMenu();
    AppendMenuW(hDefinicionesMenu, MF_STRING, IDM_TABLA_SIMBOLOS, L"&Analisis Lexico");
    AppendMenuW(hDefinicionesMenu, MF_STRING, IDM_PANEL_NATURAL, L"&Analisis Semantico");
    AppendMenuW(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hDefinicionesMenu, L"&Definiciones");

    HMENU hPalabrasReservadasSubMenu = CreatePopupMenu();
    AppendMenuW(hPalabrasReservadasSubMenu, MF_STRING, IDM_CONFIG_DML, L"&DML");
    AppendMenuW(hPalabrasReservadasSubMenu, MF_STRING, IDM_CONFIG_DDL, L"&DDL");

    HMENU hConfiguracionesMenu = CreatePopupMenu();
    AppendMenuW(hConfiguracionesMenu, MF_STRING | MF_POPUP, (UINT_PTR)hPalabrasReservadasSubMenu, L"&Palabras Reservadas");
    AppendMenuW(hConfiguracionesMenu, MF_STRING, IDM_CONFIG_FILTROS_NL, L"&Filtros NL");
    AppendMenuW(hConfiguracionesMenu, MF_STRING, IDM_CONFIG_OPERADORES_NL, L"&Operadores NL");
    AppendMenuW(hConfiguracionesMenu, MF_STRING, IDM_CONFIG_ESQUEMA_BASE, L"&Esquema Base");
    AppendMenuW(hConfiguracionesMenu, MF_STRING, IDM_CONFIG_RELACIONES, L"&Relaciones");

    AppendMenuW(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hConfiguracionesMenu, L"&Configuraciones");

    AppendMenuW(hMenu, MF_STRING, IDM_INSTRUCCIONES, L"&Instrucciones de uso");
    AppendMenuW(hMenu, MF_STRING, IDM_EXIT, L"&Salir");

    DrawMenuBar(hWnd);
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
