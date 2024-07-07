#include "untils.h"


int verficaLoggin(TCHAR* nome, TCHAR* pass, Clientes* ARRAY_CLI, Clientes* C) {

    int i = 0, flag = 1;


    while (ARRAY_CLI[i].preenchido == TRUE && flag == 1) {

        if (_tcscmp(nome, ARRAY_CLI[i].nome) == 0)
            if (_tcscmp(pass, ARRAY_CLI[i].psw) == 0)
                if (ARRAY_CLI[i].logado == FALSE) {
                    flag = 0;
                    C->ID = ARRAY_CLI[i].ID;
                    ARRAY_CLI[i].logado = TRUE;
                    break;
                }
                else
                    flag = 2;
        i++;
    }

    if (flag == 0) {
        C->logado = TRUE;
        _tcscpy_s(C->nome, TAM_V, nome);
        _tcscpy_s(C->psw, TAM_V, pass);
    }

    return flag;


}

float muda_price(float price) {

    srand(time(0));

    int randomV = rand();


    if (randomV % 2 == 0) {

        if (price > 0.01)
            price = price - (price * 0.05);

    }
    else {

        price = price + (price * 0.05);
        
    }


    return price;

}

int ComprarEmp(empresas* ARRAY_EMP, Clientes* C, Clientes* ARRAY_CLI, TCHAR* nome_emp, int num_acoes, HANDLE mutex, TCHAR* frase) {
    int flag = 0, flag_podeComprar = 0;
    int j = 0, i = 0, k = 0;

    // Encontrar cliente atual
    for (j = 0; j < N; j++) {
        if (C->ID == ARRAY_CLI[j].ID)
            break;
    }

    for (k = 0; k < 5; k++) {//verifica se ja foi comprada 
        if (_tcscmp(ARRAY_CLI[j].emp[k].nome, nome_emp) == 0 && ARRAY_CLI[j].emp[k].preenchido) {
            flag_podeComprar = 2;
            break;
        }
        else {
            flag_podeComprar = 1;
        }
    }
    WaitForSingleObject(mutex, INFINITE);
    if (flag_podeComprar == 2) { // acao ja foi comprada
        for (i = 0; i < N; i++) {
            if ((num_acoes * ARRAY_EMP[i].price) <= ARRAY_CLI[j].saldo && (_tcscmp(nome_emp, ARRAY_EMP[i].nome) == 0)) {
                ARRAY_CLI[j].emp[k].numDeAcoes += num_acoes;
                ARRAY_EMP[i].numDeAcoes -= num_acoes;
                ARRAY_CLI[j].saldo -= (num_acoes * ARRAY_EMP[i].price);
                ARRAY_EMP[i].price = muda_price(ARRAY_EMP[i].price);
                _stprintf_s(frase, 199, _TEXT("O user %s comprou %d acoes da %s por %.2f"), C->nome, num_acoes, nome_emp, ARRAY_EMP[i].price);
                flag = 1;
                break;
            }
            else {
                flag = 3; // Sem saldo suficiente
            }
        }
    }
    else if (flag_podeComprar == 1 && ARRAY_CLI[j].n_acoesCompradas < 5) {
        for (i = 0; i < N; i++) {
            if (_tcscmp(nome_emp, ARRAY_EMP[i].nome) == 0) {

                if (num_acoes <= ARRAY_EMP[i].numDeAcoes) {

                    if ((num_acoes * ARRAY_EMP[i].price) <= ARRAY_CLI[j].saldo) {
                        for (k = 0; k < 5; k++) {
                            if (!ARRAY_CLI[j].emp[k].preenchido) {
                                _tcscpy_s(ARRAY_CLI[j].emp[k].nome, TAM_V, nome_emp);
                                ARRAY_CLI[j].emp[k].numDeAcoes = num_acoes;
                                ARRAY_EMP[i].numDeAcoes -= num_acoes;
                                ARRAY_CLI[j].emp[k].preenchido = TRUE;
                                ARRAY_CLI[j].saldo -= (num_acoes * ARRAY_EMP[i].price);
                                ARRAY_EMP[i].price = muda_price(ARRAY_EMP[i].price);
                                _stprintf_s(frase, 199, _TEXT("O user %s comprou %d acoes da %s por %.2f"), C->nome, num_acoes, nome_emp, ARRAY_EMP[i].price);
                                ARRAY_CLI[j].n_acoesCompradas++;
                                _tprintf(TEXT("%d \n"), ARRAY_CLI[j].n_acoesCompradas);
                                flag = 1; // Ações compradas
                                break;
                            }
                        }
                    }
                    else {
                        flag = 3; // Sem saldo suficiente
                        break;
                    }
                }
                else {
                    flag = 2; // Número de ações disponíveis é inferior
                }
                break;
            }
            else {
                flag = 0; // Ação não existe
            }
        }
    }
    else {
        flag = 4; // Máximo de empresas compradas
    }
    ReleaseMutex(mutex);
    
    return flag;
}


