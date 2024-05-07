#include <windows.h>
#include <stdio.h>
#include <dbghelp.h>

#pragma comment(lib, "user32.lib")
#pragma comment (lib, "dbghelp.lib")

//-- pointer to the original MessageBox function
int (WINAPI * pOrigMessageBox)(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType) = MessageBox;


//-- the modified MessageBox function
int ModifiedMessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType) {
	
	printf("ModifiedMessageBox() called. No MessageBox popup on screen!\n");
	
	//pOrigMessageBox(hWnd, lpText, lpCaption, uType);
	
	//if(strcmp(lpCaption, "2nd MessageBox")==0)
		//pOrigMessageBox(hWnd, "I have been modified", lpCaption, MB_OK | MB_ICONERROR);
	
	return IDOK;
}


//-- Set hook on origFunc() by replacing it with our own function
BOOL HookTarget(char * dll, char * origFunc, PROC hookingFunc) {

    ULONG size;
	DWORD i;
	BOOL found = FALSE;

	//-- get the base address of the module which is aka the handle of the module
	HANDLE baseAddress = GetModuleHandle(NULL);			
	
	//-- get Import Table of main module
	PIMAGE_IMPORT_DESCRIPTOR importTbl = (PIMAGE_IMPORT_DESCRIPTOR) ImageDirectoryEntryToDataEx(
												baseAddress,
												TRUE,
												IMAGE_DIRECTORY_ENTRY_IMPORT,
												&size,
												NULL);

	//-- search for the DLL we want 
	for (i = 0; i < size ; i++){
		char * importName = (char *)((PBYTE) baseAddress + importTbl[i].Name);
		if (_stricmp(importName, dll) == 0) {
				found = TRUE;
				break;
		}
	}
	if (!found)
		return FALSE;

	
	//-- Search for the function we want in the Import Address Table --
	PROC origFuncAddr = (PROC) GetProcAddress(GetModuleHandle(dll), origFunc);

	PIMAGE_THUNK_DATA thunk = (PIMAGE_THUNK_DATA) ((PBYTE) baseAddress + importTbl[i].FirstThunk);
	while (thunk->u1.Function) {
		PROC * currentFuncAddr = (PROC *) &thunk->u1.Function;
		
		// found
		if (*currentFuncAddr == origFuncAddr) {

			//-- set memory to become writable
			DWORD oldProtect = 0;
			VirtualProtect((LPVOID) currentFuncAddr, 4096, PAGE_READWRITE, &oldProtect);

			//-- set the hook by assigning new modified function to replace the old one
			*currentFuncAddr = (PROC)hookingFunc;

			//-- revert back to original protection setting
			VirtualProtect((LPVOID) currentFuncAddr, 4096, oldProtect, &oldProtect);

			printf("Hook has been set on IAT function %s()\n", origFunc);
			return TRUE;
		}
	thunk++;
	}
	
	return FALSE;
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved) {

    switch (dwReason)  {
		case DLL_PROCESS_ATTACH:
			HookTarget("user32.dll", "MessageBoxA", (PROC) ModifiedMessageBox);
			break;
			
		case DLL_THREAD_ATTACH:
			break;
			
		case DLL_THREAD_DETACH:
			break;
			
		case DLL_PROCESS_DETACH:
			break;
	}
	
    return TRUE;
}

