#include "NoteManager.h"

#define DEFAULT_NEW_NOTE_NAME				("Untitled-")


extern void SetStatusText(wxString text, int index = 0);
/*
* PENDING: Open File in browser : done
*		  - Show absolute location as tooltip on notebook tab (no needed)
*		  - Fix notebook out of range when application start up

*/

NoteManager::NoteManager(wxWindow* parent, int id, wxPoint pos, wxSize size) {
	wxNotebook::Create(parent, id, pos, size);
	BuildContextMenu();
	this->Bind(wxEVT_CONTEXT_MENU, &NoteManager::OnContextMenu, this);
	this->Bind(wxEVT_RIGHT_DOWN, &NoteManager::OnRightClick, this);
}
/**************************************************************************
*							PRIVATE MEMBER								  *
**************************************************************************/
void NoteManager::BuildContextMenu(void) {
	m_contextMenu = new wxMenu();

	auto createLeft = m_contextMenu->Append(wxID_ANY, wxString::FromUTF8("Thêm tab mới vào bên trái"));
	auto createRight = m_contextMenu->Append(wxID_ANY, wxString::FromUTF8("Thêm tab mới vào bên phải"));
	auto open = m_contextMenu->Append(wxID_ANY, wxString::FromUTF8("Mở file \t Ctrl + O"));
	m_contextMenu->AppendSeparator();
	auto closeNote = m_contextMenu->Append(wxID_ANY, wxString::FromUTF8("Đóng tab này \t Ctrl + W"));
	auto closeLeft = m_contextMenu->Append(wxID_ANY, wxString::FromUTF8("Đóng các tab ở bên trái"));
	auto closeRight = m_contextMenu->Append(wxID_ANY, wxString::FromUTF8("Đóng các tab ở bên phải"));
	m_contextMenu->AppendSeparator();
	auto openInBrower = m_contextMenu->Append(wxID_ANY, wxString::FromUTF8("Mở thư mục"));


	//accelerator table
	{
		wxAcceleratorEntry entries[2];
		entries[0].Set(wxACCEL_CTRL, (int)'O', open->GetId());
		entries[1].Set(wxACCEL_CTRL, (int)'W', closeNote->GetId());
		wxAcceleratorTable table(2, entries);
		this->SetAcceleratorTable(table);
	}

	//binding
	{
		//create left
		this->Bind(wxEVT_MENU, [this](wxCommandEvent& evt) {
			int index = HitTest(m_mousePos);
			this->CreateNotePanel(index);
			this->SetSelection(index);
			}, createLeft->GetId());
		//create right
		this->Bind(wxEVT_MENU, [this](wxCommandEvent& evt) {
			int index = HitTest(m_mousePos) + 1;
			MainFrame::GetInstance()->SetStatusText(std::to_string(index), 1);
			this->CreateNotePanel(index);
			this->SetSelection(index);
			}, createRight->GetId());
		//open file
		this->Bind(wxEVT_MENU, [this](wxCommandEvent& evt) {
			MainFrame::GetInstance()->OpenFile();
			}, open->GetId());
		//close note
		this->Bind(wxEVT_MENU, [this](wxCommandEvent& evt) {
			auto notePanel = GetCurrentSelection();
			int ret = MainFrame::GetInstance()->SaveFile(false, notePanel->NeedingASave(), this->GetSelection());
			if (ret == wxYES || ret == wxNO) {
				this->DeleteSelection();
			}

			}, closeNote->GetId());
		//close right
		this->Bind(wxEVT_MENU, [this](wxCommandEvent& evt) {
			int index = HitTest(m_mousePos);
			int deleteCount = m_panelList.size() - 1 - index;
			for (int i = 1; i <= deleteCount; i++) {
				this->DeleteNotePanel(index + 1);
			}
			}, closeRight->GetId());
		//close left
		this->Bind(wxEVT_MENU, [this](wxCommandEvent& evt) {
			int index = HitTest(m_mousePos);
			int deleteCount = index;
			for (int i = 0; i < deleteCount; i++) {
				this->DeleteNotePanel(0);
			}
			}, closeLeft->GetId());
		//open in browser
		this->Bind(wxEVT_MENU, [this](wxCommandEvent& evt) {
			int index = this->HitTest(m_mousePos);
			auto noteHolder = this->GetNotePanelAt(index);
			if (noteHolder->GetFilePath() == "") {// empty file path to open
				//open default path
				wxLaunchDefaultBrowser("C:");
			}
			else {
				wxLaunchDefaultBrowser(SharedData::GetParentPath(noteHolder->GetFilePath()));
			}
			}, openInBrower->GetId());
	}
}

