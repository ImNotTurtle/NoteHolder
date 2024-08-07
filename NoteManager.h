#pragma once
#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/notebook.h>
#include <vector>
#include <filesystem>
#include <fstream>

#include "NoteHolderPanel.h"
#include "SharedData.h"

class NoteInspector;
//a note book that contains all the note holder panels
class NoteManager : public wxNotebook
{
public:
	NoteManager(wxWindow* parent, int id = -1, wxPoint pos = wxDefaultPosition, wxSize size = wxDefaultSize);

	void CreateNotePanel(int index = -1);
	void AddNotePanel(NoteHolderPanel* note, bool select = false, int index = -1);
	
	void DeleteSelection(void);
	void DeleteNotePanel(int index);

	void BindInspector(NoteInspector* inspector);
	void RequestInspectorUpdate(void);
	void RequestInspectorReset(void);
	

	NoteHolderPanel* GetCurrentSelection(void);
	NoteHolderPanel* GetNotePanelAt(int index);
	std::vector<NoteHolderPanel*> GetPanelList(void);

	void FromJson(wxString json);
	wxString ToJson(void);

	bool OnQuit(void);//return true if the program should quit
	
private:
	std::vector<NoteHolderPanel*> m_panelList;
	wxMenu* m_contextMenu;
	wxPoint m_mousePos;//save mouse position to find tab index by hittest
	NoteInspector* m_inspector;

	void BuildContextMenu(void);

	void OnResize(wxSizeEvent& evt);
	void OnRightClick(wxMouseEvent& evt);
	void OnContextMenu(wxContextMenuEvent& evt);
	void OnSelectionChanged(wxBookCtrlEvent& evt);

};

#include "NoteInspector.h"