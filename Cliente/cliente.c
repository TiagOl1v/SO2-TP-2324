#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <windows.h>

#define PIPE_NAME TEXT("\\\\.\\pipe\\PipeCLIENTES")

#define TAM_V 50
#define TAM 200
#define N 30

typedef struct {

    TCHAR nome[TAM_V];
    int numDeAcoes;
    float price;
    BOOL preenchido;

}empresas;


typedef struct StructParaCli {

    TCHAR nome[TAM_V];
    TCHAR psw[TAM_V];
    int ID;
    TCHAR comando[TAM];
    float saldo;
    empresas emp[5];
    BOOL preenchido;
    BOOL logado;
    int resultado;
    int n_acoesCompradas;

}Clientes;



typedef struct {

    Clientes* pC;
    HANDLE hPipe;

}Tdados;


DWORD WINAPI leitura(LPVOID lpParam) {

    Tdados* td = (Tdados*)lpParam;

    DWORD cbBytesWritten = 0;
    BOOL  fSucess = FALSE;
    HANDLE Hwrite;
    OVERLAPPED OverWr = { 0 };

    Hwrite = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (Hwrite == NULL) {
        _tprintf(TEXT("[ERRO] ao criar evento write\n"));
        return 1;
    }
    td->pC->resultado = 0;
    while (td->pC->resultado != 99)
    {

        _fgetts(td->pC->comando, sizeof(td->pC->comando), stdin);

        ZeroMemory(&OverWr, sizeof(OverWr));
        ResetEvent(Hwrite);
        OverWr.hEvent = Hwrite;

        fSucess = WriteFile(td->hPipe, td->pC->comando, sizeof(td->pC->comando), &cbBytesWritten, &OverWr);

        if (!fSucess) {
            _tprintf(TEXT("[ERRO] Falha na escrita do pipe (erro %d)\n"), GetLastError());
            break;
        }

        WaitForSingleObject(Hwrite, INFINITE);


    }
}

int _tmain(int argc, LPTSTR argv[]) {

    DWORD cbBytesRead = 0;
    HANDLE hPipe;
    Clientes C;
    DWORD dwMode;
    BOOL  fSucess = FALSE;
    HANDLE hThread, hSemClientes;
    TCHAR com[50], aux[50];
    Tdados Tcli;
    OVERLAPPED OverRd = { 0 };
    HANDLE hEvent_Read;

#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif
    _tprintf(TEXT("--------- PROGRAMA CLIENTE ------------ \n"));
    _tprintf(TEXT("Aguarde ate estar conectado a BOLSA... \n"));
    C.logado = FALSE;

    hPipe = CreateFile(
        PIPE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        0 | FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0 | FILE_FLAG_OVERLAPPED,
        NULL);

    if (hPipe == INVALID_HANDLE_VALUE)
    {
        _tprintf(TEXT("[ERRO] Ao abrir o named PIPE! (erro %d)\n"), GetLastError());
        return 1;
    }

    hEvent_Read = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (hEvent_Read == NULL) {
        _tprintf(TEXT("[ERRO] ao criar evento read\n"));
        return 1;
    }

    hSemClientes = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, TEXT("SEMAFORO_NUM_CLI"));

    WaitForSingleObject(hSemClientes, INFINITE);
    Tcli.pC = &C;
    Tcli.hPipe = hPipe;

    hThread = CreateThread(
        NULL,
        0,
        leitura,
        (LPVOID)&Tcli,
        0,
        NULL
    );

    dwMode = PIPE_READMODE_MESSAGE;

    fSucess = SetNamedPipeHandleState(hPipe, &dwMode, NULL, NULL);

    if (!fSucess)
    {
        _tprintf(TEXT("[ERRO]  Erro a tentar mudar para o modo PIPE_READMODE_MESSAGE (erro %d)\n"), GetLastError());
        return 1;
    }

    _tprintf(TEXT("\nCONECTADO\n"));
    _tprintf(TEXT("Use login <nome> <passowrd> || exit para sair \n"));
    while (C.resultado != 99)
    {


        if (C.logado == TRUE)
            _tprintf(_T("\n%s > "), C.nome);

        // Ler do pipe
        ZeroMemory(&OverRd, sizeof(OverRd));
        OverRd.hEvent = hEvent_Read;
        ResetEvent(hEvent_Read);

        fSucess = ReadFile(hPipe, &C, sizeof(Clientes), &cbBytesRead, &OverRd);


        // Aguardar a operação de leitura terminar
        WaitForSingleObject(hEvent_Read, INFINITE);

        if (!fSucess && GetLastError() != ERROR_IO_PENDING) {
            _tprintf(TEXT("[ERRO] Falha na leitura do pipe (erro %d)\n"), GetLastError());
            break; // Encerrar o loop se ocorrer um erro na leitura
        }

        // Obter o resultado da leitura assíncrona
        if (!GetOverlappedResult(hPipe, &OverRd, &cbBytesRead, FALSE)) {
            _tprintf(TEXT("[ERRO] Falha ao obter resultado da leitura (erro %d)\n"), GetLastError());
            break;
        }

        if (C.resultado != 90) {
            _stscanf_s(C.comando, _T("%s %s"), com, (unsigned)_countof(com), aux, (unsigned)_countof(aux));

            if (_tcscmp(com, TEXT("buy")) == 0) {
                if (C.resultado == 1)
                    _tprintf(TEXT("Compra efetuada com sucesso\n"));
                else if (C.resultado == 2)
                    _tprintf(TEXT("A empresa nao tem esse numero de acoes disponivel para compra\n"));
                else if (C.resultado == 3)
                    _tprintf(TEXT("Saldo insuficiente para realizar a acao\n"));
                else if (C.resultado == 4)
                    _tprintf(TEXT("Ja tem o maximo de acoes possiveis \n"));
                else if (C.resultado == 10)
                    _tprintf(TEXT(" Compra de acoes suspensa \n"));
            }
            if (_tcscmp(com, TEXT("login")) == 0) {

                if (C.resultado == 0)
                    _tprintf(TEXT("ENTROU\n"));
                else if (C.resultado == 1)
                    _tprintf(TEXT("Credenciais erradas\n"));
                else if (C.resultado == 2)
                    _tprintf(TEXT("Ja existe alguem logado com esse user\n"));

            }
            else if (_tcscmp(com, TEXT("sell")) == 0) {
                if (C.resultado == 1)
                    _tprintf(TEXT("Venda efetuada com sucesso\n"));
                else if (C.resultado == 2)
                    _tprintf(TEXT("A voce nao tem esse numero de acoes necessarios para vender \n"));
                else
                    _tprintf(TEXT("nao possui essa acao\n"));
            }
            else if (_tcscmp(com, TEXT("listc")) == 0) {
                _tprintf(TEXT("As suas empresas:"));
                for (int i = 0; i < 5; i++)
                    if (C.emp[i].preenchido == TRUE)
                        _tprintf(TEXT("Nome: %s, num de acoes: %d, Preco: %.2f\n"), C.emp[i].nome, C.emp[i].numDeAcoes, C.emp[i].price);
            }
            else if (_tcscmp(com, TEXT("balance")) == 0)
                _tprintf(TEXT("Saldo atual de %.2f\n"), C.saldo);

            else if (_tcscmp(com, TEXT("exit")) == 0)
                break;


        }
        else
            _tprintf(TEXT("Comando nao existe \n"));

    }
    ReleaseSemaphore(hSemClientes, 1, NULL);
    CloseHandle(hSemClientes);
    TerminateThread(hThread, 0);


    _tprintf(TEXT("Saiste"));
}