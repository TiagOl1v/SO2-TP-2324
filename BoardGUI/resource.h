//{{NO_DEPENDENCIES}}
// Arquivo de inclusão gerado pelo Microsoft Visual C++.
// Usado por BoardGUI.rc
//
#define IDI_ICON1                       101
#define IDR_MENU1                       102
#define IDD_DIALOG2                     108
#define ID_TOPEMPRESAS                  40001
#define ID_ULTIMATRANSACAO              40002
#define ID_SAIR                         40003


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


typedef struct {
	HWND hWnd;
	HANDLE hEvent;
	HANDLE hMutex;
	HANDLE hFileMap;
	DadosPartilha* MemP;
} ThreadParams;

// Next default values for new objects
// 
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE        110
#define _APS_NEXT_COMMAND_VALUE         40004
#define _APS_NEXT_CONTROL_VALUE         1011
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif
