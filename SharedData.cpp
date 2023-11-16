#include "SharedData.h"

extern void SetStatusText(wxString text, int index = 0);


#pragma region NumberManipulations



#pragma endregion


#pragma region StringManipulations
wxVector<wxString> SharedData::Split(wxString str, wxString splitBy, int splitCount) {
    wxVector<wxString> retList;
    wxString accumStr;
    wxString tempStr;
    int splitIndex = 0;
    int curSplitCount = 0;
    if (splitCount == -1) {
        splitCount = (int)(str.length());//if no split count provided, make it largest as possible 
    }
    //if the split by is null, split the list into list of character
    if (splitBy == "") {
        for (auto x : str) {
            retList.push_back(x);
        }
    }
    else {//split by is not null
        for (int i = 0; i < str.length(); i++) {
            //if the string start to match with the split by string
            if (str[i] == splitBy[splitIndex] && curSplitCount < splitCount) {
                splitIndex++;
                tempStr += str[i];
                //if the string fully match with the split by string
                if (splitIndex == splitBy.length()) {
                    //add accum str to the list and reset
                    splitIndex = 0;
                        curSplitCount++;
                        if (accumStr != "") retList.push_back(accumStr);
                        accumStr = "";
                        tempStr = "";
                }
            }
            else {
                //if the string does not match with the split by string
                if (splitIndex != 0) {//if the string nearly match with the split by string before
                    //add them to the accum string
                    accumStr += tempStr;
                    tempStr = "";
                    splitIndex = 0;
                }

                else accumStr += str[i];
            }
        }
        if (accumStr != "") {
            if (tempStr != "") accumStr += tempStr;
            retList.push_back(accumStr);
        }
    }
    
    return retList;
}
wxVector<wxString> SharedData::SplitByStartAndEnd(wxString str, wxString startWith, wxString endWith) {
    wxVector<wxString> retList;

    int startIndex = 0, endIndex = 0;
    bool startDetect = false, endDetect = false;
    wxString accumStr = "";// accum is string sum
    wxString tempStr = "";// temp save string in case current character match with some character but not all in split string
    for (int i = 0; i < str.length(); i++) {
        if (startDetect == false) {
            if (str[i] == startWith[startIndex]) {
                startIndex++;
                tempStr += str[i];
                if (startIndex == startWith.length()) {
                    startDetect = true;
                    tempStr = "";
                    continue;
                }
            }
            else {
                startIndex = 0;
                accumStr += tempStr;
                tempStr = "";
            }
        }

        if (startDetect == true) {
            if (str[i] == endWith[endIndex]) {
                endIndex++;
                tempStr += str[i];
                if (endIndex == endWith.length()) {
                    endDetect = true;
                    tempStr = "";
                }
            }
            else {//end string no matching
                if (endIndex != 0) {//if end string has been match with some characters before 
                    accumStr += tempStr;
                    tempStr = "";
                }
                else {
                    accumStr += str[i];
                }
                endIndex = 0;
            }
        }

        if (endDetect == true) {
            retList.push_back(accumStr);
            accumStr = "";
            tempStr = "";
            startIndex = 0;
            endIndex = 0;
            startDetect = false;
            endDetect = false;
        }
    }

    if (accumStr != "") {
        if (tempStr != "") {
            accumStr += tempStr;
        }
        retList.push_back(accumStr);
    }

    return retList;
}
wxString SharedData::SplitAndTake(wxString str, wxString splitBy, int takeIndex, int splitCount) {
    auto list = SharedData::Split(str, splitBy, splitCount);
    if (list.size() == 0) return "";
    if (list.size() == 1) return list[0];
    takeIndex %= (int)(list.size()); //clamp the size
    if (takeIndex < 0) {
        takeIndex = list.size() + takeIndex;
    }

    return list[takeIndex];
}
bool SharedData::StartWith(wxString str, wxString startWith) {
    int i = 0;
    while (i < startWith.length() && startWith[i] == str[i]) {
        i++;
    }
    return i == startWith.length();
}
bool SharedData::EndWith(wxString str, wxString endWith) {
    int xIndex = (int)(endWith.length() - 1);
    int i = (int)(str.length() - 1);
    while (xIndex >= 0 && endWith[xIndex] == str[i]) {
        xIndex--;
        i--;
    }
    return xIndex == -1;
}
wxString SharedData::Join(wxVector<wxString> list, wxString joinBy, int startIndex, int endIndex) {
    if (startIndex >= list.size()) return "";

    endIndex = SharedData::Clamp(endIndex, -1, (int)(list.size() - 1));
    if (endIndex == -1) endIndex = list.size() - 1;
    
    wxString retStr = "";
    for (int i = startIndex; i <= endIndex; i++) {
        retStr += list[i] + joinBy;
    }
    return retStr;
}
#pragma endregion

#pragma region FileManipulations
wxString SharedData::ReadFile(wxString filePath){
    wxFile file(filePath, wxFile::read);
    if (!file.IsOpened()) return "";
    wxString fileContent;
    if (file.ReadAll(&fileContent)) {
        return fileContent;
    }
    return "";
}
bool SharedData::WriteFile(wxString filePath, wxString fileContent) {
    wxFile file(filePath, wxFile::write);
    if (!file.IsOpened()) return false;
    file.Write(fileContent);
    return true;
}
void SharedData::EditFileAtLine(wxString filePath, int lineIndex, wxString replaceBy) {
    wxString fileContent = SharedData::ReadFile(filePath);
    auto list = SharedData::Split(fileContent, "\n");
    if (list.size() <= lineIndex) return;//index not found
    list[lineIndex] = replaceBy;
    SharedData::WriteFile(filePath, SharedData::Join(list, "\n"));
}

wxString SharedData::GetParentPath(wxString filePath) {
    wxString parentPath;
    int index = filePath.size() - 1;
    while (index >= 0 && filePath[index] != '\\') {
        index--;
    }
    for (int i = 0; i < index; i++) {
        parentPath += filePath[i];
    }
    return parentPath;
}
wxString SharedData::ConvertToStdPath(wxString filePath) {
    wxString retPath;
    for (auto i : filePath) {
        if (i == '/') {
            retPath += "\\";
        }
        else {
            retPath += i;
        }
    }
    return retPath;
}

#pragma endregion
