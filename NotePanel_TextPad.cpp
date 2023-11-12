#include "NotePanel_TextPad.h"

#define JSON_START_MARK							(wxString("@#$"))
#define JSON_END_MARK							(wxString("$#@"))

//use this macro whenever you wish to change the data
#define CHILD_CHANGED							(m_parent->OnChildChanged())

extern void SetStatusText(wxString text, int index = 0);

//menu id
const int wxID_FIT_CONTENT = wxNewId();
const int wxID_ERASE_ALL = wxNewId();
const int wxID_SELECT_ALL = wxNewId();
const int wxID_FIXED_WIDTH = wxNewId();
const int wxID_FIXED_HEIGHT = wxNewId();
const int wxID_FIXED_SIZE = wxNewId();

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

static wxString GetDataFromClipboard(void) {
	//return empty string if there is no data from the clipbord
	wxClipboard clipboard;
	if (clipboard.Open()) {
		if (clipboard.IsSupported(wxDF_TEXT)) {
			wxTextDataObject data;
			clipboard.GetData(data);
			return data.GetText();//get pasted text
		}
		clipboard.Close();
	}
	return "";
}

TextPad::TextPad(NotePanel* parent)
	: NotePad(parent) {
#pragma region Init
	m_isFixedWidth = false;
	m_isFixedHeight = false;
	m_parent = parent;
	m_tctrl = new wxTextCtrl(this, -1, "", wxDefaultPosition, wxDefaultSize, 
		wxTE_PROCESS_ENTER | wxTE_MULTILINE | wxBORDER_SIMPLE);
	m_tctrl->SetName("tctrl");
	this->SetName("TextPad");
	BuildContextMenu();

#pragma endregion


#pragma region Layout
	wxBoxSizer* mainSizerV = new wxBoxSizer(wxVERTICAL);
	mainSizerV->Add(m_tctrl, 1, wxEXPAND);
	this->SetSizer(mainSizerV);
	this->Layout();
#pragma endregion


#pragma region Binding
	//this
	{
		this->Bind(wxEVT_SIZE, &TextPad::OnResize, this);
		
	}

	//m_tctrl
	{
		m_tctrl->Bind(wxEVT_CHAR_HOOK, &TextPad::OnCharHook, this);
		m_tctrl->Bind(wxEVT_TEXT_PASTE, &TextPad::OnTextPaste, this);
		m_tctrl->Bind(wxEVT_CONTEXT_MENU, &TextPad::OnContextMenu, this);
		m_tctrl->Bind(wxEVT_RIGHT_DOWN, &TextPad::OnRightDown, this);
		m_tctrl->Bind(wxEVT_TEXT, &TextPad::OnTextChanged, this);
	}

	//m_contextMenu
	{
		m_contextMenu->Bind(wxEVT_UPDATE_UI, &TextPad::OnMenuUpdateUI, this);
	}
#pragma endregion

	this->EnableScrollbar(wxVERTICAL, false);
}
/**************************************************************************
*							PUBLIC MEMBER								  *
**************************************************************************/
#pragma region Public member
void TextPad::SetColor(wxColour color) {
	this->SetBackgroundColour(color);
	m_tctrl->SetBackgroundColour(color);
}
void TextPad::SetContentFontSize(int size) {
	m_tctrl->SetFont(wxFontInfo(size).Family(wxFONTFAMILY_DEFAULT));
}

NOTE_TYPE_e TextPad::GetType(void) {
	return TEXT;
}
void TextPad::ReceiveTabNavigation(void) {
	m_tctrl->SetFocus();
	m_tctrl->SetInsertionPointEnd();
}

