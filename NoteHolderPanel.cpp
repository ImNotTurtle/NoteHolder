#include "NoteHolderPanel.h"

extern void SetStatusText(wxString text, int index = 0);

#define MIN_ZOOM					0.4f
#define DEFAULT_ZOOM				1.0f
#define MAX_ZOOM					1.8f

NoteManager* NoteHolderPanel::s_manager = NULL;


NoteHolderPanel::NoteHolderPanel(wxWindow* parent, wxString panelName) {
	Init(parent, panelName);
}

void NoteHolderPanel::Init(wxWindow* parent, wxString panelName) {
	wxScrolled<wxPanel>::Create(parent, -1, wxDefaultPosition, wxSize(1, 1), wxHSCROLL | wxVSCROLL);

	m_zoomFactor = DEFAULT_ZOOM;
	m_panelName = panelName;
	m_filePath = "";
	m_rightDown = false;
	BuildContextMenu();
	this->SetMinSize(wxSize(100, 100));
	this->SetVirtualSize(wxSize(4000, 4000));
	this->SetDoubleBuffered(true);
	this->EnableScrolling(true, true);
	this->SetScrollbars(40, 40, 100, 100);

#pragma region Init

#pragma endregion


#pragma region Layout

#pragma endregion


#pragma region BindingEvents
	this->Bind(wxEVT_MOUSEWHEEL, &NoteHolderPanel::OnPanelScroll, this);
	this->Bind(wxEVT_LEFT_DOWN, &NoteHolderPanel::OnPanelMouseLeftDown, this);
	this->Bind(wxEVT_LEFT_UP, &NoteHolderPanel::OnPanelMouseLeftUp, this);
	this->Bind(wxEVT_MOTION, &NoteHolderPanel::OnPanelMouseMove, this);
	this->Bind(wxEVT_PAINT, &NoteHolderPanel::OnPanelPaint, this);
	this->Bind(wxEVT_SIZE, &NoteHolderPanel::OnPanelResize, this);
	this->Bind(wxEVT_CONTEXT_MENU, &NoteHolderPanel::OnContextMenuEvent, this);
	this->Bind(wxEVT_RIGHT_DOWN, &NoteHolderPanel::OnPanelMouseRightDown, this);
#pragma endregion


}
/**************************************************************************
*							PRIVATE MEMBER								  *
**************************************************************************/
void NoteHolderPanel::BuildContextMenu(void) {
	m_contextMenu = new wxMenu();
	wxMenu* addSubMenu = new wxMenu();

	auto add = m_contextMenu->AppendSubMenu(addSubMenu, wxString::FromUTF8("Thêm ghi chú"));
	auto save = m_contextMenu->Append(wxID_ANY, wxString::FromUTF8("Lưu file\tCtrl + S"));
	auto clear = m_contextMenu->Append(wxID_ANY, wxString::FromUTF8("Xóa tất cả ghi chú"));
	auto miniAll = m_contextMenu->Append(wxID_ANY, wxString::FromUTF8("Thu nhỏ tất cả ghi chú"));
	auto unMiniAll = m_contextMenu->Append(wxID_ANY, wxString::FromUTF8("Phóng to tất cả ghi chú"));
	auto resetZoom = m_contextMenu->Append(wxID_ANY, wxString::FromUTF8("Đặt tỉ lệ thu phóng về mặc định\tCtrl + R"));

	//add sub menu
	auto addTextPad = addSubMenu->Append(wxID_ANY, wxString::FromUTF8("Dạng văn bản"));
	auto addListPad = addSubMenu->Append(wxID_ANY, wxString::FromUTF8("Dạng danh sách"));

	wxAcceleratorEntry entries[2];
	entries[0].Set(wxACCEL_CTRL, (int)'S', save->GetId());
	entries[1].Set(wxACCEL_CTRL, (int)'R', resetZoom->GetId());
	wxAcceleratorTable table(2, entries);
	this->SetAcceleratorTable(table);

	this->Bind(wxEVT_MENU, [this](wxCommandEvent& evt) {
		this->CreateNote(TEXT);
		}, addTextPad->GetId());
	this->Bind(wxEVT_MENU, [this](wxCommandEvent& evt) {
		this->CreateNote(LIST);
		}, addListPad->GetId());
	this->Bind(wxEVT_MENU, [this](wxCommandEvent& evt) {
		MainFrame::GetInstance()->SaveFile(false, false);
		}, save->GetId());
	this->Bind(wxEVT_MENU, [this](wxCommandEvent& evt) {
		int ret = wxMessageBox(wxString::FromUTF8("Bạn muốn xóa hết các ghi chú?"), wxString::FromUTF8("Xác nhận"), wxYES_NO);
		if (ret == wxYES) {
			this->ClearAllNotes();
		}
		}, clear->GetId());
	this->Bind(wxEVT_MENU, [this](wxCommandEvent& evt) {
		for (auto panel : m_noteList) {
			panel->SetMinimize(true);
		}
		}, miniAll->GetId());
	this->Bind(wxEVT_MENU, [this](wxCommandEvent& evt) {
		for (auto panel : m_noteList) {
			panel->SetMinimize(false);
		}
		}, unMiniAll->GetId());
	this->Bind(wxEVT_MENU, [this](wxCommandEvent& evt) {
		OnZoom(0, DEFAULT_ZOOM);
		}, resetZoom->GetId());
}

