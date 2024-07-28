#pragma once
#include <wx/wx.h>
#include <wx/splitter.h>

#include "SharedData.h"

class NotePanel;
class NoteManager;
class NoteInspector;

class NoteInspectorItem : public wxPanel {
public:
	NoteInspectorItem(wxWindow* parent);
	NoteInspectorItem(wxWindow* parent, NotePanel* panel);

	void BindNotePanel(NotePanel* panel);
	void UnbindNotePanel(void);
	bool HasBinded(void);
	void UpdateLabel(void);
	void RequestToDelete(void);

	static void SetInspector(NoteInspector* inspector);
	static NoteInspector* GetInspector(void);

private:
	static NoteInspector* s_inspector;
	
	NotePanel* m_notePanel;
	wxStaticText* m_headerText;

	void Init(wxWindow* parent, NotePanel* panel);

	void OnResize(wxSizeEvent& evt);
	void OnTextLeftDown(wxMouseEvent& evt);
};

class NoteInspector : public wxScrolled<wxPanel>
{
public:
	NoteInspector(wxSplitterWindow* parent);
	NoteInspector(wxSplitterWindow* parent, NoteManager* manager);
	NoteInspector(wxSplitterWindow* parent, wxButton* controlButton);
	NoteInspector(wxSplitterWindow* parent,  NoteManager* manager, wxButton* controlButton);


	void ToggleInspector(void);
	void ShowInspector(bool show = true);
	void HideInspector(void);
	bool IsInspectorShown(void);
	void UpdateInspector(void); // updating the label only
	void ResetInspector(void); // erasing the old inspector items and load the new items 
	void SetInspectorLabels(wxString showLabel, wxString hideLabel);

	void BindManager(NoteManager* manager);
	void BindButton(wxButton* button);

	wxButton* GetControlButton(void);
	NoteManager* GetManager(void);

	void AddItem(NoteInspectorItem* item);
	void DeleteItem(NoteInspectorItem* item);
	void ClearItems(void);



private:

	wxSplitterWindow* m_parent;
	bool m_isShow;//is the inspector showed on the screen
	wxButton* m_controlButton;//control button for toggle the inspector
	wxPanel* m_titlePanel;
	wxPanel* m_displayPanel;//contains the information of the current selection of the note holder panel
	NoteManager* m_noteManager;
	wxVector<NoteInspectorItem*> m_itemList;
	wxString m_showLabel;
	wxString m_hideLabel;
	

	void Init(wxSplitterWindow* parent, NoteManager* manager, wxButton* controlButton);

	void OnControlButtonPress(wxCommandEvent& evt);
	void OnResize(wxSizeEvent& evt);
};

#include "NotePanel.h"
#include "NoteHolderPanel.h"
#include "NoteManager.h"
//#include "MainFrame.h"