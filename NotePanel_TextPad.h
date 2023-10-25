#pragma once
#include <wx/wx.h>
#include <wx/tokenzr.h>
#include <wx/clipbrd.h>

#include "NotePanel_NotePad.h"
#include "SharedData.h"

class TextPad : public NotePad {
public:
	TextPad(NotePanel* parent);

	virtual void SetColor(wxColour color);
	virtual void SetContentFontSize(int size);

	virtual NOTE_TYPE_e GetType(void);

	virtual void FromJson(wxString json);
	virtual wxString ToJson(void);
private:
	bool m_rightDown;
	wxTextCtrl* m_tctrl;
	wxMenu* m_contextMenu;
	void BuildContextMenu(void);

	void OnContextMenu(wxContextMenuEvent& evt);
	void OnCharHook(wxKeyEvent& evt);
	void OnTextChanged(wxCommandEvent& evt);
	void OnTextPaste(wxClipboardTextEvent& evt);
	void OnRightDown(wxMouseEvent& evt);
};

