#pragma once
#include <Windows.h>	// for GetModuleFileNameA(), ...
#include <string>
#include <vector>
#include <chrono>

using namespace std;

class IniFile
{
	string Path;
	string EXE;

private:
	wstring str2wstr1(const string& s)
	{
#ifdef _WINDOWS
		std::string curLocale = setlocale(LC_ALL, "");
		const char* _Source = s.c_str();

		wchar_t *_Dest = new wchar_t[s.length() + 1];
		wmemset(_Dest, 0, s.length() + 1);
		size_t outSize;
		mbstowcs_s(&outSize, _Dest, s.length() + 1, _Source, s.length());
		std::wstring result = _Dest;
		delete[]_Dest;
		setlocale(LC_ALL, curLocale.c_str());
		return result;
#elif __APPLE__
		wstring wstr(s.begin(), s.end());
		return wstr;
#endif
	}

	vector<string> split(string strWhole)
	{
		vector<string> vecStr;
		string strSingle;
		string printerName;

		std::size_t found;
		do {
			found = strWhole.find_first_of(" ");
			strSingle = strWhole.substr(0, found);
			strWhole = strWhole.substr(found + 1);

			if (!strSingle.empty())
				vecStr.push_back(strSingle);

		} while (!strWhole.empty() && found != std::string::npos);
		return vecStr;
	}

public:
	IniFile(string IniFilename)	// Don't need to include full path for the ini file. The ini file will be found in the directory where the application is loaded.
	{
		char szPathFilename[MAX_PATH];
		GetModuleFileNameA(nullptr, szPathFilename, MAX_PATH);
		string strPathFilename(szPathFilename);

		size_t found = strPathFilename.find_last_of("\\");
		string strPath = strPathFilename.substr(0, found);

		Path = strPath + "\\" + IniFilename;
	}

	bool IniFileExists()
	{
		if (FILE* file = fopen(Path.c_str(), "r")) {
			fclose(file);
			return true;
		}
		else {
			return false;
		}
	}

	string ReadStr(string Key, string Section)
	{
		char RetVal[255 + 1];
		GetPrivateProfileStringA(Section.c_str(), Key.c_str(), "", RetVal, 255, Path.c_str());
		return RetVal;
	}

	wstring ReadWStr(wstring Key, wstring Section)
	{
		wchar_t RetVal[255 + 1];
		wstring WPath = str2wstr1(Path);
		GetPrivateProfileStringW(Section.c_str(), Key.c_str(), L"", RetVal, 255, WPath.c_str());
		return RetVal;
	}

	float ReadFloat(string Key, string Section)
	{
		char RetVal[255 + 1];
		GetPrivateProfileStringA(Section.c_str(), Key.c_str(), "0", RetVal, 255, Path.c_str());
		return stof(RetVal);
	}

	int ReadInt(string Key, string Section)
	{
		char RetVal[255 + 1];
		GetPrivateProfileStringA(Section.c_str(), Key.c_str(), "0", RetVal, 255, Path.c_str());

		if (strcmp(RetVal, "") == 0)
			return 0;
		else
			return stoi(RetVal);
	}

	bool ReadFloatXYZ(string Key, string Section, float array[])
	{
		char RetVal[255 + 1];
		GetPrivateProfileStringA(Section.c_str(), Key.c_str(), "0", RetVal, 255, Path.c_str());
		vector<string> strArray = split(RetVal);

		if (strArray.size() >= 3)
		{
			array[0] = stof(strArray[0].c_str());
			array[1] = stof(strArray[1].c_str());
			array[2] = stof(strArray[2].c_str());
			return true;
		}
		else
			return false;
	}

	bool ReadDoubleXYZ(string Key, string Section, double array[])
	{
		char RetVal[255 + 1];
		GetPrivateProfileStringA(Section.c_str(), Key.c_str(), "0", RetVal, 255, Path.c_str());
		vector<string> strArray = split(RetVal);

		if (strArray.size() >= 3)
		{
			array[0] = stod(strArray[0].c_str());
			array[1] = stod(strArray[1].c_str());
			array[2] = stod(strArray[2].c_str());
			return true;
		}
		else
			return false;
	}
};