void TextPad::FromJson(wxString json) {
	if (SharedData::StartWith(json, JSON_START_MARK) && SharedData::EndWith(json, JSON_END_MARK)) {
		json.Remove(0, JSON_START_MARK.size());
		json.RemoveLast(JSON_END_MARK.size());
		m_tctrl->SetValue(json);
	}
}
wxString TextPad::ToJson(void) {
	wxString json;
	json += "\"content\":[" + JSON_START_MARK + m_tctrl->GetValue() + JSON_END_MARK + "]";
	return json;
}
#pragma endregion
/**************************************************************************
*							PRIVATE MEMBER								  *
**************************************************************************/
#pragma region Private member
#pragma region Events
void TextPad::OnResize(wxSizeEvent& evt) {
	this->Layout();
	evt.Skip();
}
void TextPad::OnCharHook(wxKeyEvent& evt) {//adjust the height when user press enter
	wxTextCtrl* tctrl = m_tctrl;
	if (evt.GetKeyCode() == WXK_RETURN) {
		//only adjust the height if the user do not lock the height
		if (m_isFixedHeight) {//lock the height -> do nothing
			evt.Skip();
		}
		else { // not locking the height
			//adjust the height to add new space
			int lineHeight = tctrl->GetCharHeight();
			int lineCount = tctrl->GetNumberOfLines() + 2;
			int heightDelta = lineCount * lineHeight - tctrl->GetSize().y;
			if (heightDelta > 0) {//overflow on height
				wxSize newSize = wxSize(tctrl->GetSize().x, lineHeight * lineCount);
				this->PrepareToSendSizeRequest(newSize);
			}
			tctrl->WriteText("\n");
		}
	}
	else if (evt.GetKeyCode() == WXK_TAB) { // when user press tab, add 4 spaces to the control
		tctrl->WriteText("    ");
	}
	else {
		evt.Skip();
	}
}
void TextPad::OnTextPaste(wxClipboardTextEvent& evt) {//add row when user paste text 
	wxTextCtrl* window = m_tctrl;
	wxString clipboardData = GetDataFromClipboard();
	if (!clipboardData.IsEmpty()) {
		//calculate number of row need to expand
		int lineHeight = window->GetCharHeight();
		int newLineCount = 0;
		//count how many lines in the clipboard
		{
			wxStringTokenizer tokenizer(clipboardData, "\n");
			while (tokenizer.HasMoreTokens()) {
				newLineCount++;
				wxString line = tokenizer.GetNextToken();
			}
		}

		int clipboardHeight = lineHeight * newLineCount;
		//get current content height of the textctrl
		int contentHeight = window->GetCharHeight() * (window->GetNumberOfLines());
		//adjust the height when adding new content overflow on height 
		if (contentHeight + clipboardHeight > window->GetSize().y) {
			wxSize newSize = wxSize(window->GetSize().x, contentHeight + clipboardHeight);
			this->PrepareToSendSizeRequest(newSize);
		}
		CHILD_CHANGED;
	}
	evt.Skip();
}
void TextPad::OnTextChanged(wxCommandEvent& evt) {
	//adjust the width as the user typing too long text
	if (m_isFixedWidth) return; // not possible to expand on width
	auto window = m_tctrl;
	if (window) {
		int charWidth = window->GetCharWidth() * 2; //idk but * 2 works just fine
		int maxWidth = GetLineMaxWidth(window, window->GetValue());

		if ((maxWidth + charWidth) >= window->GetSize().x) {//overflow on width
			wxSize newSize = window->GetSize();
			newSize.x += charWidth * 2.0f;
			this->PrepareToSendSizeRequest(newSize);
		}
		CHILD_CHANGED;
	}
	evt.Skip();
}
void TextPad::OnContextMenu(wxContextMenuEvent& evt) {
	if (m_rightDown) {
		//this->BuildContextMenu();
		m_contextMenu->UpdateUI();
		auto clientPos = evt.GetPosition() == wxDefaultPosition ?
			(wxPoint(this->GetSize().GetWidth() / 2, this->GetSize().GetHeight() / 2))
			: this->ScreenToClient(evt.GetPosition());

		this->PopupMenu(m_contextMenu, clientPos);
		m_rightDown = false;
	}
}
void TextPad::OnRightDown(wxMouseEvent& evt) {
	m_rightDown = true;
	evt.Skip();
}
void TextPad::OnMenuUpdateUI(wxUpdateUIEvent& evt) {
	auto owner = m_tctrl;
	auto copy = m_contextMenu->FindItem(wxID_COPY);
	{
		if (copy) {
			copy->Enable(owner->CanCopy());
		}
	}
	auto cut = m_contextMenu->FindItem(wxID_CUT);
	{
		if (cut) {
			cut->Enable(owner->CanCut());
		}
	}
	auto paste = m_contextMenu->FindItem(wxID_PASTE);
	{
		if (paste) {
			paste->Enable(owner->CanPaste());
		}
	}
	auto undo = m_contextMenu->FindItem(wxID_UNDO);
	{
		if (undo) {
			undo->Enable(owner->CanUndo());
		}
	}
	auto redo = m_contextMenu->FindItem(wxID_REDO);
	{
		if (redo) {
			redo->Enable(owner->CanRedo());
		}
	}
	auto fixedWidth = m_contextMenu->FindItem(wxID_FIXED_WIDTH);
	{
		if (fixedWidth) {
			if (m_isFixedWidth) {
				fixedWidth->SetItemLabel(wxString::FromUTF8("Hủy cố định chiều rộng"));
			}
			else {
				fixedWidth->SetItemLabel(wxString::FromUTF8("Cố định chiều rộng"));
			}
			fixedWidth->Check(m_isFixedWidth);
		}
	}
	auto fixedHeight = m_contextMenu->FindItem(wxID_FIXED_HEIGHT);
	{
		if (fixedHeight) {
			if (m_isFixedHeight) {
				fixedHeight->SetItemLabel(wxString::FromUTF8("Hủy cố định chiều cao"));
			}
			else {
				fixedHeight->SetItemLabel(wxString::FromUTF8("Cố định chiều cao"));
			}
			fixedHeight->Check(m_isFixedHeight);
		}
	}
}

