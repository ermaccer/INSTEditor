#include "App.h"

#include "..\resource.h"
#include "FileFunctions.h"
#include <CommCtrl.h>
#include <windowsx.h>
#include <filesystem>
#include "IniReader.h"

HINSTANCE        eApp::hInst;
HWND             eApp::hWindow;
HWND             eApp::hList;
HWND             eApp::hParams;
HWND             eApp::hLog;
HWND             eApp::hTable;
HMENU            eApp::hMenu;
HMENU            eApp::hPopupMenu;
DWORD            eApp::dwSel;
BOOL             eApp::bIsReady;
BOOL             eApp::bIsIni;
BOOL			 eApp::bRequiresSaving;
int              eApp::nGameMode;

INSTEditor*	     eApp::pINSTEditor;


const wchar_t* szTabNames[TOTAL_TABS] = {
	L"ID",
	L"Type",
	L"Value"
};


INT_PTR CALLBACK eApp::Process(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	hWindow = hDlg;
	HANDLE hicon = 0;



	if (!bIsReady)
	{
		EnableWindow(GetDlgItem(hDlg, ADD_INSTANCE), FALSE);
		EnableWindow(GetDlgItem(hDlg, APPLY_CHANGES), FALSE);
		EnableWindow(GetDlgItem(hDlg, DELETE_INSTANCE), FALSE);

		EnableWindow(GetDlgItem(hDlg, EXPORT_INI), FALSE);
		EnableWindow(GetDlgItem(hDlg, IMPORT_INI), FALSE);

		EnableMenuItem(hMenu, ID_FILE_CLOSE, MF_DISABLED);
		EnableMenuItem(hMenu, ID_FILE_SAVE, MF_DISABLED);
		EnableMenuItem(hMenu, ID_FILE_SAVEAS, MF_DISABLED);
	}
	else
	{
		EnableWindow(GetDlgItem(hDlg, ADD_INSTANCE), TRUE);
		EnableWindow(GetDlgItem(hDlg, DELETE_INSTANCE), TRUE);
		EnableWindow(GetDlgItem(hDlg, APPLY_CHANGES), TRUE);


		EnableWindow(GetDlgItem(hDlg, EXPORT_INI), TRUE);
		EnableWindow(GetDlgItem(hDlg, IMPORT_INI), TRUE);

		EnableMenuItem(hMenu, ID_FILE_CLOSE, MF_ENABLED);
		EnableMenuItem(hMenu, ID_FILE_SAVE, MF_ENABLED);
		EnableMenuItem(hMenu, ID_FILE_SAVEAS, MF_ENABLED);
	}




	switch (message)
	{
	case WM_INITDIALOG:
		Reset();
		hList = GetDlgItem(hDlg, INST_LIST);
		hParams = GetDlgItem(hDlg, INST_PARM);
		hMenu = GetMenu(hDlg);
		hicon = LoadImage(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDI_INSTEDITOR), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE);
		LVCOLUMN LvCol;
		LvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
		LvCol.cx = 115;
		LvCol.pszText = (LPWSTR)szTabNames[TAB_NAME];
		SendMessage(hParams, LVM_INSERTCOLUMN, TAB_NAME, (LPARAM)&LvCol);
		LvCol.pszText = (LPWSTR)szTabNames[TAB_TYPE];
		SendMessage(hParams, LVM_INSERTCOLUMN, TAB_TYPE, (LPARAM)&LvCol);
		LvCol.pszText = (LPWSTR)szTabNames[TAB_VALUE];
		SendMessage(hParams, LVM_INSERTCOLUMN, TAB_VALUE, (LPARAM)&LvCol);
		SendMessage(hParams, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
		hPopupMenu = CreatePopupMenu();
		InsertMenu(hPopupMenu, 0, MF_STRING | MF_ENABLED, MENU_EDIT, L"&Edit");
		InsertMenu(hPopupMenu, 0, MF_STRING | MF_DISABLED, MENU_CHANGE_TYPE, L"&Change type");

		SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hicon);
		return (INT_PTR)TRUE;
	case WM_CLOSE:
		if (bRequiresSaving)
		{
			if (MessageBox(hDlg,L"Do you want to quit without saving?",L"Information",MB_ICONINFORMATION | MB_YESNO) == IDYES)
				EndDialog(hDlg, LOWORD(wParam));
		}
		else
			EndDialog(hDlg, LOWORD(wParam));
		return (INT_PTR)TRUE;
	case WM_CONTEXTMENU:
		if ((HWND)wParam == hParams && eApp::bIsReady) {
			TrackPopupMenu(hPopupMenu, TPM_TOPALIGN | TPM_LEFTALIGN, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), NULL, hDlg, NULL);
		}
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == ID_OPEN_MANHUNT)
		{
			pINSTEditor = new INSTEditor();
			pINSTEditor->Init(&hLog, &hList, &hParams);
			pINSTEditor->OpenFile(SetPathFromButton(L"Instance Data (.inst)\0*.inst\0All Files (*.*)\0*.*\0", L"inst", hDlg),MODE_MANHUNT);
		}
		if (LOWORD(wParam) == ID_NEW_MANHUNT)
		{
			pINSTEditor = new INSTEditor();
			pINSTEditor->Init(&hLog, &hList, &hParams);
			pINSTEditor->CreateNewFile(SetSavePathFromButton(L"Instance Data (.inst)\0*.inst\0All Files (*.*)\0*.*\0", L"inst", hDlg), MODE_MANHUNT);
		}
		if (LOWORD(wParam) == ID_FILE_SAVE)
			pINSTEditor->Save();
		if (LOWORD(wParam) == ID_FILE_SAVEAS)
			pINSTEditor->SaveAs();
		if (LOWORD(wParam) == ID_FILE_CLOSE)
			pINSTEditor->Close();


		if (HIWORD(wParam) == LBN_SELCHANGE)
			pINSTEditor->DisplayINSTData();

		if (wParam == MENU_EDIT)
		{
			if (SendMessage(hParams, LVM_GETNEXTITEM, -1, LVNI_FOCUSED) >= 0)
				DialogBox(hInst, MAKEINTRESOURCE(EDIT_PARAM), hDlg, EditParam);
		}

		if (LOWORD(wParam) == ADD_INSTANCE)
			pINSTEditor->AddInstance();

		if (LOWORD(wParam) == DELETE_INSTANCE)
			pINSTEditor->DeleteInstance();

		if (LOWORD(wParam) == EXPORT_INI)
			pINSTEditor->ExportINI();
		if (LOWORD(wParam) == IMPORT_INI)
			pINSTEditor->ImportINI();
		if (LOWORD(wParam) == APPLY_CHANGES)
			pINSTEditor->ApplyChanges();
		if (wParam == IDM_ABOUT)
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hDlg, About);

		if (LOWORD(wParam) == IDM_EXIT)
		{
			if (bRequiresSaving)
			{
				if (MessageBox(hDlg, L"Do you want to quit without saving?", L"Information", MB_ICONINFORMATION | MB_YESNO) == IDYES)
					EndDialog(hDlg, LOWORD(wParam));
			}
			else
				EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}

	}
	return (INT_PTR)FALSE;
}