int venderEmp(empresas* ARRAY_EMP, Clientes* C, Clientes* ARRAY_CLI, TCHAR* nome_emp, int num_acoes, HANDLE mutex, TCHAR* frase) {

    int flag = 0;
    Clientes aux;



    for (int i = 0; i < N && flag == 0; i++)
    {
        if (C->ID == ARRAY_CLI[i].ID) {
            if (ARRAY_CLI[i].n_acoesCompradas == 0)
                return 3;
            WaitForSingleObject(mutex, INFINITE);
            for (int j = 0; j < 5 && flag == 0; j++)
            {
                if (_tcscmp(ARRAY_CLI[i].emp[j].nome, nome_emp) == 0 && ARRAY_CLI[i].emp[j].preenchido == TRUE) {
                    if (ARRAY_CLI[i].emp[j].numDeAcoes >= num_acoes)
                    {

                        for (int k = 0; k < N; k++)
                            if (_tcscmp(ARRAY_EMP[k].nome, nome_emp) == 0) {
                                ARRAY_EMP[k].numDeAcoes += num_acoes;
                                ARRAY_CLI[i].emp[j].numDeAcoes -= num_acoes;
                                ARRAY_CLI[i].saldo += (num_acoes * ARRAY_EMP[k].price);
                                ARRAY_EMP[k].price = muda_price(ARRAY_EMP[k].price);
                                _stprintf_s(frase, 199, _TEXT("O user %s vendeu %d acoes da %s por %.2f"), C->nome, num_acoes, nome_emp, ARRAY_EMP[k].price);

                            }

                        if ((ARRAY_CLI[i].emp[j].numDeAcoes) == 0) {//caso nao possua mais acoes
                            ARRAY_CLI[i].emp[j].preenchido = FALSE;
                            ARRAY_CLI[i].n_acoesCompradas;
                        }
                        flag = 1;

                    }
                    else
                        flag = 2;
                }


            }
            break;
        }
    }

    ReleaseMutex(mutex);
    return flag;
}


