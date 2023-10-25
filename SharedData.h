#pragma once
#include <filesystem>
#include <vector>
#include <fstream>

#include <wx/string.h>

#define APP_POS						(wxPoint(100, 50))
#define APP_SIZE					(wxSize(1100, 600))
#define APP_CWD						(std::filesystem::current_path().generic_string())
#define SAVED_DATA_PATH				(APP_CWD + "/MyResources/saved_data.txt")
#define APP_ICON_PATH				(APP_CWD + "/MyResources/app_icon.ico")

//convertion
#define TO_SIZE(p)					(wxSize(p.x, p.y))
#define TO_POINT(p)					(wxPoint(p.x, p.y))

namespace SharedData {
	//number manipulations
	template<typename T> T Min(T a, T b);
	template<typename T> T Max(T a, T b);
	template<typename T> T Clamp(T value, T min, T max);

	//string manipulations
	std::vector<wxString> Split(wxString str, wxString splitBy, int splitCount = -1);
	std::vector<wxString> SplitByStartAndEnd(wxString str, wxString startWith, wxString endWith);
	wxString SplitAndTake(wxString str, wxString splitBy, int takeIndex, int splitCount = -1);
	bool StartWith(wxString str, wxString startWith);
	bool EndWith(wxString str, wxString endWith);
	wxString Join(std::vector<wxString> list, wxString joinBy);

	//file manipulations
	wxString ReadFile(wxString filePath);
	bool WriteFile(wxString filePath, wxString fileContent);
	void EditFileAtLine(wxString filePath, int lineIndex, wxString replaceBy);
}




template<typename T>
T SharedData::Min(T a, T b) {
	return a < b ? a : b;
}
template<typename T>
T SharedData::Max(T a, T b) {
	return a > b ? a : b;
}
template<typename T>
T SharedData::Clamp(T value, T min, T max) {
	return Max(min, Min(value, max));
}

