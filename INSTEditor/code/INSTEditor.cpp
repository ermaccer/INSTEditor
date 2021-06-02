#include "..\core\App.h"
#include "INSTEditor.h"
#include <CommCtrl.h>
#include <shlobj.h>
#include <WinUser.h>
#include <filesystem>
#include <windowsx.h>
#include "..\core\FileFunctions.h"
#include "..\resource.h"
#include "..\core\IniReader.h"
#include <iostream>



void INSTEditor::Init(HWND * log, HWND * list, HWND * box)
{
	eApp::bIsReady = false;
	hLogBox = log;
	hFilesList = list;
	hParams = box;

}


void INSTEditor::CreateNewFile(std::wstring input, eGameMode game)
{
	InputPath = input;
	if (InputPath.empty())
		return;
	eApp::bIsReady = TRUE;
	eApp::UpdateGameChange();
	eApp::bRequiresSaving = true;
	SetWindowText(GetDlgItem(eApp::hWindow, FILE_PATH), InputPath.c_str());
}

void INSTEditor::OpenFile(std::wstring input, eGameMode game)
{
	if (pFile.is_open())
		pFile.close();

	Instances.clear();
	InputPath = L" ";
	OutputPath = L" ";
	InputPath = input;
	m_gameMode = game;
	SendMessage(*hFilesList, LB_RESETCONTENT, 0, 0);
	SetWindowText(GetDlgItem(eApp::hWindow, FILE_PATH), 0);
	eApp::bRequiresSaving = false;
	ReadFile();
}

void INSTEditor::ReadFile()
{
	if (InputPath.empty())
		return;
	pFile.open(InputPath, std::ifstream::binary);

	if (!pFile.is_open())
	{
		MessageBox(eApp::hWindow, L"Failed to open file!", L"Error", MB_ICONERROR);
		return;
	}

	if (pFile.is_open())
	{
		int instanceCount = 0;

		pFile.read((char*)&instanceCount, sizeof(int));

		std::vector<int> instSizes;

		// read sizes;

		for (int i = 0; i < instanceCount; i++)
		{
			int instSize = 0;
			pFile.read((char*)&instSize, sizeof(int));
			instSizes.push_back(instSize);
		}

		// read data

		for (int i = 0; i < instanceCount; i++)
		{
			instance_entry inst;
			int padOffset = 0;
			int readBytes = 0;

			std::string record;
			std::getline(pFile, record, '\0');
			readBytes += record.length() + 1;

			padOffset = makePad(record.length() + 1, INST_PAD_VALUE);
			padOffset -= record.length() + 1;
			readBytes += padOffset;
			pFile.seekg(padOffset, pFile.cur);


			std::string name;
			std::getline(pFile, name, '\0');
			readBytes += name.length() + 1;
			padOffset = makePad(name.length() + 1, INST_PAD_VALUE);
			padOffset -= name.length() + 1;
			readBytes += padOffset;
			pFile.seekg(padOffset, pFile.cur);


			vector3d pos;
			pFile.read((char*)&pos, sizeof(vector3d));
			readBytes += sizeof(vector3d);

			quaternion3d rot;
			pFile.read((char*)&rot, sizeof(quaternion3d));
			readBytes += sizeof(quaternion3d);

			std::string className;
			std::getline(pFile, className, '\0');
			readBytes += className.length() + 1;



			padOffset = makePad(className.length() + 1, INST_PAD_VALUE);
			padOffset -= className.length() + 1;
			readBytes += padOffset;
			pFile.seekg(padOffset, pFile.cur);

			inst.name = name;
			inst.recordName = record;
			inst.position = pos;
			inst.rotation = rot;
			inst.className = className;


			int amountOfParams = (instSizes[i] - readBytes) / sizeof(int);

			for (int a = 0; a < amountOfParams; a++)
			{
				if (m_gameMode == MODE_MANHUNT)
				{
					int param;
					pFile.read((char*)&param, sizeof(int));
					inst.mh_params.push_back(param);
				}
			}

			Instances.push_back(inst);

		}
		ListData();
		eApp::bIsReady = TRUE;

		SetWindowText(GetDlgItem(eApp::hWindow, FILE_PATH), InputPath.c_str());
	}
}