#pragma region Events
void NoteManager::OnRightClick(wxMouseEvent& evt) {
	m_mousePos = evt.GetPosition();
	evt.Skip();
}
void NoteManager::OnContextMenu(wxContextMenuEvent& evt) {
	auto clientPos = evt.GetPosition() == wxDefaultPosition ?
		(wxPoint(this->GetSize().GetWidth() / 2, this->GetSize().GetHeight() / 2))
		: this->ScreenToClient(evt.GetPosition());

	this->PopupMenu(m_contextMenu, clientPos);
}
#pragma endregion
/**************************************************************************
*							PUBIC MEMBER								  *
**************************************************************************/
void NoteManager::CreateNotePanel(int index) {
	static int createCount = 0;//avoid the same name for the new note
	wxString newPanelName;
	NoteHolderPanel* note = new NoteHolderPanel(this, DEFAULT_NEW_NOTE_NAME + std::to_string(++createCount) + ".txt");
	this->AddNotePanel(note, true, index);
}
void NoteManager::AddNotePanel(NoteHolderPanel* note, bool select, int index) {
	if (note->GetManager() == NULL) { // grant itself to be the manager
		note->SetManager(this);
	}
	if (index == -1) {
		this->AddPage(note, note->GetPanelName(), select);
		m_panelList.push_back(note);
	}
	else {
		this->InsertPage(index, note, note->GetPanelName(), select);
		m_panelList.insert(m_panelList.begin() + index, note);
	}
}
void NoteManager::DeleteSelection(void) {
	int index = this->GetSelection();
	DeleteNotePanel(index);
}
void NoteManager::DeleteNotePanel(int index) {
	if (index < 0 || index >= m_panelList.size()) return;
	this->DeletePage(index);
	m_panelList.erase(m_panelList.begin() + index);
}

NoteHolderPanel* NoteManager::GetCurrentSelection(void) {
	return m_panelList[this->GetSelection()];
}
NoteHolderPanel* NoteManager::GetNotePanelAt(int index) {
	return m_panelList[index];
}
std::vector<NoteHolderPanel*> NoteManager::GetPanelList(void) {
	return m_panelList;
}

