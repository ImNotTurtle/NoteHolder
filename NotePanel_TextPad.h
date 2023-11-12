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

	virtual void ReceiveTabNavigation(void);


	virtual void FromJson(wxString json);
	virtual wxString ToJson(void);
	void SetSizeToFitContent(bool fitWidth, bool fitHeight);

private:
	bool m_rightDown;
	wxTextCtrl* m_tctrl;
	wxMenu* m_contextMenu;
	bool m_isFixedWidth;
	bool m_isFixedHeight;

	void BuildContextMenu(void);
	void PrepareToSendSizeRequest(wxSize newSize);//check for the object is good to send size request
	void SetFixedWidth(bool isFixed = true);
	void SetFixedHeight(bool isFixed = true);
	void SetFixedSize(bool fixedWidth = true, bool fixedHeight = true);
	void EnableScrollbar(int direction, bool enable = true);//direction = wxVERTICAL | wxHORIZONTAL

	void OnResize(wxSizeEvent& evt);
	void OnContextMenu(wxContextMenuEvent& evt);
	void OnCharHook(wxKeyEvent& evt);
	void OnTextChanged(wxCommandEvent& evt);
	void OnTextPaste(wxClipboardTextEvent& evt);
	void OnRightDown(wxMouseEvent& evt);
	
	void OnMenuUpdateUI(wxUpdateUIEvent& evt);
};