#pragma region Events
void NoteHolderPanel::OnPanelResize(wxSizeEvent& evt) {
	auto panel = this;
	panel->Refresh();
	evt.Skip();
}
void NoteHolderPanel::OnPanelScroll(wxMouseEvent& evt) {
	OnScroll(evt.GetWheelRotation(), evt.ControlDown(), evt.ShiftDown());
}
void NoteHolderPanel::OnPanelMouseMove(wxMouseEvent& evt) {
	auto panel = this;
	if (evt.LeftIsDown() && evt.Dragging() && panel->HasCapture()) {
		//calc delta
		wxPoint deltaPos = evt.GetPosition() - m_captureMousePos;
		deltaPos = deltaPos / 15;
		panel->Scroll(m_captureViewStart - deltaPos);
		panel->Refresh();
	}
}
void NoteHolderPanel::OnPanelMouseLeftDown(wxMouseEvent& evt) {
	auto panel = this;
	panel->SetFocus();
	panel->CaptureMouse();
	m_captureMousePos = evt.GetPosition();
	m_captureViewStart = panel->GetViewStart();
}
void NoteHolderPanel::OnPanelMouseLeftUp(wxMouseEvent& evt) {
	auto panel = this;
	if (panel->HasCapture()) {
		panel->ReleaseMouse();
	}
}
void NoteHolderPanel::OnPanelPaint(wxPaintEvent& evt) {
	auto panel = this;
	static wxSize panelVirtualSize = panel->GetVirtualSize();
	wxPaintDC dc(panel);
	panel->SetVirtualSize(panelVirtualSize * m_zoomFactor);
	panel->DoPrepareDC(dc);

	wxSize panelSize = panel->GetVirtualSize();
	int hGridCount = 50;
	int vGridCount = 50;
	int gridWidth = panelSize.x / vGridCount;
	int gridHeight = panelSize.y / hGridCount;
	dc.SetPen(wxColour(87, 82, 81));
	//draw vertical grid
	for (int i = 0; i <= vGridCount; i++) {
		dc.DrawLine(wxPoint(i * gridWidth, 0), wxPoint(i * gridWidth, panelSize.y));
	}
	//draw horizontal grid
	for (int i = 0; i <= hGridCount; i++) {
		dc.DrawLine(wxPoint(0, i * gridHeight), wxPoint(panelSize.x, i * gridHeight));
	}

	//update every note position and size
	UpdateNotePanel();
}
void NoteHolderPanel::OnPanelMouseRightDown(wxMouseEvent& evt) {
	m_rightDown = true;
	evt.Skip();
}
void NoteHolderPanel::OnContextMenuEvent(wxContextMenuEvent& evt) {//popup menu when user right click
	if (m_rightDown) {
		auto clientPos = evt.GetPosition() == wxDefaultPosition ?
			(wxPoint(this->GetSize().GetWidth() / 2, this->GetSize().GetHeight() / 2))
			: this->ScreenToClient(evt.GetPosition());

		this->PopupMenu(m_contextMenu, clientPos);
		m_rightDown = false;
	}
}
#pragma endregion
/**************************************************************************
*							PUBIC MEMBER								  *
**************************************************************************/
void NoteHolderPanel::CreateNote(NOTE_TYPE_e type){
	SetFileSaveState(true);
	auto panel = this;
	wxSize panelSize = DEFAULT_NOTE_PANEL_SIZE;
	wxPoint panelPos = TO_POINT((panel->GetClientSize() / 2 - panelSize / 2));//center the panel

	NotePanel* note = new NotePanel(panel, type);
	note->SetOriginRect(wxRect(panel->CalcUnscrolledPosition(panelPos) * (1.0f / m_zoomFactor), panelSize));
	note->Raise();
	AddNote(note);
}
void NoteHolderPanel::AddNote(NotePanel* note) {
	m_noteList.push_back(note);
	this->Refresh();
}
void NoteHolderPanel::DeleteNote(NotePanel* note){
	for (int i = 0; i < m_noteList.size(); i++) {
		if (m_noteList[i] == note) {
			m_noteList.erase(m_noteList.begin() + i);
			return;
		}
	}
}
void NoteHolderPanel::ClearAllNotes(void) {
	if (m_noteList.size() == 0) return;
	SetFileSaveState(true);
	for (int i = 0; i < m_noteList.size(); i++) {
		delete(m_noteList[i]);
	}
	m_noteList.resize(0);
	this->Refresh();
}