bool NoteManager::OnQuit(void) {
	//------------create a dialog and ask user to choose files to save
	wxDialog* dialog = new wxDialog(this, -1, wxString::FromUTF8("Lưu File trước khi thoát"), wxDefaultPosition, wxDefaultSize,
		wxCLOSE_BOX | wxCAPTION);

	std::vector<wxCheckBox*> cbList;//store checkboxes to check for which file to save later

	//-------------prepare dialog content
	std::vector<NoteHolderPanel*> savePanelList;//saving note panels which are needing a save
	for (int i = 0; i < m_panelList.size(); i++) {
		if (m_panelList[i]->NeedingASave()) {
			savePanelList.push_back(m_panelList[i]);
		}
	}
	if (savePanelList.size() == 0) {//zero file needs to save 
		return true;// then quit
	}

	dialog->SetSize(wxSize(300, 45 + 60 + 30 * savePanelList.size()));

#pragma region Init
	wxPanel* contentPanel = new wxPanel(dialog, -1, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
	wxButton* yesButton = new wxButton(dialog, -1, wxString::FromUTF8("Có"), wxDefaultPosition, wxDefaultSize);
	wxButton* noButton = new wxButton(dialog, -1, wxString::FromUTF8("Không"), wxDefaultPosition, wxDefaultSize);

	wxBoxSizer* mainSizerV = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* contentSizerV = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* buttonSizerH = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* topLabelSizerH = new wxBoxSizer(wxHORIZONTAL);

#pragma endregion


#pragma region Layout
	mainSizerV->AddStretchSpacer(4);
	//top label sizer
	{
		topLabelSizerH->Add(new wxStaticText(dialog, -1, wxString::FromUTF8("Danh sách các file chưa lưu")), 4);
		topLabelSizerH->Add(new wxStaticText(dialog, -1, wxString::FromUTF8("Lưu hay không")), 1);
	}
	mainSizerV->Add(topLabelSizerH, 1, wxEXPAND | wxLEFT | wxRIGHT, 15);
	mainSizerV->AddStretchSpacer(2);
	//content sizer
	{
		for (int i = 0; i < savePanelList.size(); i++) {
			wxBoxSizer* rowSizerH = new wxBoxSizer(wxHORIZONTAL);
			wxPanel* rowPanel = new wxPanel(contentPanel);
			wxStaticText* text = new wxStaticText(rowPanel, -1, savePanelList[i]->GetPanelName());
			wxCheckBox* cb = new wxCheckBox(rowPanel, -1, "");

			text->SetFont(wxFontInfo(11).Underlined().Light());
			text->Bind(wxEVT_LEFT_DOWN, [this, savePanelList, i](wxMouseEvent& evt) {
					int index = this->FindPage(savePanelList[i]);
					this->SetSelection(index);
				});
			text->Bind(wxEVT_MOTION, [this](wxMouseEvent& evt) {
				wxSetCursor(wxCURSOR_HAND);
				});
			text->Bind(wxEVT_LEAVE_WINDOW, [this](wxMouseEvent& evt) {
				wxSetCursor(wxCURSOR_DEFAULT);
				});
			if (savePanelList[i]->GetFilePath() != "") {// only checked if the panel has its path already
				cb->SetValue(true); 
			}
			else {//popup the save dialog to ask user choose save location
				cb->Bind(wxEVT_CHECKBOX, [=](wxCommandEvent& evt) {
					wxFileDialog* save = new wxFileDialog(this, wxString::FromUTF8("Chọn file để save"), APP_CWD, "", "*.txt", wxFD_SAVE | wxSTAY_ON_TOP);
					if (save->ShowModal() == wxID_OK) {
						savePanelList[i]->SetFilePath(save->GetPath());
						text->SetLabel(savePanelList[i]->GetPanelName());
					}
					save->Destroy();
					});
			}
			cbList.push_back(cb);
			rowSizerH->Add(text, wxSizerFlags(4).CenterVertical());
			rowSizerH->Add(cb, wxSizerFlags(1).CenterVertical());
			rowPanel->SetSizer(rowSizerH);

			contentSizerV->Add(rowPanel, 1, wxEXPAND | wxLEFT | wxRIGHT, 10);
		}
		contentPanel->SetSizer(contentSizerV);
	}
	mainSizerV->Add(contentPanel, 16, wxEXPAND | wxLEFT | wxRIGHT, 15);
	mainSizerV->AddStretchSpacer(3);
	//button sizer
	{
		buttonSizerH->Add(yesButton, 3, wxEXPAND);
		buttonSizerH->AddStretchSpacer(3);
		buttonSizerH->Add(noButton, 3, wxEXPAND);
	}
	mainSizerV->Add(buttonSizerH, 3, wxEXPAND | wxLEFT | wxRIGHT, 20);
	mainSizerV->AddStretchSpacer(1);

	dialog->SetSizer(mainSizerV);
#pragma endregion


#pragma region BindingEvents
	yesButton->Bind(wxEVT_BUTTON, [=](wxCommandEvent& evt) {
		dialog->EndModal(wxID_OK);
		});

	noButton->Bind(wxEVT_BUTTON, [=](wxCommandEvent& evt) {
		dialog->EndModal(wxID_NO);
		});
#pragma endregion


	dialog->Center();
	dialog->Layout();
	int ret = dialog->ShowModal();
	if (ret == wxID_OK) {
		//get indexes which checkbox is checked
		for (int i = 0; i < cbList.size(); i++) {
			if (cbList[i]->IsChecked()) {
				auto panel = savePanelList[i];
				panel->ExportToFile(panel->GetFilePath());
			}
		}
		return true;
	}
	if (ret == wxID_NO) return true;//not save but quit still
	return false;//dont quit
}
/**************************************************************************
*							STATIC MEMBER								  *
**************************************************************************/