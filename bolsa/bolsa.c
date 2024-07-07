
#include "untils.h"

int _tmain(int argc, LPTSTR argv[]) {

    //-- Vaiaveis Registry --
    TCHAR chave_dir[TAM] = TEXT("Software\\TP_SO2\\");
    HKEY chave;
    DWORD lResult;
    TCHAR k_nome[TAM] = TEXT("NCLIENTES");    //nome da chave
    TCHAR k_valor[TAM] = TEXT("5");   //valor da chave por default
    int nMaxCli = 0;
    //---  Vaiaveis Arquivos
    Clientes Cli[N]; // estrutura para guardar clientes
    FILE* arquivo; 
    FILE* arquivo2;
    int TotalDeCli = 0; // contar n de clientes lidos do ficheiro de texto
    int TotalDeEmp = 0;

    HANDLE hPipe, hThread_TrataCLI, hEvent_WRITE;
    
    HANDLE hFileMap, hEventMemeS, hMutexMeme,hThreadMeme, hSemClientes;
    BOOL  fConnected = FALSE;
    DWORD dwThreadId;
    ThreadCLI T_dadosC[N];
    int k = 0;

    empresas Todas_Empresas[N];
  
    TDados tdados;
    int flag_end = 0;
    HANDLE  hThread_Teclado, hEventFlag;

    TmemoriaPartilhada dadosParaMeme;
    DWORD tamanho;
    int flag_PODERcomprar = 0;
    HANDLE hEventPODERcomprar;
    Ttime PTimeThread;
    int segundos = 0;
    HANDLE  hTimeThread;
    int MaxTop;
    TCHAR frase[200];
  


#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif

    //------------------- PARTE DO REGISTERY -------------------

    MaxTop = _tstoi(k_valor);

    if (argc != 3) {
        _tprintf(TEXT("Parametros de inicializacao mal defenido <./Nome_do_programa numero ficheiro>\n"));
        return 1;
    }

    MaxTop = _tstoi(argv[1]);

    if (MaxTop < 1 || MaxTop > 10) {
        _tprintf(TEXT("Numero de empresas no top mal defenido\n"));
        return 1;
    }

    if (RegCreateKeyEx(
        HKEY_CURRENT_USER,
        chave_dir,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        NULL,
        &chave,
        &lResult
    ) != ERROR_SUCCESS) {
        _tprintf(TEXT("Chave nao foi nem criada nem aberta! ERRO!"));
        return 1;
    }
    if (lResult == REG_CREATED_NEW_KEY) {

        if (RegSetValueEx(
            chave,
            k_nome,
            0,
            REG_SZ,
            (LPCBYTE)&k_valor,
            sizeof(TCHAR) * (_tcslen(k_valor) + 1)

        ) != ERROR_SUCCESS)
            _tprintf(TEXT("O atributo nao foi alterado nem criado! ERRO! %s"), k_nome);
        return 1;
    }

    else {

        k_valor[0] = '\0';
         tamanho = sizeof(k_valor);
        if (RegQueryValueEx(
            chave,
            k_nome,
            0,
            NULL,
            (LPBYTE)k_valor,
            &tamanho
        ) != ERROR_SUCCESS)
            _tprintf(TEXT("Atributo [%s] nao foi encontrado!\n"), k_nome);
        else
            _tprintf(TEXT("\nAtributo encontrado com o valor : %s \n"), k_valor);

    }

    RegCloseKey(chave);

    //------------------- LER CLIENTES DO FICHEIRO -------------------
    for (size_t i = 0; i < N; i++)
        Cli[i].preenchido = FALSE;

    errno_t err = _tfopen_s(&arquivo, argv[2], TEXT("r")); // abrir ficheiro dos clientes
    if (err != 0){
        _tprintf(TEXT("Erro ao abrir o arquivo.\n"));
        return 1;
    }

    nMaxCli = _tstoi(k_valor);

    if (nMaxCli <= 0) {
        _tprintf(TEXT("Numero max de clientes defenido com um valor invalido"));
        return 1;
    }

    while (TotalDeCli < nMaxCli &&
        _ftscanf_s(arquivo, TEXT("%49s %49s %f"),
            Cli[TotalDeCli].nome, TAM_V,
            Cli[TotalDeCli].psw, TAM_V,
            &Cli[TotalDeCli].saldo) == 3) { // ler os dados dos clientes do ficheiro
        Cli[TotalDeCli].preenchido = TRUE;
        Cli[TotalDeCli].logado = FALSE;
        Cli[TotalDeCli].n_acoesCompradas = 0;
        Cli[TotalDeCli].ID = TotalDeCli;
        for (int i = 0; i < 5; i++)
        {
            Cli[TotalDeCli].emp[i].preenchido = FALSE;
        }
        TotalDeCli++;
    }

    _tprintf(TEXT("Clientes lidos do arquivo:\n"));
    for (int j = 0; j < TotalDeCli; j++) {
        _tprintf(TEXT("Nome: %s, Password: %s, Saldo: %.2f\n"), Cli[j].nome, Cli[j].psw, Cli[j].saldo);
    }

    fclose(arquivo);

    //------------------- LER EMPRESAS DO FICHEIRO -------------------

    for (int i = 0; i < N; i++)
        Todas_Empresas[i].preenchido = FALSE;
    

    errno_t err2 = _tfopen_s(&arquivo2, TEXT("empresas.txt"), TEXT("r")); // abrir ficheiro dos clientes
    if (err2 != 0) {
        _tprintf(TEXT("Erro ao abrir o arquivo.\n"));
        return 1;
    }

    while (_ftscanf_s(arquivo2, TEXT("%49s %d %f"), Todas_Empresas[TotalDeEmp].nome, TAM_V, &Todas_Empresas[TotalDeEmp].numDeAcoes, &Todas_Empresas[TotalDeEmp].price) == 3 ) { // ler os dados dos clientes do ficheiro
        Todas_Empresas[TotalDeEmp].preenchido = TRUE;
        TotalDeEmp++;
    }


    _tprintf(TEXT("Empresas lidas do arquivo:\n"));
    for (int j = 0; j < TotalDeEmp; j++) {
        _tprintf(TEXT("Nome: %s, num de acoes: %d, Preco: %.2f\n"), Todas_Empresas[j].nome, Todas_Empresas[j].numDeAcoes, Todas_Empresas[j].price);
    }

    fclose(arquivo2);

    _tprintf(TEXT("\n\n--------- PROGRAMA BOLSA -----------\n"));

    //------ Mutex empresas ------------
    hMutexMeme = CreateMutex(NULL, FALSE, MUTEX_MEME_NAME);

    if (hMutexMeme == NULL) {
        _tprintf(TEXT("Erro ao criar mutex da memoria partilhada\n"));
        CloseHandle(hMutexMeme);
        return 1;
    }

    hSemClientes = CreateSemaphore(NULL, nMaxCli, nMaxCli, TEXT("SEMAFORO_NUM_CLI"));

    //------------------- Thread para o teclado -------------------
    hEventFlag = CreateEvent(NULL, FALSE, FALSE, NULL); // evento para avisar do fim
    hEventPODERcomprar = CreateEvent(NULL, FALSE, FALSE, NULL);  // evento para comprar

    tdados.flagEvent = hEventFlag; // evento para avisar do fim
    tdados.flag = &flag_end; // flag do fim
    tdados.hPipe_emEspera[0] = &hPipe; // handle do pipe que esta a espera do cliente se conectar
    tdados.copy_arrayEmp = Todas_Empresas;
    tdados.MUTEX = hMutexMeme;
    tdados.array_cli = Cli; 
    tdados.flag_poderComprar = &flag_PODERcomprar;
    tdados.hEventPoderC = hEventPODERcomprar;
    tdados.valoremSeg = &segundos;

    hThread_Teclado = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)teclado, &tdados, 0, NULL);

    //------------------- Thread para a memoria partilhada -------------------

    hEventMemeS = CreateEvent(NULL, TRUE, FALSE, NAME_EvMeme); 

    if (hEventMemeS == NULL) {
        _tprintf(TEXT("Erro ao criar evento da memoria partilhada\n"));
        CloseHandle(hEventMemeS);
        return 1;
    }

    hFileMap = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        sizeof(DadosPartilha)*5,
        SHM_NAME);

    if (hFileMap == NULL) {
        _tprintf(TEXT("Erro ao criar memoria partilhada\n"));
        CloseHandle(hEventMemeS);
        CloseHandle(hMutexMeme);
        return 1;
    }
    
    dadosParaMeme.pMapeamento = (DadosPartilha*)MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);

    if (dadosParaMeme.pMapeamento == NULL) {
        _tprintf(TEXT("Erro ao mapear memoria partilhada\n"));
        CloseHandle(hEventMemeS);
        CloseHandle(hMutexMeme);
        CloseHandle(hFileMap);
        return 1;
    }

    dadosParaMeme.flag = &flag_end;
    dadosParaMeme.hMutex = hMutexMeme;
    dadosParaMeme.hEventEnd = hEventFlag;
    dadosParaMeme.hEventoMem = hEventMemeS;
    dadosParaMeme.pEmpresas = Todas_Empresas;
    dadosParaMeme.fraseUltimaT = frase;
    dadosParaMeme.estrutura.num = MaxTop;

    hThreadMeme = CreateThread(NULL, 0, TshareMeme, &dadosParaMeme, 0, NULL);
    
    //------------------- Thread do timer de vendas suspensas  -------------------

    PTimeThread.flag_poderComprar = &flag_PODERcomprar;
    PTimeThread.hEventPoderC = hEventPODERcomprar;
    PTimeThread.valoremSeg = &segundos;

    hTimeThread = CreateThread(NULL, 0, ThresperaCompra, &PTimeThread, 0, NULL);
    //------------------- PARTE DO NAMED PIPE PARA TRATAR DOS CLIENTES -------------------

    hEvent_WRITE = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (hEvent_WRITE == NULL) {
        _tprintf(TEXT("[ERRO] ao criar evento write na main\n"));
        return 1;
    }

    while (flag_end == 0) {
       
        hPipe = CreateNamedPipe( // Named PIPE para os clientes 
            PIPE_NAME, 
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            sizeof(Clientes)*2,
            sizeof(Clientes)*2,
            4000,
            NULL);

        if (hPipe == INVALID_HANDLE_VALUE)
        {
            _tprintf(TEXT("[ERRO] Criar Named Pipe! (erro %d)\n"), GetLastError());
            return 1;
        }
        
        hEvent_WRITE = CreateEvent(NULL, TRUE, FALSE, NULL);
        T_dadosC[k].todos_clis = Cli;
        T_dadosC[k].array_emp = Todas_Empresas;
        T_dadosC[k].hEvent = hEvent_WRITE;
        T_dadosC[k].hPipe = hPipe;
        T_dadosC[k].hMutex = hMutexMeme;
        T_dadosC[k].flag = &flag_end;
        T_dadosC[k].flagEvent = hEventFlag;// evento que avisa o fim
        T_dadosC[k].flag_poderComprar = &flag_PODERcomprar;
       T_dadosC[k].fraseUltimaT = frase;
        
        fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
      
        if (fConnected) {

            hThread_TrataCLI = CreateThread(
                NULL,
                0,
                Th_TrataCLI,
                (LPVOID)&T_dadosC[k],
                0,
                &dwThreadId
            );

            if (hThread_TrataCLI == NULL){
                        _tprintf(TEXT("[ERRO] Criar Named Pipe! (erro %d)\n"), GetLastError());
                        return 1;
            }
        }
        k++;
        
    }
   
    for (size_t i = 0; i < k; i++)
    {
        FlushFileBuffers(T_dadosC[i].hPipe);

        _tprintf(TEXT("<ESCRITOR> Disconnecting  named pipe...\n"));

        if (!DisconnectNamedPipe(T_dadosC[i].hPipe)) {
            _tprintf(TEXT("[ERROR] Failed to disconnect named pipe (%d)\n"), GetLastError());
            CloseHandle(T_dadosC[i].hPipe);
            return -1;
        }
        CloseHandle(T_dadosC[i].hPipe);
    }

    CloseHandle(hThread_Teclado);
    CloseHandle(hEvent_WRITE);
    CloseHandle(hEventFlag);
    CloseHandle(hFileMap);
    CloseHandle(hEventMemeS);
    CloseHandle(hMutexMeme);
    CloseHandle(hThreadMeme);
    CloseHandle(hEventPODERcomprar);
    UnmapViewOfFile(dadosParaMeme.pMapeamento);
    CloseHandle(hTimeThread);
    CloseHandle(hSemClientes);

    return 0;
}