DWORD WINAPI teclado(LPVOID lpParam) {
    TDados* td = (TDados*)lpParam;
    TCHAR cmd[100];
    TCHAR pri[30], para1[30], para2[30];
    int i = 0;
    float  numeroFeI, para3;
    int numSeg;
    int flag_comand;

    do {
        flag_comand = 0;
        para3 = 0;

        _fgetts(cmd, _countof(cmd), stdin); // Ler a linha inteira como uma string


        _stscanf_s(cmd, TEXT("%29s %29s %29s %f"), pri, (unsigned)_countof(pri), para1, (unsigned)_countof(para1), para2, (unsigned)_countof(para2), &para3);

        numSeg = _tstoi(para1);
        numeroFeI = _tstof(para2);

        if (_tcscmp(pri, TEXT("listc")) == 0) {
            _tprintf(TEXT("Empresas existentes:\n"));
            WaitForSingleObject(td->MUTEX, INFINITE);
            for (int j = 0; j < N && td->copy_arrayEmp[j].preenchido == TRUE; j++) {
                _tprintf(TEXT("Empresa: %s, num de acoes: %d, Preco: %.2f\n"),
                    td->copy_arrayEmp[j].nome,
                    td->copy_arrayEmp[j].numDeAcoes,
                    td->copy_arrayEmp[j].price);
            }
            ReleaseMutex(td->MUTEX);
            flag_comand = 1;
        }
        else if (_tcscmp(pri, TEXT("users")) == 0) {
            _tprintf(TEXT("Clientes registados no sistema:\n"));
            for (int j = 0; j < N && td->array_cli[j].preenchido == TRUE; j++)
            {
                if (td->array_cli[j].logado == TRUE)
                    _tprintf(TEXT("[ONLINE] Nome: %s  Saldo: %.2f\n"), td->array_cli[j].nome, td->array_cli[j].saldo);
                else
                    _tprintf(TEXT(" [OFFLINE]  Nome: %s  Saldo: %.2f\n"), td->array_cli[j].nome, td->array_cli[j].saldo);

            }

            flag_comand = 1;
        }

        else if (_tcscmp(pri, TEXT("pause")) == 0 && numSeg > 0) {
            if (*(td->flag_poderComprar) == 0) {
                *(td->valoremSeg) = numSeg * 1000;
                _tprintf(TEXT("timer defenido para %d segundos\n"), numSeg);
                SetEvent(td->hEventPoderC);
            }
            else
                _tprintf(TEXT("pause ja foi defenido, esperar acabar\n"));
            flag_comand = 1;
        }

        else if (_tcscmp(pri, TEXT("addc")) == 0 && numeroFeI > 0 && para3 > 0.10) {

            WaitForSingleObject(td->MUTEX, INFINITE);
            for (int j = 0; j < N; j++)
                if (td->copy_arrayEmp[j].preenchido == FALSE) {
                    td->copy_arrayEmp[j].numDeAcoes = numeroFeI;
                    td->copy_arrayEmp[j].preenchido = TRUE;
                    td->copy_arrayEmp[j].price = para3;
                    _tcscpy_s(td->copy_arrayEmp[j].nome, 30, para1);
                    flag_comand = 1;
                    break;
                }
            ReleaseMutex(td->MUTEX);
        }

        else if (_tcscmp(pri, TEXT("stock")) == 0 && numeroFeI > 0) {
            _tprintf(TEXT("aqui\n"));
            WaitForSingleObject(td->MUTEX, INFINITE);
            for (int j = 0; j < N; j++)
                if (_tcscmp(para1, td->copy_arrayEmp[j].nome) == 0) {
                    td->copy_arrayEmp[j].price = numeroFeI;
                    flag_comand = 1;
                }
            ReleaseMutex(td->MUTEX);
        }
        if (flag_comand == 1)
            _tprintf(TEXT("Operacao realizada com sucesso\n"));
        else
            _tprintf(TEXT("Nao foi possivel realizar a operacao, verifique o comando\n"));

    } while (_tcscmp(pri, TEXT("close")) != 0);

    *(td->flag) = 1;
    while (i <= 5) {
        SetEvent(td->flagEvent);
        i++;
    }
    CloseHandle(*(td->hPipe_emEspera[0]));

}


