#include "NotePanel.h"

extern void SetStatusText(wxString text, int index = 0);

/******************************************************************
# Features: resizable, raise on top when received focus,

# Note: CHILD_MOVED on text changed, mouse dragging, resizing, delete

*******************************************************************/
#define BORDER_SIZE					(3)
#define MIN_SIZE					(wxSize(200, 100))
#define NUMBER(x)					(std::stoi(x))
#define STRING(x)					(std::to_string(x))
#define PAIR(p)						(std::to_string(p.x) + "," + std::to_string(p.y))

#define DEFAULT_HOVER_COLOR			(wxColour(204, 208, 230))
#define DEFAULT_HEADER_FONT_SIZE	(14)
#define DEFAULT_HEADER_FONT			(wxFontInfo(DEFAULT_HEADER_FONT_SIZE))
#define DEFAULT_CONTENT_FONT_SIZE	(7)
#define DEFAULT_CONTENT_FONT		(wxFontInfo(DEFAULT_CONTENT_FONT_SIZE))
#define DEFAULT_PANEL_COLOR			(wxColour(180, 197, 240))
#define DEFAULT_DRAG_PANEL_COLOR	(wxColour(124, 157, 242))

#define CHILD_CHANGED				(m_parent->OnChildChanged())

wxPoint NotePanel::s_delta = wxPoint(0, 0);
wxPoint NotePanel::s_panelPos = wxPoint(0, 0);
wxPoint NotePanel::s_rectPos = wxPoint(0, 0);

////private data
static wxSize ClampMinSize(wxSize size, wxSize min) {
	if (size.x < min.x) size.x = min.x;
	if (size.y < min.y) size.y = min.y;
	return size;
}
static int GetLineMaxWidth(wxWindow* window, wxString content) {
	int maxWidth = 0;
	wxStringTokenizer tokenizer(content, "\n");
	while (tokenizer.HasMoreTokens()) {
		wxString line = tokenizer.GetNextToken();
		int width = window->GetTextExtent(line).x;
		maxWidth = std::max(maxWidth, width);
	}
	return maxWidth;
}

