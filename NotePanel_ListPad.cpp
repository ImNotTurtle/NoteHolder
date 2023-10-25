#include "NotePanel_ListPad.h"


ListPad::ListPad(NotePanel* parent)
	: NotePad(parent){
#pragma region Init
	m_parent = parent;
	m_grid = new wxGrid(this, -1);
	m_grid->CreateGrid(2, 2);
#pragma endregion


#pragma region Layout
	wxBoxSizer* mainSizerV = new wxBoxSizer(wxVERTICAL);
	mainSizerV->Add(m_grid, 1, wxEXPAND);
	this->SetSizer(mainSizerV);


	this->Layout();
#pragma endregion


#pragma region BindingEvent

#pragma endregion
}

/**************************************************************************
*							PUBLIC MEMBER								  *
**************************************************************************/
#pragma region Public member
void ListPad::SetColor(wxColour color) {
	this->SetBackgroundColour(color);
	m_grid->SetBackgroundColour(color);
}
void ListPad::SetContentFontSize(int size) {
	m_grid->SetFont(wxFontInfo(size).Family(wxFONTFAMILY_DEFAULT).Light());
}

NOTE_TYPE_e ListPad::GetType(void) {
	return LIST;
}


void ListPad::FromJson(wxString json) {

}
wxString ListPad::ToJson(void) {
	return "ListPad";
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

#pragma endregion
