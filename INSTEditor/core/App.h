#pragma once
#include <Windows.h>
#include "..\code\INSTEditor.h"


class eApp {
public:
	static HINSTANCE hInst;
	static HWND      hWindow;
	static HWND      hList;
	static HWND      hParams;
	static HWND      hLog;
	static HWND      hTable;
	static HMENU     hMenu;
	static HMENU     hPopupMenu;
	static DWORD     dwSel;
	static BOOL      bIsReady;
	static BOOL      bIsIni;
	static BOOL      bRequiresSaving;
	static int       nGameMode;
	static INSTEditor*    pINSTEditor;
	static INT_PTR CALLBACK Process(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK EditParam(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	static void Reset();
	static void CreateTooltip(HWND hWnd, LPCWSTR text);
	static void UpdateGameChange();
	static void Begin();
};