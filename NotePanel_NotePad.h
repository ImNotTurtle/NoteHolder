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

	virtual void SetFixedSize(bool fixedWidth = true, bool fixedHeight = true);
	virtual void SetFixedWidth(bool isFixed = true);
	virtual void SetFixedHeight(bool isFixed = true);
	virtual void EnableScrollbar(int orientation, bool enable = true);
	virtual void SetSizeToFitContent(bool fitWidth = true, bool fitHeight = true);

	virtual NOTE_TYPE_e GetType(void);

	virtual void AddOwnContextMenu(wxWindow* parent, wxMenu* menu);
	virtual void UpdateOwnContextMenu(wxMenu* menu);

	virtual void ReceiveTabNavigation(void);

	virtual void FromJson(wxString json);
	virtual wxString ToJson(void);

	bool IsFixedWidth(void);
	bool IsFixedHeight(void);

	static NOTE_TYPE_e GetType(wxString text);
	static wxString GetTypeString(NOTE_TYPE_e type);

protected:
	NotePanel* m_parent;
	bool m_isFixedWidth;
	bool m_isFixedHeight;

};

#include "NotePanel_TextPad.h"
#include "NotePanel_ListPad.h"

#include "NotePanel.h"
