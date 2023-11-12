#include "NotePanel_NotePad.h"


NotePad::NotePad(NotePanel* parent)
	: wxWindow(parent, -1){
	m_parent = parent;
}

/**************************************************************************
*							PUBLIC MEMBER								  *
**************************************************************************/
#pragma region Public member
void NotePad::FromJson(wxString json) {

}
wxString NotePad::ToJson(void) {
	return "NotePad";
}

NOTE_TYPE_e NotePad::GetType(void) {
	return NOT_SET;
}
void NotePad::ReceiveTabNavigation(void) {

}

void NotePad::SetColor(wxColour color) {
	this->SetBackgroundColour(color);
}
void NotePad::SetContentFontSize(int size) {
	this->SetFont(wxFontInfo(size).Family(wxFONTFAMILY_DEFAULT).Light());
}
#pragma endregion
/**************************************************************************
*							PRIVATE MEMBER								  *
**************************************************************************/
#pragma region Private member

#pragma endregion
/**************************************************************************
*							STATIC MEMBER								  *
**************************************************************************/
#pragma region Static member
NOTE_TYPE_e NotePad::GetType(wxString text) {
	if (text == "notset") {
		return NOT_SET;
	}
	if (text == "text") {
		return TEXT;
	}
	if (text == "list") {
		return LIST;
	}
	return NOT_SET;
}
wxString NotePad::GetTypeString(NOTE_TYPE_e type) {
	if (type == NOT_SET) return "notset";
	if (type == TEXT) return "text";
	if (type == LIST) return "list";
	return "notset";
}
#pragma endregion