void NoteHolderPanel::SetZoomFactor(float zoomFactor){
	m_zoomFactor = SharedData::Clamp(zoomFactor, MIN_ZOOM, MAX_ZOOM);
}
void NoteHolderPanel::SetPanelName(wxString name) {//set panel name and display it on notebook tab
	m_panelName = name;
	int pageIndex = GetManager()->FindPage(this);
	GetManager()->SetPageText(pageIndex, GetPanelName());
}
void NoteHolderPanel::SetFilePath(wxString filePath) {
	m_filePath = filePath;
	//extract and take the file name to be the panel name
	SetPanelName(SharedData::SplitAndTake(filePath, "\\", -1));//get the file name in path
}

float NoteHolderPanel::GetZoomFactor(void){
	return m_zoomFactor;
}
wxString NoteHolderPanel::GetPanelName(void) {
	return m_panelName;
}
wxString NoteHolderPanel::GetFilePath(void) {
	return m_filePath;
}

void NoteHolderPanel::UpdateNotePanel(void){
	if (m_noteList.size() == 0) return;
	for (int i = 0; i < m_noteList.size(); i++) {
		m_noteList[i]->UpdateOnZoom();
	}
}
void NoteHolderPanel::ImportFromFile(wxString filePath){
	wxString fileContent = SharedData::ReadFile(filePath);
	FromJson(fileContent);
	SetFilePath(filePath);
}
bool NoteHolderPanel::ExportToFile(wxString filePath){
	if (filePath == "") return false;
	SetFileSaveState(false);
	
	return SharedData::WriteFile(filePath, ToJson());
}

void NoteHolderPanel::FromJson(wxString json){
	auto noteList = SharedData::SplitByStartAndEnd(json, "{", "},");
	if (noteList.size() == 0) return;
	for (int i = 0; i < noteList.size(); i++) {
		if (noteList[i] == "") continue;//skip empty json
		//convention: [type, pos, size, header, content]
		//take the type out
		auto jsonList = SharedData::SplitByStartAndEnd(noteList[i], "[", "]");
		
		if (jsonList.size() == 0) continue;
		auto type = jsonList[0];
		NotePanel* note = new NotePanel(this, TEXT);
		note->FromJson(noteList[i]);
		this->AddNote(note);
	}
}
wxString NoteHolderPanel::ToJson(void){
	wxString retStr;
	for (int i = 0; i < m_noteList.size(); i++) {
		retStr += "" + m_noteList[i]->ToJson() + "\n";
	}
	return retStr;
}

void NoteHolderPanel::OnChildChanged(void) {
	SetFileSaveState(true);
}
void NoteHolderPanel::SetFileSaveState(bool needToSave) {
	if (needToSave) {
		if (!SharedData::EndWith(GetPanelName(), "*")) {// not end with *
			SetPanelName(GetPanelName() + "*");
		}
	}
	else {
		if (SharedData::EndWith(GetPanelName(), "*")) { // end with *
			SetPanelName(m_panelName.RemoveLast(1));
		}
	}
}
bool NoteHolderPanel::NeedingASave(void) {
	return SharedData::EndWith(GetPanelName(), "*")//has been modified
		|| this->GetFilePath() == ""; // or no file path to save
}

void NoteHolderPanel::OnScroll(int wheelRotation, bool ctrlDown, bool shiftDown) {//public scroll for children to access
	auto panel = this;
	int scrollStep = 5;
	int scrollRotation = wheelRotation < 0 ? 1 : -1;
	if (!ctrlDown) {//not zooming
		if (!shiftDown) {//vscroll
			panel->Scroll(panel->GetViewStart() + wxPoint(0, 1) * scrollStep * scrollRotation);
		}
		else {//hscroll
			panel->Scroll(panel->GetViewStart() + wxPoint(1, 0) * scrollStep * scrollRotation);
		}
		panel->Refresh();
	}
	else {//zooming
		OnZoom(- scrollRotation);
	}
}
void NoteHolderPanel::OnZoom(int scrollRotation, float zoomFactor) {//passing zoomFactor to jump to that zoom scale, passing 0 to zoom normally
	if (zoomFactor != 0.0f) {
		m_zoomFactor = 1.0f;
	}
	else {
		float zoomStep = 0.1f;
		m_zoomFactor += zoomStep * scrollRotation;
		m_zoomFactor = SharedData::Clamp(m_zoomFactor, MIN_ZOOM, MAX_ZOOM);//decrease when zoom out, increase when zoom in
	}
	this->Refresh();
}
/**************************************************************************
*							STATIC MEMBER								  *
**************************************************************************/
void NoteHolderPanel::SetManager(NoteManager* manager) {
	s_manager = manager;
}
NoteManager* NoteHolderPanel::GetManager(void) {
	return s_manager;
}