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
	virtual void SetFixedWidth(bool isFixed = true);
	virtual void SetFixedHeight(bool isFixed = true);
	virtual void SetFixedSize(bool fixedWidth = true, bool fixedHeight = true);
	virtual void SetSizeToFitContent(bool fitWidth, bool fitHeight);

	virtual NOTE_TYPE_e GetType(void);
	virtual void EnableScrollbar(int orientation, bool enable = true);

	virtual void ReceiveTabNavigation(void);
	virtual void AddOwnContextMenu(wxWindow* parent, wxMenu* menu);
	virtual void UpdateOwnContextMenu(wxMenu* menu);

	virtual void FromJson(wxString json);
	virtual wxString ToJson(void);

private:
	wxTextCtrl* m_tctrl;

	void PrepareToSendSizeRequest(wxSize newSize);//check for the object is good to send size request

	void OnResize(wxSizeEvent& evt);
	void OnCharHook(wxKeyEvent& evt);
	void OnTextChanged(wxCommandEvent& evt);
	void OnTextPaste(wxClipboardTextEvent& evt);
	
};

