#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>
#include <fstream>

enum eGameMode {
	MODE_MANHUNT = 1,
	MODE_MANHUNT2
};

enum eParamTabs {
	TAB_NAME,
	TAB_TYPE,
	TAB_VALUE,
	TOTAL_TABS
};


#define INST_EDITOR_VERSION L"1.0"
#define INST_PAD_VALUE 4


struct vector3d {
	float x, y, z;
};

struct quaternion3d {
	float x, y, z, w;
};

struct mh2_param_entry {
	unsigned int hash;
	char		 type[4];
	unsigned int value;
};


struct instance_entry {
	std::string name;
	std::string recordName;
	std::string className;

	vector3d	 position;
	quaternion3d rotation;

	std::vector<int> mh_params;
	std::vector<mh2_param_entry> mh2_params;

};


class INSTEditor {
private:
	HWND* hLogBox;
	HWND* hFilesList;
	HWND* hGroupBox;
	HWND* hParams;
public:
	std::ifstream pFile;
	std::wstring InputPath;
	std::wstring OutputPath;

	eGameMode m_gameMode;

	std::vector<instance_entry> Instances;

	void         Init(HWND* log, HWND* list, HWND* box);

	void CreateNewFile(std::wstring input, eGameMode game);
	void OpenFile(std::wstring input, eGameMode game);
	void ReadFile();
	void Save();
	void SaveAs();
	void Reset();
	void Close();

	void ListData();
	void ListParams(DWORD id);
	void SetActiveIndex(int id);
	void DisplayINSTData();
	void AddInstance();
	void DeleteInstance();
	void ApplyChanges();

	void ExportINI();
	void ImportINI();

};


std::wstring   SetPathFromButton(wchar_t* filter, wchar_t* ext, HWND hWnd);
std::wstring   SetSavePathFromButton(wchar_t* filter, wchar_t* ext, HWND hWnd);
std::wstring   SetSavePathFromButtonWithName(std::wstring name, wchar_t* filter, wchar_t* ext, HWND hWnd);
std::wstring   SetFolderFromButton(HWND hWnd);
