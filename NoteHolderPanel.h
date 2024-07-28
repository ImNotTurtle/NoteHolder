#pragma once
#include <wx/wx.h>
#include <wx/popupwin.h>

#include <vector>

#include "NotePanel.h"
#include "SharedData.h"

class NoteManager;

//a class / a page that contains all the note panels
class NoteHolderPanel : public wxScrolled<wxPanel>
{
public:
	NoteHolderPanel(wxWindow* parent, wxString panelName = "");

	void CreateNote(NOTE_TYPE_e type);
	void AddNote(NotePanel* note);
	void DeleteNote(NotePanel* note);
	void ClearAllNotes(void);
	void UpdateNotePanel(void);

	void SetZoomFactor(float zoomFactor);
	void SetPanelName(wxString name);
	void SetFilePath(wxString filePath);
	void SetViewPosition(wxPoint pos);
	//void Set

	float GetZoomFactor(void);
	wxVector<NotePanel*> GetNoteList(void);
	wxString GetPanelName(void);
	wxString GetFilePath(void);
	wxVector<wxString> GetNoteHeaderList(void);

	void FromJson(wxString json);
	wxString ToJson(void);
	void ImportFromFile(wxString filePath);
	bool ExportToFile(wxString filePath);

	void OnChildChanged(void);//handle when child is moved
	void RequestInspectorUpdate(void);
	void RequestInspectorReset(void);
	void SetFileSaveState(bool needToSave);//set the file status to need to save or not
	bool NeedingASave(void);

	void OnScroll(int wheelRotation, int wheelDelta, bool ctrlDown, bool shiftDown);
	void OnZoom(int scrollRotation, float zoomFactor = 0.0f);//passing zoomFactor = 0 to performs normal zoom, jump to that zoomFactor otherwise
	
	static void SetManager(NoteManager* manager);
	static NoteManager* GetManager(void);
private:
	static NoteManager* s_manager;
	wxString m_filePath;//file path to its content
	wxString m_panelName;//the name to display on manager tab
	float m_zoomFactor;
	wxPoint m_captureMousePos;
	wxPoint m_captureViewStart;
	wxVector<NotePanel*> m_noteList;
	wxMenu* m_contextMenu;
	bool m_rightDown;

	void Init(wxWindow* parent, wxString panelName);
	void BuildContextMenu(void);

	void OnPanelScroll(wxMouseEvent& evt);
	void OnPanelMouseMove(wxMouseEvent& evt);
	void OnPanelMouseLeftDown(wxMouseEvent& evt);
	void OnPanelMouseLeftUp(wxMouseEvent& evt);
	void OnPanelPaint(wxPaintEvent& evt);
	void OnPanelResize(wxSizeEvent& evt);
	void OnPanelMouseRightDown(wxMouseEvent& evt);
	void OnContextMenuEvent(wxContextMenuEvent& evt);
};

#include "NoteManager.h"
#include "MainFrame.h"