NotePanel::NotePanel(NoteHolderPanel* parent, NOTE_TYPE_e type){
	Init(parent, type);
}
void NotePanel::Init(NoteHolderPanel* parent, NOTE_TYPE_e type) {
	wxPanel::Create(parent, -1, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
	m_parent = parent;
	m_resizeDirection = NONE;
	m_headerFontSize = DEFAULT_HEADER_FONT_SIZE;
	m_contentFontSize = DEFAULT_CONTENT_FONT_SIZE;
	m_minimizeHoverColor = DEFAULT_HOVER_COLOR;
	m_closeHoverColor = DEFAULT_HOVER_COLOR;
	m_isMinimized = false;
	m_rightDown = false;


#pragma region Init
	m_dragPanel = new wxPanel(this, -1, wxDefaultPosition, wxSize(1, 1));
	m_headerTC = new wxTextCtrl(this, -1, wxString::FromUTF8("Tiêu đề"), wxDefaultPosition, wxSize(1, 1), wxTE_CENTER | wxTE_NO_VSCROLL | wxBORDER_NONE);
	CreateNotePad(type);
	m_miniButton = new wxButton(this, -1, "", wxDefaultPosition, wxSize(1, 1), wxBORDER_NONE);
	m_closeButton = new wxButton(this, -1, "", wxDefaultPosition, wxSize(1, 1), wxBORDER_NONE);

	m_headerTC->SetMinSize(wxSize(50, 36));
	m_dragPanel->SetMinSize(wxSize(20, 10));
	this->SetMinSize(MIN_SIZE);
	this->SetHeaderFontSize(DEFAULT_HEADER_FONT_SIZE);
	this->SetContentFontSize(DEFAULT_CONTENT_FONT_SIZE);
	this->SetColor(DEFAULT_PANEL_COLOR);
	this->SetDragPanelColor(DEFAULT_DRAG_PANEL_COLOR);
	m_miniButton->SetBackgroundColour(m_dragPanel->GetBackgroundColour());
	m_miniButton->SetMinSize(wxSize(18, 18));
	m_closeButton->SetBackgroundColour(m_dragPanel->GetBackgroundColour());
	m_closeButton->SetMinSize(wxSize(18, 18));
#pragma endregion


#pragma region Layout
	wxBoxSizer* mainSizerV = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* topSizerV = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* dragSizerH = new wxBoxSizer(wxHORIZONTAL);
	dragSizerH->Add(m_dragPanel, 34, wxEXPAND);
	dragSizerH->Add(m_miniButton, 1);
	dragSizerH->Add(m_closeButton, 1);
	topSizerV->Add(dragSizerH, 3, wxEXPAND);
	topSizerV->Add(m_headerTC, 6, wxEXPAND | wxLEFT | wxRIGHT, BORDER_SIZE);
	mainSizerV->Add(topSizerV, 0, wxEXPAND);
	mainSizerV->Add(m_notepad, 10, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, BORDER_SIZE);
	this->SetSizer(mainSizerV);
#pragma endregion


#pragma region BindingEvents
	//m_dragPanel
	{
		m_dragPanel->Bind(wxEVT_MOTION, &NotePanel::OnDragPanelMouseMove, this);
		m_dragPanel->Bind(wxEVT_LEFT_DOWN, &NotePanel::OnDragPanelMouseLeftDown, this);
		m_dragPanel->Bind(wxEVT_LEFT_UP, &NotePanel::OnDragPanelMouseLeftUp, this);
		m_dragPanel->Bind(wxEVT_LEAVE_WINDOW, &NotePanel::OnDragPanelMouseLeave, this);
	}

	//m_miniButton
	{
		m_miniButton->Bind(wxEVT_ENTER_WINDOW, &NotePanel::OnMinimizeButtonMouseEnter, this);
		m_miniButton->Bind(wxEVT_LEAVE_WINDOW, &NotePanel::OnMinimizeButtonMouseLeave, this);
		m_miniButton->Bind(wxEVT_LEFT_DOWN, &NotePanel::OnMinimizeButtonMouseLeftDown, this);
		m_miniButton->Bind(wxEVT_PAINT, &NotePanel::OnMinimizeButtonPaint, this);
	}
	
	//m_closeButton
	{
		m_closeButton->Bind(wxEVT_ENTER_WINDOW, &NotePanel::OnCloseButtonMouseEnter, this);
		m_closeButton->Bind(wxEVT_LEAVE_WINDOW, &NotePanel::OnCloseButtonMouseLeave, this);
		m_closeButton->Bind(wxEVT_LEFT_DOWN, &NotePanel::OnCloseButtonMouseLeftDown, this);
		m_closeButton->Bind(wxEVT_PAINT, &NotePanel::OnCloseButtonPaint, this);
	}
	
	//m_headerTC
	{
		m_headerTC->Bind(wxEVT_TEXT, &NotePanel::OnHeaderTextChanged, this);
		//m_headerTC->Bind(wxEVT_CHAR_HOOK, &NotePanel::OnCharHook, this);
	}

	//this
	{
		this->Bind(wxEVT_MOTION, &NotePanel::OnMouseMove, this);
		this->Bind(wxEVT_LEFT_UP, &NotePanel::OnMouseLeftUp, this);
		this->Bind(wxEVT_LEFT_DOWN, &NotePanel::OnMouseLeftDown, this);
		this->Bind(wxEVT_LEAVE_WINDOW, &NotePanel::OnMouseLeave, this);
	}

	BindingEventRecursive(this);
#pragma endregion


	this->Layout();
	this->SetDoubleBuffered(true);
}
/**************************************************************************
*							PUBLIC MEMBER								  *
**************************************************************************/
#pragma region Public member
bool NotePanel::operator == (NotePanel* other) {//same note if they have the same origin rect and same header (in case rect overlapping)
	return this->GetOriginRect() == other->GetOriginRect() && this->GetHeaderText() == other->GetHeaderText();
}
bool NotePanel::operator != (NotePanel* other) {
	return !(*this == other);
}

void NotePanel::SetColor(wxColour color) {
	m_headerTC->SetBackgroundColour(color);
	m_notepad->SetColor(color);
}
void NotePanel::SetDragPanelColor(wxColour color) {
	this->SetBackgroundColour(color);
	m_dragPanel->SetBackgroundColour(color);
	m_miniButton->SetBackgroundColour(m_dragPanel->GetBackgroundColour());
	m_closeButton->SetBackgroundColour(m_dragPanel->GetBackgroundColour());
}
void NotePanel::SetOriginRect(wxRect rect){
	m_originRect = rect;
}
void NotePanel::SetFontScale(float scalar) {
	m_notepad->SetContentFontSize(m_contentFontSize * scalar);
}
void NotePanel::SetHeaderFontSize(int size){
	m_headerFontSize = size;
	m_headerTC->SetFont(wxFontInfo(size).Family(wxFONTFAMILY_DEFAULT).Bold());
}
void NotePanel::SetContentFontSize(int size){
	m_contentFontSize = size;
	m_notepad->SetContentFontSize(size);
}
void NotePanel::SetHeaderText(wxString text){
	m_headerTC->SetLabel(text);
}
void NotePanel::SetContentText(wxString text){
	m_notepad->SetLabel(text);
}

wxRect& NotePanel::GetOriginRect(void){
	return m_originRect;
}
wxString NotePanel::GetHeaderText(void){
	return m_headerTC->GetValue();
}
wxString NotePanel::GetContentText(void){
	return m_notepad->ToJson();
}

void NotePanel::UpdateSize(void) {//set the panel size to match with the origin rect size and zoom factor
	float zoomFactor = m_parent->GetZoomFactor();
	wxSize panelSize = m_originRect.GetSize() * zoomFactor;
	this->SetSize(panelSize);
	this->Layout();
}
void NotePanel::UpdateOnZoom(void) {//update size, position, font size and header alignment based on zoom factor
	//align left the header when zoom out (0.7f)
	float zoomFactor = m_parent->GetZoomFactor();
	if (zoomFactor <= 0.7f) {
		m_headerTC->SetWindowStyle(m_headerTC->GetWindowStyle() & ~wxTE_CENTER | wxTE_LEFT);
	}
	else {//align back to center when zoom in
		m_headerTC->SetWindowStyle(m_headerTC->GetWindowStyle() & ~wxTE_LEFT | wxTE_CENTER);
	}
	//scale the font
	this->SetFontScale(zoomFactor * 1.5f);

	//scaling size and re-position
	wxPoint panelPos = m_parent->CalcScrolledPosition(m_originRect.GetPosition() * zoomFactor);
	this->SetPosition(panelPos);
	this->UpdateSize();
}

//child events
void NotePanel::OnChildSizeReport(wxSize childSize) {//this function is used when the note panel children wants to change the parent size 
	float widthRatio = m_originRect.width * 1.0f / m_notepad->GetSize().x;
	float heightRatio = m_originRect.height * 1.0f / m_notepad->GetSize().y;
	int newWidth = childSize.x * widthRatio;
	int newHeight = childSize.y * heightRatio;
	m_originRect.SetWidth(newWidth);
	m_originRect.SetHeight(newHeight);
	this->UpdateSize();
	CHILD_CHANGED;
}
void NotePanel::OnChildChanged(void) {//propagates upward child changed event
	CHILD_CHANGED;
}

void NotePanel::FromJson(wxString json) {
	//convention: [type, pos, size, header, content]
	auto jsonList = SharedData::SplitByStartAndEnd(json, "[", "]");
	if (jsonList.size() != 5) return;//invalid size
	//skip the type
	
	//position
	{
		auto pos = SharedData::Split(jsonList[1], ",");
		if (pos.size() == 2) {//valid size
			m_originRect.SetPosition(wxPoint(NUMBER(pos[0].ToStdString()), NUMBER(pos[1].ToStdString())));
		}
	}
	//size
	{
		auto size = SharedData::Split(jsonList[2], ",");
		if (size.size() == 2) {//valid size
			m_originRect.SetSize(wxSize(NUMBER(size[0].ToStdString()), NUMBER(size[1].ToStdString())));

		}
	}
	this->UpdateOnZoom();//update after receive position and size
	//header
	{
		auto header = jsonList[3];
		if (SharedData::StartWith(header, "@\"") && SharedData::EndWith(header, "\"#")) {//start with @" and end with "#
			header.Remove(0, 2);//remove the first 2 characters
			header.RemoveLast(2);//remove the last 2 characters
			m_headerTC->SetValue(wxString::FromUTF8(header));
		}
	}
	//content
	{
		auto content = jsonList[4];
		m_notepad->FromJson(content);
	}
}
wxString NotePanel::ToJson(void) {
	if(m_isMinimized == true) SetMinimize(false); // is minimizing, un minimize it before save
	wxString retStr;
	retStr += "{";
	retStr += "\"type\":[" + NotePad::GetTypeString(m_notepad->GetType()) + "],";
	retStr += "\"pos\":[" + PAIR(m_originRect.GetPosition()) + "],";
	retStr += "\"size\":[" + PAIR(m_originRect.GetSize()) + "],";
	retStr += "\"header\":[@\"" + m_headerTC->GetValue() + "\"#],";
	retStr += m_notepad->ToJson();
	retStr += "},";
	return retStr;
}
#pragma endregion
/**************************************************************************
*							PRIVATE MEMBER								  *
**************************************************************************/
#pragma region Private member
RESIZE_DIRECTION NotePanel::GetCurrentResizeDirection(wxPoint mousePos) {
	int x = mousePos.x;
	int y = mousePos.y;
	int width = this->GetSize().x;
	int height = this->GetSize().y;
	int cornerOffset = 5;
	if (x <= 2 * BORDER_SIZE && y >= 2 * BORDER_SIZE &&  y <= height - 2 * BORDER_SIZE - cornerOffset) return LEFT;
	if (x <= BORDER_SIZE + cornerOffset && y > height - BORDER_SIZE - cornerOffset && y <= height) return BOTTOMLEFT;
	if (x > 2 * BORDER_SIZE + cornerOffset && x < width - 2 * BORDER_SIZE - cornerOffset &&  y > height - 2 * BORDER_SIZE && y <= height) return BOTTOM;
	if (x >= width - BORDER_SIZE - cornerOffset && x <= width && y >= height - BORDER_SIZE - cornerOffset && y <= height) return BOTTOMRIGHT;
	if (x >= width - 2 * BORDER_SIZE && x <= width && y < height - 2 * BORDER_SIZE - cornerOffset) return RIGHT;
	return NONE;
}
void NotePanel::SetMinimize(bool minimized) {
	float zoomFactor = m_parent->GetZoomFactor();
	if (minimized) {//the user asks to minimize
		if (!m_isMinimized) {//but the note has to be un-minimize
			m_originSize = m_originRect.GetSize();
			m_originRect.SetSize(wxSize(m_originRect.GetSize().x, 80));
			m_notepad->Hide();
		}
	}
	else {//the user asks to un-minimize
		if (m_isMinimized) {//but the note has to be un-mnimize
			m_notepad->Show();
			m_originRect.SetSize(m_originSize);
		}
	}
	m_isMinimized = minimized;
	this->UpdateOnZoom();
}
void NotePanel::BindingEventRecursive(wxWindow* window) {
	window->Bind(wxEVT_SET_FOCUS, &NotePanel::OnFocus, this);
	window->Bind(wxEVT_RIGHT_DOWN, [this](wxMouseEvent& evt) {
		m_rightDown = true;
		evt.Skip();
	});
	window->Bind(wxEVT_MOUSEWHEEL, [this](wxMouseEvent& evt) {
		evt.Skip(); 
		m_parent->OnScroll(evt.GetWheelRotation(), evt.ControlDown(), evt.ShiftDown()); 
	});

	auto childs = window->GetChildren();
	for (auto child : childs) {
		BindingEventRecursive(child);
	}
}
void NotePanel::CreateNotePad(NOTE_TYPE_e type) {
	switch (type) {
	case NOT_SET:
		m_notepad = NULL;
		break;
	case TEXT:
		m_notepad = new TextPad(this);
		break;
	case LIST:
		m_notepad = new ListPad(this);
		break;
	default:
		break;
	}
	this->Layout();
}

#pragma region Events
//dragging
void NotePanel::OnDragPanelMouseMove(wxMouseEvent& evt){
	auto panel = m_dragPanel;
	SetCursor(wxCURSOR_HAND);
	if (evt.LeftIsDown() && evt.Dragging() && panel->HasCapture()) {
		wxPoint delta = wxGetMousePosition() - NotePanel::s_delta;
		delta.x = delta.x / m_parent->GetZoomFactor();
		delta.y = delta.y / m_parent->GetZoomFactor();
		m_originRect.SetPosition((NotePanel::s_rectPos + delta));
		this->SetPosition(NotePanel::s_panelPos + delta);

		CHILD_CHANGED;
	}
}
void NotePanel::OnDragPanelMouseLeftDown(wxMouseEvent& evt){
	auto panel = m_dragPanel;
	panel->CaptureMouse();
	this->Raise();
	NotePanel::s_delta = wxGetMousePosition();
	NotePanel::s_panelPos = this->GetPosition();
	NotePanel::s_rectPos = m_originRect.GetPosition();
}
void NotePanel::OnDragPanelMouseLeftUp(wxMouseEvent& evt){
	auto panel = m_dragPanel;
	if (panel->HasCapture()) {
		panel->ReleaseMouse();
	}
}
void NotePanel::OnDragPanelMouseLeave(wxMouseEvent& evt){
	SetCursor(wxCURSOR_DEFAULT);
}

//text handling
void NotePanel::OnHeaderTextChanged(wxCommandEvent& evt) {//adjust width when user write too much text
	wxTextCtrl* tctrl = static_cast<wxTextCtrl*>(evt.GetEventObject());
	if (tctrl) { 
		float zoomFactor = m_parent->GetZoomFactor();
		int charWidth = tctrl->GetTextExtent(" ").x * 2;
		int maxWidth = GetLineMaxWidth(tctrl, tctrl->GetValue());

		if ((maxWidth + 30) * zoomFactor  >= tctrl->GetSize().x * zoomFactor) {//overflow on width
			m_originRect.SetWidth(m_originRect.width + (charWidth * 1.2f * zoomFactor));
			this->UpdateSize();
		}
		CHILD_CHANGED;
	}
	evt.Skip();
}

// resizing
void NotePanel::OnMouseMove(wxMouseEvent& evt){
	auto panel = this;
	RESIZE_DIRECTION direction = GetCurrentResizeDirection(evt.GetPosition());
	if (m_resizeDirection != NONE) direction = m_resizeDirection;
	if (direction == LEFT || direction == RIGHT) {
		SetCursor(wxCURSOR_SIZEWE);
	}
	else if (direction == BOTTOM) {
		SetCursor(wxCURSOR_SIZENS);
	}
	else if (direction == BOTTOMLEFT) {
		SetCursor(wxCURSOR_SIZENESW);
	}
	else if (direction == BOTTOMRIGHT) {
		SetCursor(wxCURSOR_SIZENWSE);
	}
	else SetCursor(wxCURSOR_DEFAULT);

	if (evt.LeftIsDown() && evt.Dragging() && panel->HasCapture() && m_resizeDirection != NONE) {
		wxPoint delta = wxGetMousePosition() - m_prevMousePos;
		delta.x = delta.x / m_parent->GetZoomFactor();
		delta.y = delta.y / m_parent->GetZoomFactor();
		wxSize size = wxSize(delta.x, delta.y);
		wxPoint pos = wxPoint(0, 0);
		
		if (m_resizeDirection == LEFT) {
			pos.x += delta.x;//x position increase
			size.y = 0;//no resize on height
			size.x *= -1;//size decrease
		}
		else if (m_resizeDirection == BOTTOMLEFT) {
			pos.x += delta.x;//x position increase
			size.x *= -1;//size decrease
		}
		else if (m_resizeDirection == RIGHT) {
			//no re-position
			size.y = 0;//no resize on height
		}
		else if (m_resizeDirection == BOTTOMRIGHT) {
			//no re-position
			//resize on both direction
		}
		else if (m_resizeDirection == BOTTOM) {
			//no re-position
			size.x = 0;//no resize on width
		}

		bool moveX = true;
		if ((m_prevPanelSize + size).x <= MIN_SIZE.x * m_parent->GetZoomFactor()) {//if min width then no change position on x direction
			moveX = false;
		}
		if (moveX == true) {
			m_originRect.SetPosition(m_prevRectPos + pos);
			this->SetPosition(m_prevPanelPos + pos);
		}
		m_originRect.SetSize(ClampMinSize(m_prevRectSize + size, MIN_SIZE));//make sure the size wont below min size even after zooming
		this->SetSize(ClampMinSize(m_prevPanelSize + size, MIN_SIZE));

		CHILD_CHANGED;
	}
	
}
void NotePanel::OnMouseLeave(wxMouseEvent& evt){
	if (m_resizeDirection != NONE) return;//resizing
	SetCursor(wxCURSOR_DEFAULT);
}
void NotePanel::OnMouseLeftDown(wxMouseEvent& evt){
	this->CaptureMouse();
	m_prevMousePos = wxGetMousePosition();
	m_prevRectPos = m_originRect.GetPosition();
	m_prevRectSize = m_originRect.GetSize();
	m_prevPanelPos = this->GetPosition();
	m_prevPanelSize = this->GetSize();
	m_resizeDirection = GetCurrentResizeDirection(evt.GetPosition());
}
void NotePanel::OnMouseLeftUp(wxMouseEvent& evt){
	if (this->HasCapture()) {
		this->ReleaseMouse();
	}
	m_resizeDirection = NONE;
}


#pragma region ButtonEvents
//minimize button
void NotePanel::OnMinimizeButtonMouseEnter(wxMouseEvent& evt) {
	m_minimizeHoverColor = wxColour(126, 127, 133);
	m_miniButton->Refresh();
}
void NotePanel::OnMinimizeButtonMouseLeave(wxMouseEvent& evt) {
	m_minimizeHoverColor = DEFAULT_HOVER_COLOR;
	m_miniButton->Refresh();
}
void NotePanel::OnMinimizeButtonMouseLeftDown(wxMouseEvent& evt) {
	SetMinimize(!m_isMinimized);
}
void NotePanel::OnMinimizeButtonPaint(wxPaintEvent& evt) {
	auto panel = m_miniButton;
	wxPaintDC dc(panel);
	dc.SetPen(*wxBLACK);
	dc.SetBrush(m_minimizeHoverColor);

	int width = std::max(panel->GetClientSize().x, 14);
	int height = std::max(panel->GetClientSize().y, 7);
	int radius = std::min(width, height) / 2 - 2;//min value between width and height

	dc.DrawCircle(wxPoint(width / 2, height / 2), wxCoord(radius));

	//draw minimize character
	dc.SetFont(wxFontInfo(13).Family(wxFONTFAMILY_DEFAULT).Light());
	wxSize textSize = dc.GetTextExtent("-");
	dc.DrawText("-", wxPoint(width / 2, height / 2) - (textSize / 2));
}
//close button
void NotePanel::OnCloseButtonMouseEnter(wxMouseEvent& evt) {
	m_closeHoverColor = wxColour(240, 86, 74);
	m_closeButton->Refresh();
}
void NotePanel::OnCloseButtonMouseLeave(wxMouseEvent& evt) {
	m_closeHoverColor = DEFAULT_HOVER_COLOR;
	m_closeButton->Refresh();
}
void NotePanel::OnCloseButtonMouseLeftDown(wxMouseEvent& evt) {
	CHILD_CHANGED;
	m_parent->DeleteNote(this);
	this->Destroy();
}
void NotePanel::OnCloseButtonPaint(wxPaintEvent& evt) {
	auto panel = m_closeButton;
	wxPaintDC dc(panel);
	dc.SetPen(*wxBLACK);
	dc.SetBrush(m_closeHoverColor);

	int width = std::max(panel->GetClientSize().x, 14);
	int height = std::max(panel->GetClientSize().y, 7);
	int radius = std::min(width, height) / 2 - 2;//min value between width and height

	dc.DrawCircle(wxPoint(width / 2, height / 2), wxCoord(radius));

	//draw close character
	dc.SetFont(wxFontInfo(10).Family(wxFONTFAMILY_DEFAULT).Light());
	wxSize textSize = dc.GetTextExtent("x");
	dc.DrawText("x", wxPoint(width / 2, height / 2) - (textSize / 2) - wxPoint(0, 1));
}
#pragma endregion

void NotePanel::OnFocus(wxFocusEvent& evt) {//raise panel when received focus
	auto window = static_cast<wxWindow*>(evt.GetEventObject());
	if (window) {
		this->Raise();
	}
	evt.Skip();
}
#pragma endregion
#pragma endregion
/**************************************************************************
*							STATIC MEMBER								  *
**************************************************************************/