void INSTEditor::Save()
{
	if (pFile.is_open())
		pFile.close();

	std::ofstream oFile(InputPath, std::ofstream::binary);

	int instanceCount = Instances.size();

	oFile.write((char*)&instanceCount, sizeof(int));

	// count size of each instance 

	for (unsigned int i = 0; i < Instances.size(); i++)
	{
		int size = 0;
		size += makePad(Instances[i].recordName.length() + 1, INST_PAD_VALUE);
		size += makePad(Instances[i].name.length() + 1, INST_PAD_VALUE);
		size += sizeof(vector3d);
		size += sizeof(quaternion3d);
		size += makePad(Instances[i].className.length() + 1, INST_PAD_VALUE);
		size += Instances[i].mh_params.size() * sizeof(int);
		
		oFile.write((char*)&size, sizeof(int));
	}

	// write data

	for (unsigned int i = 0; i < Instances.size(); i++)
	{
		char pad = 'p';
		char end = 0x00;
		int str_len = Instances[i].recordName.length() + 1;
		oFile.write(Instances[i].recordName.data(), str_len - 1);
		oFile.write((char*)&end, sizeof(char));
		int pad_len = makePad(str_len, INST_PAD_VALUE) - str_len;

		for (unsigned int a = 0; a < pad_len; a++)
			oFile.write((char*)&pad, sizeof(char));

		str_len = Instances[i].name.length() + 1;
		oFile.write(Instances[i].name.data(), str_len - 1);
		oFile.write((char*)&end, sizeof(char));
		pad_len = makePad(str_len, INST_PAD_VALUE) - str_len;
		for (unsigned int a = 0; a < pad_len; a++)
			oFile.write((char*)&pad, sizeof(char));;

		oFile.write((char*)&Instances[i].position, sizeof(vector3d));
		oFile.write((char*)&Instances[i].rotation, sizeof(quaternion3d));

		str_len = Instances[i].className.length() + 1;
		oFile.write(Instances[i].className.data(), str_len - 1);
		oFile.write((char*)&end, sizeof(char));
		pad_len = makePad(str_len, INST_PAD_VALUE) - str_len;
		for (unsigned int a = 0; a < pad_len; a++)
			oFile.write((char*)&pad, sizeof(char));

		for (unsigned int a = 0; a < Instances[i].mh_params.size(); a++)
			oFile.write((char*)&Instances[i].mh_params[a], sizeof(int));


	}
	eApp::bRequiresSaving = false;
	MessageBox(eApp::hWindow, L"INST saved!", L"Information", MB_ICONINFORMATION);

}

void INSTEditor::SaveAs()
{
	std::wstring originalPath = InputPath;

	InputPath = SetSavePathFromButton(L"Instance Data (.inst)\0*.inst\0All Files (*.*)\0*.*\0", L"inst", eApp::hWindow);

	if (InputPath.empty())
	{
		InputPath = originalPath;
		return;
	}

	Save();
	InputPath = originalPath;
}

void INSTEditor::Reset()
{
	Instances.clear();
	OutputPath = L" ";
	OutputPath = L" ";
	SendMessage(*hFilesList, LB_RESETCONTENT, 0, 0);
	SendMessage(*hParams, LVM_DELETEALLITEMS, 0, 0);
	SetWindowText(GetDlgItem(eApp::hWindow, INSTANCE_NAME), 0);
	SetWindowText(GetDlgItem(eApp::hWindow, RECORD_NAME), 0);
	SetWindowText(GetDlgItem(eApp::hWindow, INSTANCE_CLASS),0);


	SetWindowText(GetDlgItem(eApp::hWindow, POS_X), 0);
	SetWindowText(GetDlgItem(eApp::hWindow, POS_Y), 0);
	SetWindowText(GetDlgItem(eApp::hWindow, POS_Z), 0);


	SetWindowText(GetDlgItem(eApp::hWindow, ROT_X), 0);
	SetWindowText(GetDlgItem(eApp::hWindow, ROT_Y), 0);
	SetWindowText(GetDlgItem(eApp::hWindow, ROT_Z), 0);
	SetWindowText(GetDlgItem(eApp::hWindow, ROT_W), 0);

	SetWindowText(GetDlgItem(eApp::hWindow, FILE_PATH), 0);
	eApp::bRequiresSaving = false;

}

