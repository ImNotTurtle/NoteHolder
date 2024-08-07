#pragma once
#include <wx/wx.h>
#include <wx/tokenzr.h>
#include <wx/clipbrd.h>
#include <wx/msgdlg.h>

#include <string>
#include "NotePanel_NotePad.h"

#define DEFAULT_NOTE_PANEL_SIZE					(wxSize(250, 300))
#define NOTE_PANEL_MIN_SIZE						(wxSize(150, 60))

enum RESIZE_DIRECTION {
	NONE,
	LEFT,
	BOTTOMLEFT,
	BOTTOM,
	BOTTOMRIGHT,
	RIGHT
};

class NoteHolderPanel;

class NotePanel : public wxPanel
{
public:
	NotePanel(NoteHolderPanel* parent, NOTE_TYPE_e type = NOT_SET);

	bool operator == (NotePanel* other);
	bool operator != (NotePanel* other);

	void SetColor(wxColour color);
	void SetDragPanelColor(wxColour color);
	void SetFontScale(float scalar);
	void SetOriginRect(wxRect rect);
	void SetHeaderFontSize(int size);
	void SetContentFontSize(int size);
	void SetHeaderText(wxString text);
	void SetContentText(wxString text);
	void SetMinimize(bool minimized);
	void ResetToDefaultSize(bool resetWidth, bool resetHeight);
	void RequestFocus(void);//send request to parent to move the view position so that the user can see the note

	wxRect& GetOriginRect(void);
	NotePad* GetNotePad(void);
	wxString GetHeaderText(void);
	wxString GetContentText(void);

	void UpdateSize(void);
	void UpdateOnZoom(void);//take action when zoom in or out

	//child events
	void OnChildSizeReport(wxSize childSize);
	void OnChildChanged(void);

	void OnClose(void);

	void FromJson(wxString json);
	wxString ToJson(void);
private:
	//UI things
	NoteHolderPanel* m_parent;
	wxPanel* m_dragPanel;
	wxTextCtrl* m_headerTC;
	NotePad* m_notepad;
	wxRect m_originRect;
	wxButton* m_miniButton;
	wxButton* m_closeButton;

	int m_headerFontSize;
	int m_contentFontSize;
	wxColour m_minimizeHoverColor;//hover color for minimize button
	wxColour m_closeHoverColor;//hover color for close button
	bool m_rightDown;
	wxMenu* m_contextMenu; // right click menu

	//private usage for resizing panel
	wxPoint m_prevRectPos;//save rect position when left down
	wxSize m_prevRectSize;//save rect size when left down
	wxPoint m_prevMousePos;//save mouse position when left down
	wxPoint m_prevPanelPos;
	wxSize m_prevPanelSize;
	RESIZE_DIRECTION m_resizeDirection;

	wxSize m_originSize;//save size for un-minimizing
	bool m_isMinimized;

	//global usage for all notes in parent when dragging notes
	static wxPoint s_delta;//mouse offset from topleft when left down
	static wxPoint s_panelPos;//panel position when left down
	static wxPoint s_rectPos;//hold origin rect position when left is down

	void Init(NoteHolderPanel* parent, NOTE_TYPE_e type);
	void CreateNotePad(NOTE_TYPE_e type);
	RESIZE_DIRECTION GetCurrentResizeDirection(wxPoint mousePos);
	void BindingEventRecursive(wxWindow* window);
	wxSize GetChildSizeDiff(void); // get the size different between m_notepad and the note its self 
	void SetNoteState(wxString state);
	wxString GetNoteState(void); // return the note state
	void BuildContextMenu(void);
	

	//on drag panel
	void OnDragPanelMouseMove(wxMouseEvent& evt);
	void OnDragPanelMouseLeftDown(wxMouseEvent& evt);
	void OnDragPanelMouseLeftUp(wxMouseEvent& evt);
	void OnDragPanelMouseLeave(wxMouseEvent& evt); 
	//on textctrl
	void OnHeaderTextChanged(wxCommandEvent& evt);
	void OnHeaderCharHook(wxKeyEvent& evt);
	//on this ( resizing )
	void OnResize(wxSizeEvent& evt);
	void OnMouseMove(wxMouseEvent& evt);
	void OnMouseLeave(wxMouseEvent& evt);
	void OnMouseLeftDown(wxMouseEvent& evt);
	void OnMouseRightDown(wxMouseEvent& evt);
	void OnMouseLeftUp(wxMouseEvent& evt);
	void OnContextMenu(wxContextMenuEvent& evt);
	void OnMenuUpdateUI(wxUpdateUIEvent& evt);

	void OnMinimizeButtonMouseEnter(wxMouseEvent& evt);
	void OnMinimizeButtonMouseLeave(wxMouseEvent& evt);
	void OnMinimizeButtonMouseLeftDown(wxMouseEvent& evt);
	void OnMinimizeButtonPaint(wxPaintEvent& evt);

	void OnCloseButtonMouseEnter(wxMouseEvent& evt);
	void OnCloseButtonMouseLeave(wxMouseEvent& evt);
	void OnCloseButtonMouseLeftDown(wxMouseEvent& evt);
	void OnCloseButtonPaint(wxPaintEvent& evt);

	//recursive events
	void OnFocus(wxFocusEvent& evt);
	void OnMouseScroll(wxMouseEvent& evt);
};

#include "NoteManager.h"
#include "NoteHolderPanel.h"