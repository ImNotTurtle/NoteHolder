#include "MainFrame.h"

extern APP_STATE_e e_appState;


#define STRING(s)					(std::to_string(s))
#define POINT(p)					(std::to_string(p.x) + "," + std::to_string(p.y))
#define MIN_ZOOM					0.4f
#define MAX_ZOOM					1.6f

/*
TODO:
*/

MainFrame* MainFrame::instance = NULL;

void SetStatusText(wxString text, int index = 0) {
	MainFrame::GetInstance()->SetStatusText(text, index);
}


MainFrame::MainFrame(wxWindow* parent, int id, const wxString& title, wxPoint pos, wxSize size, long style)
	: wxFrame(parent, id, title, pos, size, style) {
	if (instance == NULL) {
		instance = this;
	}

	#pragma region Init
	wxPanel* framePanel = new wxPanel(this);
	m_toolbar = new wxPanel(framePanel, -1, wxDefaultPosition, wxSize(1, 30));
	{
		
	}
	m_splitWindow = new wxSplitterWindow(framePanel, -1, wxDefaultPosition, 
		wxDefaultSize, wxSP_THIN_SASH | wxSP_LIVE_UPDATE);

	m_noteManager = new NoteManager(m_splitWindow, -1, wxDefaultPosition, wxSize(1, 1));
	m_toggleInspectorButton = new wxButton(m_toolbar, -1, "", wxDefaultPosition, wxSize(30, 30));
	m_inspector = new NoteInspector(m_splitWindow, m_noteManager, m_toggleInspectorButton);
	{
		m_inspector->SetInspectorLabels("<", ">");
	}

	//m_splitWindow
	{
		m_splitWindow->SplitVertically(m_noteManager, m_inspector);
		m_splitWindow->SetMinimumPaneSize(50);
		m_splitWindow->SetSashGravity(1.0f);
	}

	//m_noteManager
	{
		m_noteManager->BindInspector(m_inspector);
	}
	
	m_timer = new wxTimer(this);

	#pragma region SystemComponents
	wxStatusBar* statusBar = new wxStatusBar(this);
	statusBar->SetFieldsCount(3); 


	wxMenuBar* menubar = new wxMenuBar();
	wxMenu* fileMenu = new wxMenu();
	fileMenu->Append(wxID_NEW, wxT("&New\tCtrl + N"));
	fileMenu->Append(wxID_OPEN, wxT("&Open\tCtrl + O"));
	fileMenu->Append(wxID_SAVE, wxT("&Save\tCtrl + S"));
	fileMenu->Append(wxID_SAVEAS, wxT("&Save As...\tCtrl + Shift + S"));
	fileMenu->Append(wxID_EXIT, wxT("&Quit"));

	wxMenu* helpMenu = new wxMenu();
	helpMenu->Append(wxID_HELP, wxT("&How to use"));
	helpMenu->Append(wxID_ABOUT, wxT("&About"));

	menubar->Append(fileMenu, wxT("&File"));
	menubar->Append(helpMenu, wxT("&Help"));


	Bind(wxEVT_MENU, &MainFrame::OnNew, this, wxID_NEW);
	Bind(wxEVT_MENU, &MainFrame::OnOpen, this, wxID_OPEN);
	Bind(wxEVT_MENU, &MainFrame::OnSave, this, wxID_SAVE);
	Bind(wxEVT_MENU, &MainFrame::OnSaveAs, this, wxID_SAVEAS);
	Bind(wxEVT_MENU, &MainFrame::OnHelp, this, wxID_HELP);
	Bind(wxEVT_MENU, &MainFrame::OnQuit, this, wxID_EXIT);
	Bind(wxEVT_MENU, &MainFrame::OnAbout, this, wxID_ABOUT);

	//accelerator table
	const int entryCount = 4;
	wxAcceleratorEntry entries[entryCount];
	entries[0].Set(wxACCEL_CTRL, (int)('N'), wxID_NEW);
	entries[1].Set(wxACCEL_CTRL, (int)('O'), wxID_OPEN);
	entries[2].Set(wxACCEL_CTRL, (int)('S'), wxID_SAVE);
	entries[3].Set(wxACCEL_CTRL | wxACCEL_SHIFT, (int)('S'), wxID_SAVEAS);
	
	wxAcceleratorTable accel(entryCount, entries);

	#pragma endregion
	#pragma endregion


	#pragma region Layout
	wxBoxSizer* frameSizer = new wxBoxSizer(wxVERTICAL);
	frameSizer->Add(m_toolbar, 0, wxEXPAND);
	frameSizer->Add(m_splitWindow, 1, wxEXPAND);
	framePanel->SetSizer(frameSizer);

	//m_toolbar
	{
		wxBoxSizer* toolbarSizer = new wxBoxSizer(wxHORIZONTAL);
		toolbarSizer->AddStretchSpacer(1);
		toolbarSizer->Add(m_toggleInspectorButton, wxSizerFlags(0).Center());
		m_toolbar->SetSizer(toolbarSizer);
	}
	#pragma endregion


	#pragma region BindingEvents
	this->Bind(wxEVT_TIMER, &MainFrame::OnTimer, this);
	this->Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnClose, this);
	this->Bind(wxEVT_SIZE, &MainFrame::OnResize, this);
	#pragma endregion


	this->SetStatusBar(statusBar);
	this->SetMenuBar(menubar);
	this->SetAcceleratorTable(accel);
	if(wxFile::Exists(APP_ICON_PATH)) {
		wxIcon icon(APP_ICON_PATH, wxBITMAP_TYPE_ICO);
		if (icon.IsOk()) {
			this->SetIcons(icon);
		}
	}
	

	ProgramStart();
}