void INSTEditor::Close()
{
	if (pFile.is_open())
		pFile.close();
	Reset();
	eApp::bIsReady = FALSE;
	eApp::bRequiresSaving = false;
	SetWindowText(GetDlgItem(eApp::hWindow, FILE_PATH), 0);
}

void INSTEditor::ListData()
{
	SendMessage(*hFilesList, LB_RESETCONTENT, 0, 0);

	for (unsigned int i = 0; i < Instances.size(); i++)
	{
		std::wstring wstr(Instances[i].name.length(), L' ');
		std::copy(Instances[i].name.begin(), Instances[i].name.end(), wstr.begin());
		SendMessage(*hFilesList, LB_ADDSTRING, 0, (LPARAM)wstr.c_str());
	}

}

void INSTEditor::ListParams(DWORD id)
{
	wchar_t wstr_buff[256] = {};

	SendMessage(*hParams, LVM_DELETEALLITEMS, 0, 0);
	for (unsigned int i = 0; i < Instances[id].mh_params.size(); i++)
	{
		DWORD itemCount = SendMessage(*hParams, LVM_GETITEMCOUNT, 0, 0);

		LVITEM LvItem;
		LvItem.mask = LVIF_TEXT;
		LvItem.cchTextMax = 255;


		// param 1 - health

		switch (i)
		{
		case 0:
			wsprintf(wstr_buff, L"Health");
			break;
		default:
			wsprintf(wstr_buff, L"%d", i + 1);
			break;
		}

		if (Instances[id].className == "Hunter_Inst")
		{
			switch (i)
			{
			case 0:
				wsprintf(wstr_buff, L"Health");
				break;
			case 4:
				wsprintf(wstr_buff, L"Melee Weapon");
				break;
			case 5:
				wsprintf(wstr_buff, L"Ranged Weapon");
				break;
			default:
				wsprintf(wstr_buff, L"%d", i + 1);
				break;
			}

		}


		LvItem.iItem = itemCount;
		LvItem.iSubItem = TAB_NAME;
		LvItem.pszText = wstr_buff;
		SendMessage(*hParams, LVM_INSERTITEM, 0, (LPARAM)&LvItem);

		LvItem.iItem = itemCount;
		LvItem.iSubItem = TAB_TYPE;
		LvItem.pszText = L"";
		SendMessage(*hParams, LVM_SETITEM, 0, (LPARAM)&LvItem);


		LvItem.iItem = itemCount;
		LvItem.iSubItem = TAB_VALUE;


		wsprintf(wstr_buff, L"%d", Instances[id].mh_params[i]);


		LvItem.pszText = wstr_buff;
		SendMessage(*hParams, LVM_SETITEM, 0, (LPARAM)&LvItem);
	
	}


}

void INSTEditor::SetActiveIndex(int id)
{
	if (id < 0)
		id = 0;

	ListBox_SetCurSel(*hFilesList, id);
	ListBox_SetCaretIndex(*hFilesList, id);
	DisplayINSTData();
}

void INSTEditor::AddInstance()
{
	instance_entry inst;

	inst.mh_params.push_back(100);
	inst.className = "Base_Inst";
	inst.name = "NewInstance";
	inst.recordName = "RecordName";
	inst.position = { 0.0,0.0,0.0 };
	inst.rotation = { 0.0,0.0,0.0,1.0};
	

	Instances.push_back(inst);
	ListData();
	SetActiveIndex(Instances.size() - 1);
	eApp::bRequiresSaving = true;
}