DWORD WINAPI Th_TrataCLI(LPVOID lpParam) {
    ThreadCLI* arg = (ThreadCLI*)lpParam;
    DWORD cbBytesRead = 0, cbBytesWritten = 0;
    HANDLE hEvent_Read;
    Clientes Cli_atual;
    BOOL  fSucess = FALSE;
    TCHAR  para1[30], para2[50], comando[200];
    OVERLAPPED OverRd = { 0 }, OverWr = { 0 };
    int int_param = 0;


    if (arg->hPipe == NULL) {

        _tprintf(TEXT("ERRO no handle enviado para a thread.\n"));
        return 1;
    }

    hEvent_Read = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (hEvent_Read == NULL) {
        _tprintf(TEXT("[ERRO] ao criar evento\n"));
        return 1;
    }
    HANDLE hEvents[2] = { arg->flagEvent, hEvent_Read };
    //adicionar cli 
    Cli_atual.logado = FALSE;
    Cli_atual.resultado = 90;

    while (Cli_atual.resultado != 99 && *(arg->flag) == 0) {
        Cli_atual.resultado = 90;
        // Reinicializar a estrutura OVERLAPPED
        ZeroMemory(&OverRd, sizeof(OverRd));
        ResetEvent(hEvent_Read);
        OverRd.hEvent = hEvent_Read;

        fSucess = ReadFile(arg->hPipe, &Cli_atual.comando, sizeof(Cli_atual.comando), &cbBytesRead, &OverRd);

        if (!fSucess && GetLastError() != ERROR_IO_PENDING) {
            _tprintf(TEXT("[ERRO] Falha na leitura do pipe (erro %d)\n"), GetLastError());
            break; // Encerrar o loop se ocorrer um erro na leitura
        }

        DWORD dwWait = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);

        if (dwWait == WAIT_OBJECT_0) {
            Cli_atual.resultado = 99;

        }
        if (!GetOverlappedResult(arg->hPipe, &OverRd, &cbBytesRead, FALSE) && *(arg->flag) == 0) {
            _tprintf(TEXT("[ERRO] Falha ao obter resultado da leitura (erro %d)\n"), GetLastError());
            break; // Encerrar o loop se não for possível obter o resultado
        }
        size_t length = _tcsclen(Cli_atual.comando);

        Cli_atual.comando[length - 1] = '\0'; // Substituir a quebra de linha pelo terminador nulo

        //  _tprintf(TEXT("[LOG]Recebi o comando %s\n"), Cli_atual.comando);

          //-------------------- Parte do loggin ----------------
         //codigo de comando nao reconhecido

        if (Cli_atual.logado == FALSE) {

            if (_stscanf_s(Cli_atual.comando, _T("%s %s %s"),
                comando, sizeof(comando) / sizeof(comando[0]),
                para1, sizeof(para1) / sizeof(para1[0]),
                para2, sizeof(para2) / sizeof(para2[0])) == 3)

                if (_tcscmp(comando, TEXT("login")) == 0)

                    Cli_atual.resultado = verficaLoggin(para1, para2, arg->todos_clis, &Cli_atual);
        }
        else {
            _stscanf_s(Cli_atual.comando, _T("%s %s %s"),
                comando, sizeof(comando) / sizeof(comando[0]),
                para1, sizeof(para1) / sizeof(para1[0]),
                para2, sizeof(para2) / sizeof(para2[0]));

            int_param = _tstoi(para2);

            if (int_param > 0) {
                if (*(arg->flag_poderComprar) == 0) {
                    Cli_atual.resultado = 91;
                    if (_tcscmp(comando, TEXT("buy")) == 0)
                        Cli_atual.resultado = ComprarEmp(arg->array_emp, &Cli_atual, arg->todos_clis, para1, int_param, arg->hMutex, arg->fraseUltimaT);
                    else if (_tcscmp(comando, TEXT("sell")) == 0)
                        Cli_atual.resultado = venderEmp(arg->array_emp, &Cli_atual, arg->todos_clis, para1, int_param, arg->hMutex, arg->fraseUltimaT);
                }
                else
                    Cli_atual.resultado = 10;


            }

            if (_tcscmp(Cli_atual.comando, TEXT("listc")) == 0) {
                WaitForSingleObject(arg->hMutex, INFINITE);
                for (int j = 0; j < N && arg->todos_clis[j].preenchido == TRUE; j++)
                    if (Cli_atual.ID == arg->todos_clis[j].ID)
                        for (int i = 0; i < 5; i++) {
                            for (int k = 0; k < N && arg->array_emp[k].preenchido == TRUE; k++) {
                                if (_tcscmp(arg->todos_clis[j].emp[i].nome, arg->array_emp[k].nome) == 0) {

                                    arg->todos_clis[j].emp[i].price = arg->array_emp[k].price;
                                    Cli_atual.emp[i] = arg->todos_clis[j].emp[i];

                                }

                            }

                        }
                ReleaseMutex(arg->hMutex);
            }
            else if (_tcscmp(Cli_atual.comando, TEXT("balance")) == 0) {
                for (int j = 0; j < N && arg->todos_clis[j].preenchido == TRUE; j++)
                    if (Cli_atual.ID == arg->todos_clis[j].ID)
                        Cli_atual.saldo = arg->todos_clis[j].saldo;
            }
            if (_tcscmp(Cli_atual.comando, TEXT("exit")) == 0)
                Cli_atual.resultado = 99;


        }
        if (_tcscmp(Cli_atual.comando, TEXT("exit")) == 0)
            Cli_atual.resultado = 99;

        //-------------------- Escrita no PIPE ----------------


        ZeroMemory(&OverWr, sizeof(OverWr));
        ResetEvent(arg->hEvent);
        OverWr.hEvent = arg->hEvent;

        fSucess = WriteFile(arg->hPipe, &Cli_atual, sizeof(Cli_atual), &cbBytesWritten, &OverWr);

        if (!fSucess) {
            _tprintf(TEXT("[ERRO] Falha na escrita do pipe (erro %d)\n"), GetLastError());
            break;
        }

        WaitForSingleObject(arg->hEvent, INFINITE);

        if (!GetOverlappedResult(arg->hPipe, &OverWr, &cbBytesWritten, FALSE)) {
            _tprintf(TEXT("[ERRO] Falha ao obter resultado da leitura (erro %d)\n"), GetLastError());
            break; // Encerrar o loop se não for possível obter o resultado
        }

    }
    for (size_t i = 0; i < N; i++)
        if (Cli_atual.ID == arg->todos_clis[i].ID)
            arg->todos_clis[i].logado = FALSE;


}

