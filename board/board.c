#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

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

typedef struct {

	empresas Top_emp[10];
	TCHAR fraseUltimaT[200];
	int num;

} DadosPartilha;

int _tmain(int argc, TCHAR* argv[]) {

	HANDLE hFileMap, hEventMemeS, hMutexMeme, hThreadMeme;
	DadosPartilha *MemP;
	DadosPartilha estrut;

#ifdef UNICODE 
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	hEventMemeS  = OpenEvent(EVENT_ALL_ACCESS, FALSE, NAME_EvMeme);
	hMutexMeme = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEX_MEME_NAME);

	hFileMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, SHM_NAME);
	if (hFileMap == NULL) {
		_tprintf(TEXT("Erro ao abrir memoria partilhada\n"));
		CloseHandle(hEventMemeS);
		CloseHandle(hMutexMeme);
	}

	MemP = (DadosPartilha*)MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);



	while (1)
	{
		WaitForSingleObject(hEventMemeS, INFINITE);
		WaitForSingleObject(hMutexMeme, INFINITE);

		CopyMemory(&estrut, MemP, sizeof(DadosPartilha)*2);

		_tprintf(TEXT("\n------As %d empresas mais caras ------\n"),estrut.num);
		for (int i = 0; i < estrut.num && estrut.Top_emp[i].preenchido == TRUE; i++)
			_tprintf(TEXT("Nome: %s, num de acoes: %d, Preco: %.2f\n"), estrut.Top_emp[i].nome, estrut.Top_emp[i].numDeAcoes, estrut.Top_emp[i].price);

		_tprintf(TEXT("\n----- Ultima transacao foi -----\n"));
		_tprintf(TEXT(" %s \n"), estrut.fraseUltimaT);
		
		ReleaseMutex(hMutexMeme);
		ResetEvent(hEventMemeS);
	

	}

	UnmapViewOfFile(MemP);
	CloseHandle(hFileMap);
	CloseHandle(hEventMemeS);
	CloseHandle(hMutexMeme);


	return 0;
}