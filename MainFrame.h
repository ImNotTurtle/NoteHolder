#pragma once
#include <wx/wx.h>
#include <wx/file.h>
#include <wx/notebook.h>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <wx/splitter.h>

#include "SharedData.h"
#include "NotePanel.h"
#include "NoteHolderPanel.h"
#include "NoteManager.h"
#include "NoteInspector.h"



class MainFrame : public wxFrame
{
public:
	MainFrame(wxWindow* parent, int id, const wxString& title, wxPoint pos = wxDefaultPosition, wxSize size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE);

	int SaveFile(bool saveAs = false, bool askForSave = false, int index = -1);//popup and ask the user to save file
	void OpenFile(bool popupDialog = true, wxString filePath = "");//open dialog to choose file

	static MainFrame* GetInstance(void);

private:
	static MainFrame* instance;
	wxSplitterWindow* m_splitWindow;
	wxPanel* m_toolbar;
	NoteManager* m_noteManager; // this is a notebook panel which handles keyboard accelerator
	NoteInspector* m_inspector;
	wxButton* m_toggleInspectorButton;
	wxTimer* m_timer;

	void ProgramStart(void);
	
	void ProgramQuit(void);

	void OnTimer(wxTimerEvent& evt);
	void OnClose(wxCloseEvent& evt);
	void OnResize(wxSizeEvent& evt);


	//menu events
	void OnNew(wxCommandEvent& evt);
	void OnOpen(wxCommandEvent& evt);
	void OnSave(wxCommandEvent& evt);
	void OnSaveAs(wxCommandEvent& evt);
	void OnQuit(wxCommandEvent& evt);
	void OnHelp(wxCommandEvent& evt);
	void OnAbout(wxCommandEvent& evt);

	
};