#pragma endregion


void TextPad::BuildContextMenu(void) {
	auto owner = m_tctrl;
	m_contextMenu = new wxMenu();

	auto selectAll = m_contextMenu->Append(wxID_SELECT_ALL, wxString::FromUTF8("Chọn toàn bộ nội dung \t Ctrl + A"));
	auto eraseContent = m_contextMenu->Append(wxID_ERASE_ALL, wxString::FromUTF8("Xóa nội dung"));
	auto copy = m_contextMenu->Append(wxID_COPY, wxString::FromUTF8("Sao chép \t Ctrl + C"));
	auto paste = m_contextMenu->Append(wxID_PASTE, wxString::FromUTF8("Dán \t Ctrl + V"));
	auto cut = m_contextMenu->Append(wxID_CUT, wxString::FromUTF8("Cắt \t Ctrl + X"));
	auto undo = m_contextMenu->Append(wxID_UNDO, wxString::FromUTF8("Hoàn tác\tCtrl + Z"));
	auto redo = m_contextMenu->Append(wxID_REDO, wxString::FromUTF8("Hủy hoàn tác\tCtrl + Y"));
	m_contextMenu->AppendSeparator();
	auto fitContent = m_contextMenu->Append(wxID_FIT_CONTENT, wxString::FromUTF8("Tùy chỉnh kích thước tự động"));
	auto fixedWidth = m_contextMenu->AppendCheckItem(wxID_FIXED_WIDTH, wxString::FromUTF8("Cố định chiều rộng"));
	auto fixedHeight = m_contextMenu->AppendCheckItem(wxID_FIXED_HEIGHT, wxString::FromUTF8("Cố định chiều cao"));

	wxAcceleratorEntry entries[1];
	entries[0].Set(wxACCEL_CTRL, (int)'Y', redo->GetId());
	
	wxAcceleratorTable table(1, entries);
	owner->SetAcceleratorTable(table);

	this->Bind(wxEVT_MENU, [this](wxCommandEvent& evt) {
		this->SetSizeToFitContent(true, true);
		}, fitContent->GetId());
	this->Bind(wxEVT_MENU, [=](wxCommandEvent& evt) {
		owner->Clear();
		CHILD_CHANGED;
		}, eraseContent->GetId());
	this->Bind(wxEVT_MENU, [=](wxCommandEvent& evt) {
		owner->SelectAll();
		}, selectAll->GetId());
	this->Bind(wxEVT_MENU, [=](wxCommandEvent& evt) {
		owner->Copy();
		}, copy->GetId());
	this->Bind(wxEVT_MENU, [=](wxCommandEvent& evt) {
		owner->Paste();
		}, paste->GetId());
	this->Bind(wxEVT_MENU, [=](wxCommandEvent& evt) {
		owner->Cut();
		}, cut->GetId());
	this->Bind(wxEVT_MENU, [=](wxCommandEvent& evt) {
		owner->Undo();
		}, undo->GetId());
	this->Bind(wxEVT_MENU, [=](wxCommandEvent& evt) {
		owner->Redo();
		}, redo->GetId());
	this->Bind(wxEVT_MENU, [=](wxCommandEvent& evt) {
		m_isFixedWidth = !m_isFixedWidth;
		this->SetFixedWidth(m_isFixedWidth);
		}, fixedWidth->GetId());
	this->Bind(wxEVT_MENU, [=](wxCommandEvent& evt) {
		m_isFixedHeight = !m_isFixedHeight;
		this->SetFixedHeight(m_isFixedHeight);
		}, fixedHeight->GetId());
}
void TextPad::SetFixedWidth(bool isFixed) {
	return;
	int scrollbarWidth = 15;
	auto window = m_tctrl;
	if (isFixed) {
		//show the vertical scroll bar
		m_tctrl->SetScrollbar(wxVERTICAL, 0, 20, 50);
		
		//increase the width after add the scrollbar
		wxSize newSize = this->GetSize();
		newSize.SetWidth(newSize.GetWidth() + scrollbarWidth);
		this->PrepareToSendSizeRequest(newSize);
	}
	else {

		//hide the vertical scroll bar
		m_tctrl->SetScrollbar(wxVERTICAL, 0, 0, 0);
		//expand the height to fit the content
		this->SetSizeToFitContent(false, true);

		//descrease the size when remove scrollbar
		wxSize newSize = this->GetSize();
		newSize.SetWidth(newSize.GetWidth() - scrollbarWidth);
		this->PrepareToSendSizeRequest(newSize);
	}
}
void TextPad::SetFixedHeight(bool isFixed) {
	int scrollbarWidth = 15;
	auto window = m_tctrl;
	if (isFixed) {
		//show the vertical scroll bar
		this->EnableScrollbar(wxVERTICAL);

		//increase the width after add the scrollbar
		wxSize newSize = this->GetSize();
		newSize.SetWidth(newSize.GetWidth() + scrollbarWidth);
		this->PrepareToSendSizeRequest(newSize);
	}
	else {
		//hide the vertical scroll bar
		this->EnableScrollbar(wxVERTICAL, false);
		//expand the height to fit the content
		this->SetSizeToFitContent(false, true);

		//descrease the size when remove scrollbar
		wxSize newSize = this->GetSize();
		newSize.SetWidth(newSize.GetWidth() - scrollbarWidth);
		this->PrepareToSendSizeRequest(newSize);
	}
}
void TextPad::SetFixedSize(bool fixedWidth, bool fixedHeight) {
	this->SetFixedWidth(fixedWidth);
	this->SetFixedHeight(fixedHeight);
}
void TextPad::EnableScrollbar(int direction, bool enable) {
	auto window = m_tctrl;
	if (enable) {
		window->SetScrollbar(direction, 0, 20, 50);
	}
	else {
		window->SetScrollbar(direction, 0, 0, 0);
	}
}
void TextPad::PrepareToSendSizeRequest(wxSize newSize) {
	//this class is not allow to change their size directly
	//when the control wish to change the size, call this function to report to the parent
	m_parent->OnChildSizeReport(newSize);
}
void TextPad::SetSizeToFitContent(bool fitWidth, bool fitHeight) {
	if (!fitWidth && !fitHeight) return; // nothing to change here
	auto window = m_tctrl;
	wxClientDC dc(window);
	//find the text size and fit it
	wxSize newSize = dc.GetMultiLineTextExtent(window->GetValue()); // the size request for the text
	wxSize minSize = m_parent->GetMinSize();
	//add some padding
	newSize.x += window->GetCharWidth() * 3;
	newSize.y += window->GetCharHeight();

	if (!fitWidth) {//keep the width un-change
		newSize.SetWidth(window->GetSize().GetWidth());
	}
	if (!fitHeight) {//keep the height un-change
		newSize.SetHeight(window->GetSize().GetHeight());
	}
	this->PrepareToSendSizeRequest(newSize);
	CHILD_CHANGED;
}
#pragma endregion
/**************************************************************************
*							STATIC MEMBER								  *
**************************************************************************/