void INSTEditor::DeleteInstance()
{
	if (Instances.size() > 0)
	{
		DWORD dwSel = SendMessage(*hFilesList, LB_GETCURSEL, 0, 0);

		if (dwSel >= 0 && eApp::bIsReady)
		{
			Instances.erase(Instances.begin() + dwSel);
			ListData();
			SetActiveIndex(dwSel - 1);
		}
	}
	eApp::bRequiresSaving = true;
}

void INSTEditor::ApplyChanges()
{
	DWORD dwSel = SendMessage(*hFilesList, LB_GETCURSEL, 0, 0);

	if (dwSel >= 0 && eApp::bIsReady)
	{
		// make strings

		wchar_t buff[512] = {};
		GetWindowText(GetDlgItem(eApp::hWindow, INSTANCE_NAME), buff, sizeof(buff));
		std::wstring w_name = buff;
		std::string name("", w_name.length());
		std::copy(w_name.begin(), w_name.end(), name.begin());

		Instances[dwSel].name = name;


		GetWindowText(GetDlgItem(eApp::hWindow, INSTANCE_CLASS), buff, sizeof(buff));
		std::wstring w_class = buff;
		std::string s_class("", w_class.length());
		std::copy(w_class.begin(), w_class.end(), s_class.begin());

		Instances[dwSel].className = s_class;


		GetWindowText(GetDlgItem(eApp::hWindow, RECORD_NAME), buff, sizeof(buff));
		std::wstring w_record = buff;
		std::string record("", w_record.length());
		std::copy(w_record.begin(), w_record.end(), record.begin());

		Instances[dwSel].recordName = record;

		GetWindowText(GetDlgItem(eApp::hWindow, POS_X), buff, sizeof(buff));
		swscanf(buff, L"%f", &Instances[dwSel].position.x);

		GetWindowText(GetDlgItem(eApp::hWindow, POS_Y), buff, sizeof(buff));
		swscanf(buff, L"%f", &Instances[dwSel].position.y);


		GetWindowText(GetDlgItem(eApp::hWindow, POS_Z), buff, sizeof(buff));
		swscanf(buff, L"%f", &Instances[dwSel].position.z);


		GetWindowText(GetDlgItem(eApp::hWindow, ROT_X), buff, sizeof(buff));
		swscanf(buff, L"%f", &Instances[dwSel].rotation.x);


		GetWindowText(GetDlgItem(eApp::hWindow, ROT_Y), buff, sizeof(buff));
		swscanf(buff, L"%f", &Instances[dwSel].rotation.y);


		GetWindowText(GetDlgItem(eApp::hWindow, ROT_Z), buff, sizeof(buff));
		swscanf(buff, L"%f", &Instances[dwSel].rotation.z);

		GetWindowText(GetDlgItem(eApp::hWindow, ROT_W), buff, sizeof(buff));
		swscanf(buff, L"%f", &Instances[dwSel].rotation.w);

		MessageBeep(MB_ICONINFORMATION);
		ListData();
		SetActiveIndex(dwSel);
		eApp::bRequiresSaving = true;
	}
}

