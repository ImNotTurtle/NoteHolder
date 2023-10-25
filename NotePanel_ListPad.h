#pragma once
#include <wx/wx.h>
#include <wx/grid.h>

#include "NotePanel_NotePad.h"

class ListPad : public NotePad {
public:
	ListPad(NotePanel* parent);

	virtual void SetColor(wxColour color);
	virtual void SetContentFontSize(int size);

	virtual NOTE_TYPE_e GetType(void);

	virtual void FromJson(wxString json);
	virtual wxString ToJson(void);
private:
	wxGrid* m_grid;
};