INT_PTR eApp::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
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

INT_PTR eApp::EditParam(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
	case WM_INITDIALOG:
		DWORD dwSel, dwListSel;
		dwSel = SendMessage(hList, LB_GETCURSEL, 0, 0);
		dwListSel = SendMessage(hParams, LVM_GETNEXTITEM, -1, LVNI_FOCUSED);
		if (dwSel >= 0)
		{
			std::wstring value;
			value = std::to_wstring(pINSTEditor->Instances[dwSel].mh_params[dwListSel]);
			SetWindowText(GetDlgItem(hDlg, EP_VAL), value.c_str());
		}
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == EP_CONFIRM)
		{
			DWORD dwSel, dwListSel;
			dwSel = SendMessage(hList, LB_GETCURSEL, 0, 0);
			dwListSel = SendMessage(hParams, LVM_GETNEXTITEM, -1, LVNI_FOCUSED);
			if (dwSel >= 0)
			{
				unsigned int value;
				BOOL status = false;

				value = GetDlgItemInt(hDlg,EP_VAL, NULL, false);
				pINSTEditor->Instances[dwSel].mh_params[dwListSel] = value;
				pINSTEditor->DisplayINSTData();
			}

			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}

		if (LOWORD(wParam) == EP_CANCEL || LOWORD(wParam) == IDM_EXIT)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}



void eApp::Reset()
{
	nGameMode = 0;
}

void eApp::CreateTooltip(HWND hWnd, LPCWSTR text)
{
	HWND hwndTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, WS_POPUP |
		TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hWnd, NULL,
		hInst, NULL);
	SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	TOOLINFO ti;
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
	ti.hwnd = hWindow;
	ti.hinst = NULL;
	ti.uId = (UINT_PTR)hWnd;
	ti.lpszText = (LPWSTR)text;

	RECT rect;
	GetClientRect(hWnd, &rect);

	ti.rect.left = rect.left;
	ti.rect.top = rect.top;
	ti.rect.right = rect.right;
	ti.rect.bottom = rect.bottom;

	SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)&ti);
}

void eApp::UpdateGameChange()
{

}

void eApp::Begin()
{
	pINSTEditor = nullptr;
	DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), 0, Process);
}