void INSTEditor::ExportINI()
{
	OutputPath = SetSavePathFromButton(L"Configuration File\0*.ini\0All Files (*.*)\0*.*\0", L"ini", eApp::hWindow);

	if (OutputPath.empty())
		return;

	std::wofstream ini(OutputPath, std::ofstream::binary);


	ini << L"[Settings]\n";
	ini << L"Instances = " << Instances.size() << std::endl;
	ini << L"Game = " << m_gameMode << std::endl;


	for (unsigned int i = 0; i < Instances.size(); i++)
	{

		// make wstrings

		std::wstring w_name(Instances[i].name.length(), L' ');
		std::copy(Instances[i].name.begin(), Instances[i].name.end(), w_name.begin());

		std::wstring w_record(Instances[i].recordName.length(), L' ');
		std::copy(Instances[i].recordName.begin(), Instances[i].recordName.end(), w_record.begin());

		std::wstring w_class(Instances[i].className.length(), L' ');
		std::copy(Instances[i].className.begin(), Instances[i].className.end(), w_class.begin());



		ini << L"[INST" + std::to_wstring(i) + L"]" << std::endl;
		ini << L"Name = " + w_name << std::endl;
		ini << L"Record = " + w_record << std::endl;
		ini << L"Class = " + w_class<< std::endl;
		ini << L"Pos = " << Instances[i].position.x << L" " << Instances[i].position.y << L" " << Instances[i].position.z << std::endl;
		ini << L"Rot = " << Instances[i].rotation.x << L" " << Instances[i].rotation.y << L" " << Instances[i].rotation.z << L" " << Instances[i].rotation.w <<std::endl;
		ini << L"Params = " << Instances[i].mh_params.size() << std::endl;
		for (unsigned int a = 0; a < Instances[i].mh_params.size(); a++)
		{
			ini << L"Param" + std::to_wstring(a) + L" = " << Instances[i].mh_params[a] <<std::endl;
		}
	}
	ini.close();
	MessageBox(eApp::hWindow, L"INI exported!", L"Information", MB_ICONINFORMATION);
}

void INSTEditor:: ImportINI()
{
	OutputPath = SetPathFromButton(L"Configuration File\0*.ini\0All Files (*.*)\0*.*\0", L"ini", eApp::hWindow);

	if (OutputPath.empty())
		return;

	CIniReader ini(OutputPath.c_str());

	int instanceCount = ini.ReadInteger(L"Settings", L"Instances", 0);

	if (instanceCount > 0)
	{

		int append = MessageBox(eApp::hWindow, L"Do you want to append INI items to current INST? Selecting No will clear the table and add all items from file.", L"Information", MB_ICONINFORMATION | MB_YESNOCANCEL);
		if (append == IDCANCEL)
			return;

		if (append == IDNO)
			Instances.clear();
		wchar_t buf[256] = {};
		for (int i = 0; i < instanceCount; i++)
		{
			instance_entry inst;

			wsprintf(buf, L"INST%d", i);
			std::wstring name = ini.ReadString(buf, L"Name", L"null");
			std::string tmp_n("", name.length());
			std::copy(name.begin(), name.end(), tmp_n.begin());
			inst.name = tmp_n;

			std::wstring record = ini.ReadString(buf, L"Record", L"null");
			std::string tmp_r("", record.length());
			std::copy(record.begin(), record.end(), tmp_r.begin());
			inst.recordName = tmp_r;

			std::wstring classname = ini.ReadString(buf, L"Class", L"null");
			std::string tmp_c("", classname.length());
			std::copy(classname.begin(), classname.end(), tmp_c.begin());
			inst.className = tmp_c;

			wchar_t* pos = ini.ReadString(buf, L"Pos", L"0 0 0");
			swscanf(pos, L"%f %f %f", &inst.position.x, &inst.position.y, &inst.position.z);
			wchar_t* rot = ini.ReadString(buf, L"Rot", L"0 0 0 1");
			swscanf(rot, L"%f %f %f %f", &inst.rotation.x, &inst.rotation.y, &inst.rotation.z, &inst.rotation.w);

			int params = ini.ReadInteger(buf, L"Params", 0);
			for (int a = 0; a < params; a++)
			{
				wchar_t prm[128] = {};
				wsprintf(prm, L"Param%d", a);
				int value = ini.ReadInteger(buf, prm, 0);
				inst.mh_params.push_back(value);
			}
			Instances.push_back(inst);
		}
		ListData();
		SetActiveIndex(Instances.size() - 1);
		eApp::bRequiresSaving = true;
	}
	else
		MessageBox(eApp::hWindow, L"Nothing to import!", 0, MB_ICONERROR);
}