/**************************************************************************
*							PRIVATE MEMBER								  *
**************************************************************************/
void MainFrame::ProgramStart(void) {
	m_timer->Start(1000);//start the timer
	//-------------------open the lastest working file-------------------//
	wxString fileContent = SharedData::ReadFile(SAVED_DATA_PATH);
	if (fileContent == "") {//no saved content, do nothing
		return;
	}
	auto dataList = SharedData::Split(fileContent, "\n");
	//note manager takes the first 2 line
	m_noteManager->FromJson(SharedData::Join(dataList, "\n", 0, 1));
}
void MainFrame::ProgramQuit(void) {
	//let the manager decides to quit
	if (m_noteManager->OnQuit()) {
		wxString fileContent = m_noteManager->ToJson();
		SharedData::WriteFile(SAVED_DATA_PATH, fileContent);
		this->Destroy();
	}
}
#pragma region Events
void MainFrame::OnTimer(wxTimerEvent& evt) {

}
void MainFrame::OnClose(wxCloseEvent& evt) {
	e_appState = APP_STATE_e::CLOSE;
	ProgramQuit();
}
void MainFrame::OnResize(wxSizeEvent& evt) {
	auto window = m_splitWindow;
	evt.Skip();
}


#pragma region MenuEvents
void MainFrame::OnNew(wxCommandEvent& evt){
	m_noteManager->CreateNotePanel();
}
void MainFrame::OnOpen(wxCommandEvent& evt){
	OpenFile();
}
void MainFrame::OnSave(wxCommandEvent& evt){
	SaveFile();
}
void MainFrame::OnSaveAs(wxCommandEvent& evt){
	SaveFile(true);
}
void MainFrame::OnQuit(wxCommandEvent& evt){
	ProgramQuit();
}
void MainFrame::OnHelp(wxCommandEvent& evt) {
	wxMessageBox("This is how to use");
}
void MainFrame::OnAbout(wxCommandEvent& evt){
	wxMessageBox("About message");
}
#pragma endregion


#pragma endregion

/**************************************************************************
*							PUBILC MEMBER								  *
**************************************************************************/
//this function save the current selected or a specific panel, ask user for save as needed and popup save dialog if that is a unlocated panel
//return the choice of the message box
int MainFrame::SaveFile(bool saveAs, bool askForSave, int index) {
	if (askForSave) {//ask the user to save or not
		int ret = wxMessageBox(wxString::FromUTF8("Bạn có muốn lưu file này không?"), wxString::FromUTF8("Xác nhận"), wxYES_NO | wxCANCEL);
		if (ret != wxYES) return ret;
	}
	
	//determine which panel to save by index
	NoteHolderPanel* panel;
	{
		if (index == -1 || index < 0 || index > m_noteManager->GetPanelList().size()) {
			panel = m_noteManager->GetCurrentSelection();
		}
		else {
			panel = m_noteManager->GetNotePanelAt(index);
		}
	}
	
	int statusIndex = 0;
	wxString savePath = panel->GetFilePath();
	//empty file location or the user ask to save as new file, open file dialog to choose location
	{
		if (panel->GetFilePath() == "" || saveAs) {
			wxFileDialog* dialog = new wxFileDialog(this, wxString::FromUTF8("Chọn đường dẫn để lưu file"), APP_CWD, "", "*.txt", wxFD_SAVE);
			int ret = dialog->ShowModal();
			if (ret == wxID_OK) {
				if (!saveAs) { // only change the panel path when user ask to save, not save as
					panel->SetFilePath(dialog->GetPath());
				}
				savePath = dialog->GetPath();
			}
			else {
				return wxCANCEL;
			}
			dialog->Destroy();
		}
	}
	//export content to path
	{
		if (panel->ExportToFile(savePath)) {
			SetStatusText(wxString::FromUTF8("Lưu file thành công."), statusIndex);
		}
		else {
			SetStatusText(wxString::FromUTF8("Lưu file thất bại."), statusIndex);
		}
		if (saveAs) {//open that save as file
			this->OpenFile(false, savePath);
		}
	}
	//erase status after 5 seconds
	{
		std::thread thread = std::thread{ [this, statusIndex]() {
		std::this_thread::sleep_for(std::chrono::seconds(5));
		SetStatusText("", statusIndex);
	} };
		thread.detach();
	}
	return wxYES;
}
void MainFrame::OpenFile(bool popupDialog, wxString filePath) {
	//modify the filepath parameter when the user need to choose the file location to open
	if (popupDialog) {
		wxFileDialog* open = new wxFileDialog(this, wxString::FromUTF8("Chọn file để mở"), APP_CWD, "", "*.txt", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
		if (open->ShowModal() == wxID_OK) {
			filePath = open->GetPath();
		}
		open->Destroy();
	}
	
	//open the file at filepath location
	{
		//check if the path selected is opening now
		bool noteOpening = false;
		auto panelList = m_noteManager->GetPanelList();
		for (int i = 0; i < panelList.size(); i++) {
			if (panelList[i]->GetFilePath() == filePath) {//the file is opening, set that panel to be select
				m_noteManager->SetSelection(i);
				noteOpening = true;
				break;
			}
		}
		if (!noteOpening) { // if the file is not openning then open that file
			m_noteManager->CreateNotePanel();
			m_noteManager->GetCurrentSelection()->ImportFromFile(filePath);
		}
	}
}
/**************************************************************************
*							STATIC MEMBER								  *
**************************************************************************/
MainFrame* MainFrame::GetInstance(void) {
	return instance;
}