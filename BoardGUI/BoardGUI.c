#include <windows.h>
#include <tchar.h>
#include <Windowsx.h>
#include "resource.h"





LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI AtualizaDados(LPVOID param);


TCHAR szProgName[] = TEXT("Base");

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPTSTR lpCmdLine, int nCmdShow) {
    HWND hWnd;
    MSG lpMsg;
    WNDCLASSEX wcApp;
    COLORREF bgColor = RGB(255, 192, 203);

    wcApp.cbSize = sizeof(WNDCLASSEX);
    wcApp.hInstance = hInst;

    wcApp.lpszClassName = szProgName;
    wcApp.lpfnWndProc = TrataEventos;

    wcApp.style = CS_HREDRAW | CS_VREDRAW;
    wcApp.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
    wcApp.hIconSm = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
    wcApp.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcApp.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);


    wcApp.cbClsExtra = 0;
    wcApp.cbWndExtra = 0;
    wcApp.hbrBackground = CreateSolidBrush(bgColor);

    if (!RegisterClassEx(&wcApp))
        return(0);

    hWnd = CreateWindow(
        szProgName,
        TEXT("BoardGUI"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        900,
        500,
        (HWND)HWND_DESKTOP,
        (HMENU)NULL,
        (HINSTANCE)hInst,
        0);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    //------ Codigo board ------

    HANDLE hEventMemeS = OpenEvent(EVENT_ALL_ACCESS, FALSE, NAME_EvMeme);
    HANDLE hMutexMeme = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEX_MEME_NAME);
    HANDLE hFileMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, SHM_NAME);

    if (hFileMap == NULL) {
        MessageBox(hWnd, TEXT("Erro ao abrir memoria partilhada"), TEXT("Erro"), MB_OK);
        CloseHandle(hEventMemeS);
        CloseHandle(hMutexMeme);
        return 0;
    }

    DadosPartilha* MemP = (DadosPartilha*)MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);

    static ThreadParams params;
    params.hWnd = hWnd;
    params.hEvent = hEventMemeS;
    params.hMutex = hMutexMeme;
    params.hFileMap = hFileMap;
    params.MemP = MemP;


    CreateThread(NULL, 0, AtualizaDados, (LPVOID)&params, 0, NULL);

    while (GetMessage(&lpMsg, NULL, 0, 0) > 0) {
        TranslateMessage(&lpMsg);
        DispatchMessage(&lpMsg);
    }

    UnmapViewOfFile(MemP);
    CloseHandle(hFileMap);
    CloseHandle(hEventMemeS);
    CloseHandle(hMutexMeme);

    return (int)lpMsg.wParam;
}

DWORD WINAPI AtualizaDados(LPVOID param) {
    ThreadParams* params = (ThreadParams*)param;
    while (1) {
        WaitForSingleObject(params->hEvent, INFINITE);
        WaitForSingleObject(params->hMutex, INFINITE);

        DadosPartilha estrut;
        CopyMemory(&estrut, params->MemP, sizeof(DadosPartilha));

        ReleaseMutex(params->hMutex);
        ResetEvent(params->hEvent);

       
        SendMessage(params->hWnd, WM_USER + 1, (WPARAM)&estrut, 0);
    }
    return 0;
}

void MostraTopEmpresas(HWND hWnd, DadosPartilha* estrut) {
    TCHAR buffer[1024] = TEXT("TOP das empresas mais caras são:\n");
    for (int i = 0; i < estrut->num && estrut->Top_emp[i].preenchido == TRUE; i++) {
        _stprintf_s(buffer + _tcslen(buffer), sizeof(buffer) / sizeof(TCHAR) - _tcslen(buffer),
            TEXT("%d - empresas %s com acoes %d de preco %.2f\n"),
            i + 1, estrut->Top_emp[i].nome, estrut->Top_emp[i].numDeAcoes, estrut->Top_emp[i].price);
    }
    MessageBox(hWnd, buffer, TEXT("Top Empresas"), MB_OK);
}

void MostraUltimaTransacao(HWND hWnd, DadosPartilha* estrut) {
    MessageBox(hWnd, estrut->fraseUltimaT, TEXT("Última Transação"), MB_OK);
}

LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
    static DadosPartilha dados;
    switch (messg) {
    case WM_CREATE:
        EnableMenuItem(GetMenu(hWnd), ID_TOPEMPRESAS, MF_ENABLED);
        EnableMenuItem(GetMenu(hWnd), ID_ULTIMATRANSACAO, MF_ENABLED);
        EnableMenuItem(GetMenu(hWnd), ID_SAIR, MF_ENABLED);
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_TOPEMPRESAS:
            MostraTopEmpresas(hWnd, &dados);
            break;
        case ID_ULTIMATRANSACAO:
            MostraUltimaTransacao(hWnd, &dados);
            break;
        case ID_SAIR:
            DestroyWindow(hWnd);
            break;
        }
        break;
    case WM_USER + 1:
        CopyMemory(&dados, (DadosPartilha*)wParam, sizeof(DadosPartilha));  
        break;

    case WM_CLOSE:
        
        if (MessageBox(hWnd, TEXT("Quer mesmo sair?"), TEXT("Confirmação"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
       
            DestroyWindow(hWnd);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return(DefWindowProc(hWnd, messg, wParam, lParam));
        break;
    }
    return 0;
}