void INSTEditor::DisplayINSTData()
{
	DWORD dwSel = SendMessage(*hFilesList, LB_GETCURSEL, 0, 0);

	if (dwSel >= 0 && eApp::bIsReady)
	{
		// make wstrings
		std::wstring w_name(Instances[dwSel].name.length(), L' ');
		std::copy(Instances[dwSel].name.begin(), Instances[dwSel].name.end(), w_name.begin());

		std::wstring w_record(Instances[dwSel].recordName.length(), L' ');
		std::copy(Instances[dwSel].recordName.begin(), Instances[dwSel].recordName.end(), w_record.begin());

		std::wstring w_class(Instances[dwSel].className.length(), L' ');
		std::copy(Instances[dwSel].className.begin(), Instances[dwSel].className.end(), w_class.begin());

		SetWindowText(GetDlgItem(eApp::hWindow, INSTANCE_NAME), w_name.c_str());
		SetWindowText(GetDlgItem(eApp::hWindow, RECORD_NAME), w_record.c_str());
		SetWindowText(GetDlgItem(eApp::hWindow, INSTANCE_CLASS), w_class.c_str());


		SetWindowText(GetDlgItem(eApp::hWindow, POS_X), std::to_wstring(Instances[dwSel].position.x).c_str());
		SetWindowText(GetDlgItem(eApp::hWindow, POS_Y), std::to_wstring(Instances[dwSel].position.y).c_str());
		SetWindowText(GetDlgItem(eApp::hWindow, POS_Z), std::to_wstring(Instances[dwSel].position.z).c_str());


		SetWindowText(GetDlgItem(eApp::hWindow, ROT_X), std::to_wstring(Instances[dwSel].rotation.x).c_str());
		SetWindowText(GetDlgItem(eApp::hWindow, ROT_Y), std::to_wstring(Instances[dwSel].rotation.y).c_str());
		SetWindowText(GetDlgItem(eApp::hWindow, ROT_Z), std::to_wstring(Instances[dwSel].rotation.z).c_str());
		SetWindowText(GetDlgItem(eApp::hWindow, ROT_W), std::to_wstring(Instances[dwSel].rotation.w).c_str());

		ListParams(dwSel);

	}


}




std::wstring SetPathFromButton(wchar_t* filter, wchar_t* ext, HWND hWnd)
{
	wchar_t szBuffer[MAX_PATH] = {};
	OPENFILENAME ofn = {};

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = szBuffer;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = ext;
	std::wstring out;
	if (GetOpenFileName(&ofn))
		out = szBuffer;

	return out;
}

std::wstring SetSavePathFromButton(wchar_t* filter, wchar_t* ext, HWND hWnd)
{
	wchar_t szBuffer[MAX_PATH] = {};
	OPENFILENAME ofn = {};

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = szBuffer;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = ext;
	std::wstring out;
	if (GetSaveFileName(&ofn))
		out = szBuffer;

	return out;
}

std::wstring SetSavePathFromButtonWithName(std::wstring name, wchar_t * filter, wchar_t * ext, HWND hWnd)
{
	wchar_t szBuffer[MAX_PATH] = {};
	OPENFILENAME ofn = {};

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = filter;
	wsprintf(szBuffer, name.c_str());
	ofn.lpstrFile = szBuffer;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = ext;
	std::wstring out;
	if (GetSaveFileName(&ofn))
		out = szBuffer;

	return out;
}


std::wstring   SetFolderFromButton(HWND hWnd)
{
	wchar_t szBuffer[MAX_PATH];

	BROWSEINFO bi = {};
	bi.lpszTitle = L"Select Folder";
	bi.hwndOwner = hWnd;
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

	LPITEMIDLIST idl = SHBrowseForFolder(&bi);

	std::wstring out;

	if (idl)
	{
		SHGetPathFromIDList(idl, szBuffer);
		out = szBuffer;

	}

	return out;
}