void ordenarEmpresas(empresas* pEmpresas, empresas* Top_emp, int num) {

    empresas copiaEmpresas[N];
    for (int i = 0; i < N; i++) {
        copiaEmpresas[i] = pEmpresas[i];
    }
    int i, j, max_idx;

    for (i = 0; i < N - 1; i++) {
        max_idx = i;
        for (j = i + 1; j < N; j++) {
            if (copiaEmpresas[j].price > copiaEmpresas[max_idx].price) {
                max_idx = j;
            }
        }
        if (max_idx != i) {
            empresas temp = copiaEmpresas[i];
            copiaEmpresas[i] = copiaEmpresas[max_idx];
            copiaEmpresas[max_idx] = temp;
        }
    }

    for (i = 0; i < 10 && i < num; i++) {
        Top_emp[i] = copiaEmpresas[i];
    }
}

DWORD WINAPI TshareMeme(LPVOID lpParam) {

    TmemoriaPartilhada* arg = (TmemoriaPartilhada*)lpParam;

    while (*(arg->flag) == 0) {

        ordenarEmpresas(arg->pEmpresas, arg->estrutura.Top_emp, 10);
        Sleep(5000);

            _tcsncpy_s(arg->estrutura.fraseUltimaT, 200, arg->fraseUltimaT, 199);
            arg->estrutura.fraseUltimaT[199] = '\0'; 
      
        WaitForSingleObject(arg->hMutex, INFINITE);

        CopyMemory(arg->pMapeamento, &arg->estrutura, sizeof(DadosPartilha) * 2);

        ReleaseMutex(arg->hMutex);

        SetEvent(arg->hEventoMem);

    }

}

DWORD WINAPI ThresperaCompra(LPVOID lpParam) {

    Ttime* arg = (Ttime*)lpParam;
    while (1) {
        WaitForSingleObject(arg->hEventPoderC, INFINITE);
        *(arg->flag_poderComprar) = 1;
        Sleep(*(arg->valoremSeg));
        *(arg->flag_poderComprar) = 0;
        ResetEvent(arg->hEventPoderC);
    }
}