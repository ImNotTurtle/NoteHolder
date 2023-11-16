#include "NotePanel_NotePad.h"

extern void SetStatusText(wxString text, int index = 0);

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



bool NotePad::IsFixedWidth(void) {
	return m_isFixedWidth;
}
bool NotePad::IsFixedHeight(void) {
	return m_isFixedHeight;
}
void NotePad::EnableScrollbar(int orientation, bool enable) {
	if(enable){
		this->SetScrollbar(orientation, 0, 20, 50);
	}
	else{
		this->SetScrollbar(orientation, 0, 0, 0);
	}
}

void NotePad::ReceiveTabNavigation(void) {

}
void NotePad::AddOwnContextMenu(wxWindow* parent, wxMenu* menu) {
	SetStatusText("goes here");
}
void NotePad::UpdateOwnContextMenu(wxMenu* menu) {
	SetStatusText("goes here", 1);
}

void NotePad::SetColor(wxColour color) {
	this->SetBackgroundColour(color);
}
void NotePad::SetContentFontSize(int size) {
	this->SetFont(wxFontInfo(size).Family(wxFONTFAMILY_DEFAULT).Light());
}
void NotePad::SetFixedSize(bool fixedWidth, bool fixedHeight) {
	this->SetFixedWidth(fixedWidth);
	this->SetFixedHeight(fixedHeight);
}
void NotePad::SetFixedWidth(bool isFixed){

}
void NotePad::SetFixedHeight(bool isFixed){

}
void NotePad::SetSizeToFitContent(bool fitWidth, bool fitHeight) {

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
