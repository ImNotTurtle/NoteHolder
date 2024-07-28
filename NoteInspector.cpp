#include "NoteInspector.h"

extern APP_STATE_e e_appState;
extern void SetStatusText(wxString text, int index = 0);

#define DEFAULT_SASH_POSITION_PORTION				(0.8f)
#define DEFAULT_SHOW_LABEL							(wxString("Show"))
#define DEFAULT_HIDE_LABEL							(wxString("Hide"))


#pragma region NoteInspectorItem class
NoteInspector* NoteInspectorItem::s_inspector = NULL;

NoteInspectorItem::NoteInspectorItem(wxWindow* parent) {
	this->Init(parent, NULL);
}

NoteInspectorItem::NoteInspectorItem(wxWindow* parent, NotePanel* panel) {
	this->Init(parent, panel);
}

void NoteInspectorItem::Init(wxWindow* parent, NotePanel* panel){
	wxPanel::Create(parent, -1, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
#pragma region Init
	m_notePanel = panel;
	m_headerText = new wxStaticText(this, -1, "");

#pragma endregion


#pragma region Layout
	wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);
	mainSizer->Add(m_headerText, 1, wxEXPAND | wxTOP | wxBOTTOM | wxLEFT, 5);
	this->SetSizer(mainSizer);
#pragma endregion


#pragma region Binding
	this->Bind(wxEVT_SIZE, &NoteInspectorItem::OnResize, this);

	m_headerText->Bind(wxEVT_LEFT_DOWN, &NoteInspectorItem::OnTextLeftDown, this);
#pragma endregion
}

void NoteInspectorItem::BindNotePanel(NotePanel* panel){
	if (this->HasBinded()) {//unbind before bind the new panel
		this->UnbindNotePanel();
	}
	if (panel != NULL) {
		m_notePanel = panel;
	}
	UpdateLabel();
}
void NoteInspectorItem::UnbindNotePanel(void){
	m_notePanel = NULL;
	m_headerText->SetLabel("");
}
bool NoteInspectorItem::HasBinded(void){
	return m_notePanel != NULL;
}
void NoteInspectorItem::UpdateLabel(void) {
	if (m_notePanel == NULL) {// null reference -> request inspector to delete itself
		this->RequestToDelete();
	}
	else {
		m_headerText->SetLabel(m_notePanel->GetHeaderText());
		this->Layout();
	}
}
void NoteInspectorItem::RequestToDelete(void) {
	if (s_inspector != NULL) {
		s_inspector->DeleteItem(this);
		this->Destroy();
	}
}

void NoteInspectorItem::SetInspector(NoteInspector* inspector){
	s_inspector = inspector;
}
NoteInspector* NoteInspectorItem::GetInspector(void){
	return s_inspector;
}

void NoteInspectorItem::OnResize(wxSizeEvent& evt){
	this->Layout();
}
void NoteInspectorItem::OnTextLeftDown(wxMouseEvent& evt){
	SetStatusText(m_headerText->GetLabel(), 0);
	m_notePanel->RequestFocus();
}

#pragma endregion


NoteInspector::NoteInspector(wxSplitterWindow* parent){
	this->Init(parent, nullptr, nullptr);
}
NoteInspector::NoteInspector(wxSplitterWindow* parent, NoteManager* manager){
	this->Init(parent, manager, nullptr);
}
NoteInspector::NoteInspector(wxSplitterWindow* parent, wxButton* controlButton){
	this->Init(parent, nullptr, controlButton);
}
NoteInspector::NoteInspector(wxSplitterWindow* parent, NoteManager* manager, wxButton* controlButton) {
	this->Init(parent, manager, controlButton);
}

