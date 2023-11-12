#pragma once
#include <wx/wx.h>

enum NOTE_TYPE_e {
	NOT_SET,
	TEXT,
	LIST
};

class NotePanel;

class NotePad : public wxWindow
{//base class for variants of pad: textpad, listpad
public:
	NotePad(NotePanel* parent);

	virtual void SetColor(wxColour color);
	virtual void SetContentFontSize(int size);

	virtual NOTE_TYPE_e GetType(void);

	virtual void ReceiveTabNavigation(void);


	virtual void FromJson(wxString json);
	virtual wxString ToJson(void);

	static NOTE_TYPE_e GetType(wxString text);
	static wxString GetTypeString(NOTE_TYPE_e type);

protected:
	NotePanel* m_parent;
};

#include "NotePanel_TextPad.h"
#include "NotePanel_ListPad.h"

#include "NotePanel.h"
