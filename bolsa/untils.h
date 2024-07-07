#ifndef __UNTILS_H__
#define __UNTILS_H__
#pragma once

#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <windows.h>

#define PIPE_NAME TEXT("\\\\.\\pipe\\PipeCLIENTES")

#define SHM_NAME _TEXT("Share_meme")
#define NAME_EvMeme _TEXT("Event_meme")
#define MUTEX_MEME_NAME _TEXT("Mutex_meme")

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

    empresas Top_emp[10];
    TCHAR fraseUltimaT[200];
    int num;

} DadosPartilha;

typedef struct T_PIPE {

    Clientes *todos_clis;
    HANDLE hEvent;
    HANDLE hMutex;
    HANDLE hPipe;
    HANDLE flagEvent;
    int* flag;
    int terminar;
    empresas* array_emp;
    int* flag_poderComprar;
    TCHAR* fraseUltimaT;

}ThreadCLI;

typedef struct {

    HANDLE flagEvent;
    int* flag;
    empresas* copy_arrayEmp;
    HANDLE* hPipe_emEspera[1];
    HANDLE MUTEX;
    Clientes* array_cli;

    int* flag_poderComprar;
    HANDLE hEventPoderC;
    int* valoremSeg;

} TDados;


typedef struct {

    empresas* pEmpresas;
    HANDLE hEventoMem, hMutex, hEventEnd;
    int* flag;
    empresas* pMapeamento;
    DadosPartilha estrutura;
    TCHAR* fraseUltimaT;

} TmemoriaPartilhada;


typedef struct {

    int* flag_poderComprar;
    HANDLE hEventPoderC;
    int* valoremSeg;

} Ttime;



DWORD WINAPI Th_TrataCLI(LPVOID lpParam);

DWORD WINAPI teclado(LPVOID lpParam);

DWORD WINAPI TshareMeme(LPVOID lpParam);

DWORD WINAPI ThresperaCompra(LPVOID lpParam);

#endif