void NoteInspector::Init(wxSplitterWindow* parent, NoteManager* manager, wxButton* controlButton) {
#pragma region Init
	wxScrolled<wxPanel>::Create(parent, -1, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
	this->SetScrollRate(0, 50);
	this->SetVirtualSize(this->GetSize());
	BindButton(controlButton);
	BindManager(manager);

	m_parent = parent;
	m_titlePanel = new wxPanel(this);
	m_displayPanel = new wxPanel(this, -1, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);

	wxStaticText* titleText = new wxStaticText(m_titlePanel, -1, wxString::FromUTF8("Danh sách các ghi chú"));
	titleText->SetWindowStyle(wxALIGN_CENTER);

	this->SetInspectorLabels(DEFAULT_SHOW_LABEL, DEFAULT_HIDE_LABEL);
#pragma endregion


#pragma region Layout
	//m_titlePanel
	{
		wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);
		mainSizer->Add(titleText, wxSizerFlags(0).Center());
		m_titlePanel->SetSizer(mainSizer);
	}

	//m_displayPanel sizer
	{
		wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
		m_displayPanel->SetSizer(mainSizer);
	}
	
	//this sizer
	{
		wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
		mainSizer->Add(m_titlePanel, 0, wxEXPAND);
		mainSizer->Add(m_displayPanel, 9, wxEXPAND | wxALL, 5);
		this->SetSizer(mainSizer);
	}

	

#pragma endregion


#pragma region BindingEvents
	this->Bind(wxEVT_SIZE, &NoteInspector::OnResize, this);

#pragma endregion
}
/**************************************************************************
*							PUBLIC MEMBER								  *
**************************************************************************/
void NoteInspector::ToggleInspector(void) {
	this->ShowInspector(!m_isShown);
}
void NoteInspector::ShowInspector(bool show) {
	auto button = m_controlButton;
	if (button == NULL) return;
	auto tooltip = m_controlButton->GetToolTip();

	m_isShown = show;
	if (show) { // show the inspector
		if (!m_parent->IsSplit()) {
			//split vertically
			if (m_parent->GetSplitMode() == wxSPLIT_VERTICAL) {
				m_parent->SplitVertically(m_parent->GetWindow1(), this);
				//set the sash position as 80% of the parent size
				m_parent->SetSashPosition(m_parent->GetSize().x * DEFAULT_SASH_POSITION_PORTION);
			}
			else {//split horizontally
				m_parent->SplitHorizontally(m_parent->GetWindow1(), this);
				//set the sash position as 80% of the parent size
				m_parent->SetSashPosition(m_parent->GetSize().y * DEFAULT_SASH_POSITION_PORTION);
			}
		}
		m_parent->Layout();
		this->ResetInspector();
		this->Show();
		button->SetLabel(m_hideLabel);
		if (tooltip) {
			tooltip->SetTip("Hide Inspector");
		}
	}
	else {//hide the inspector
		if (m_parent->IsSplit()) {
			m_parent->Unsplit();
		}
		//move the inspector to get out of the parent -> avoiding display error
		this->SetPosition(wxPoint(m_parent->GetSize().x, 0));
		this->Hide();
		button->SetLabel(m_showLabel);
		if (tooltip) {
			tooltip->SetTip("Show Inspector");
		}
	}
	this->Update();
}
void NoteInspector::HideInspector(void) {
	this->ShowInspector(false);
}
bool NoteInspector::IsInspectorShown(void) {
	return m_isShown;
}
void NoteInspector::SetInspectorLabels(wxString showLabel, wxString hideLabel) {
	m_showLabel = showLabel;
	m_hideLabel = hideLabel;
}

void NoteInspector::UpdateInspector(void) {
	for (auto item : m_itemList) {
		item->UpdateLabel();
	}
}
void NoteInspector::ResetInspector(void) {
	auto currentSelectPanel = m_noteManager->GetCurrentSelection();
	if (currentSelectPanel == NULL) return;
	this->ClearItems();
	auto parent = m_displayPanel;
	auto noteList = currentSelectPanel->GetNoteList();
	for (auto note : noteList) {
		auto item = new NoteInspectorItem(parent, note);
		this->AddItem(item);
	}
	this->UpdateInspector();
}

void NoteInspector::BindManager(NoteManager* manager) {
	if (manager == NULL) return;
	m_noteManager = manager;
}
void NoteInspector::BindButton(wxButton* button) {
	if (button == NULL) return;
	m_controlButton = button;
	//add tooltip
	wxToolTip* tooltip = new wxToolTip("Show Inspector");
	m_controlButton->Bind(wxEVT_BUTTON, &NoteInspector::OnControlButtonPress, this);
	m_controlButton->SetToolTip(tooltip);
}

wxButton* NoteInspector::GetControlButton(void){
	return m_controlButton;
}
NoteManager* NoteInspector::GetManager(void){
	return m_noteManager;
}

void NoteInspector::AddItem(NoteInspectorItem* item){
	m_itemList.push_back(item);
	auto panel = m_displayPanel;
	auto sizer = panel->GetSizer();
	if (sizer) {
		sizer->Add(item, 0, wxEXPAND);
	}
	panel->Layout();
}
void NoteInspector::DeleteItem(NoteInspectorItem* item) {
	for (int i = 0; i < m_itemList.size(); i++) {
		if (m_itemList[i] == item) {
			m_itemList.erase(m_itemList.begin() + i);
			auto panel = m_displayPanel;
			auto sizer = panel->GetSizer();
			if (sizer) {
				sizer->Remove(i);
			}
			panel->Layout();
			SetStatusText("touch here", 1);
			break;
		}
	}
}
void NoteInspector::ClearItems(void){
	for (auto item : m_itemList) {
		item->Destroy();
	}
	m_itemList.clear();
}
/**************************************************************************
*							PRIVATE MEMBER								  *
**************************************************************************/
void NoteInspector::OnControlButtonPress(wxCommandEvent& evt) {
	this->ToggleInspector();
}
void NoteInspector::OnResize(wxSizeEvent& evt) {
	if (e_appState == APP_STATE_e::INIT) {
		this->HideInspector();
	}
	this->Layout();
	evt.Skip();
}

/**************************************************************************
*							STATIC MEMBER								  *
**************************************************************************/

