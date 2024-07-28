#include "NotePanel_TextPad.h"

#define JSON_START_MARK							(wxString("@#$"))
#define JSON_END_MARK							(wxString("$#@"))

//use this macro whenever you wish to change the data
#define CHILD_CHANGED							(m_parent->OnChildChanged())

extern void SetStatusText(wxString text, int index = 0);


//menu id
const int wxID_ERASE_ALL = wxNewId();
const int wxID_SELECT_ALL = wxNewId();
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
		m_tctrl->Bind(wxEVT_TEXT, &TextPad::OnTextChanged, this);
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
void TextPad::SetFixedWidth(bool isFixed) {
	m_isFixedWidth = isFixed;
	//do nothing, yet
	return;
	//int scrollbarWidth = 15;
	//auto window = m_tctrl;
	//if (isFixed) {
	//	//show the vertical scroll bar
	//	m_tctrl->SetScrollbar(wxVERTICAL, 0, 20, 50);
	//	
	//	//increase the width after add the scrollbar
	//	wxSize newSize = this->GetSize();
	//	newSize.SetWidth(newSize.GetWidth() + scrollbarWidth);
	//	this->PrepareToSendSizeRequest(newSize);
	//}
	//else {

	//	//hide the vertical scroll bar
	//	m_tctrl->SetScrollbar(wxVERTICAL, 0, 0, 0);
	//	//expand the height to fit the content
	//	this->SetSizeToFitContent(false, true);

	//	//descrease the size when remove scrollbar
	//	wxSize newSize = this->GetSize();
	//	newSize.SetWidth(newSize.GetWidth() - scrollbarWidth);
	//	this->PrepareToSendSizeRequest(newSize);
	//}
}
void TextPad::SetFixedHeight(bool isFixed) {
	m_isFixedHeight = isFixed;
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

NOTE_TYPE_e TextPad::GetType(void) {
	return TEXT;
}

void TextPad::ReceiveTabNavigation(void) {
	m_tctrl->SetFocus();
	m_tctrl->SetInsertionPointEnd();
}
void TextPad::AddOwnContextMenu(wxWindow* parent, wxMenu* menu) {
	auto owner = m_tctrl;

	menu->AppendSeparator();
	auto selectAll = menu->Append(wxID_SELECT_ALL, wxString::FromUTF8("Chọn toàn bộ nội dung \t Ctrl + A"));
	auto eraseContent = menu->Append(wxID_ERASE_ALL, wxString::FromUTF8("Xóa nội dung"));
	auto copy = menu->Append(wxID_COPY, wxString::FromUTF8("Sao chép \t Ctrl + C"));
	auto paste = menu->Append(wxID_PASTE, wxString::FromUTF8("Dán \t Ctrl + V"));
	auto cut = menu->Append(wxID_CUT, wxString::FromUTF8("Cắt \t Ctrl + X"));
	auto undo = menu->Append(wxID_UNDO, wxString::FromUTF8("Hoàn tác\tCtrl + Z"));
	auto redo = menu->Append(wxID_REDO, wxString::FromUTF8("Hủy hoàn tác\tCtrl + Y"));
	wxAcceleratorEntry entries[1];
	entries[0].Set(wxACCEL_CTRL, (int)'Y', redo->GetId());

	wxAcceleratorTable table(1, entries);
	owner->SetAcceleratorTable(table);

	parent->Bind(wxEVT_MENU, [=](wxCommandEvent& evt) {
		owner->Remove(0, owner->GetLastPosition());
		CHILD_CHANGED;
		}, eraseContent->GetId());
	parent->Bind(wxEVT_MENU, [=](wxCommandEvent& evt) {
		owner->SelectAll();
		}, selectAll->GetId());
	parent->Bind(wxEVT_MENU, [=](wxCommandEvent& evt) {
		owner->Copy();
		}, copy->GetId());
	parent->Bind(wxEVT_MENU, [=](wxCommandEvent& evt) {
		owner->Paste();
		}, paste->GetId());
	parent->Bind(wxEVT_MENU, [=](wxCommandEvent& evt) {
		owner->Cut();
		}, cut->GetId());
	parent->Bind(wxEVT_MENU, [=](wxCommandEvent& evt) {
		owner->Undo();
		}, undo->GetId());
	parent->Bind(wxEVT_MENU, [=](wxCommandEvent& evt) {
		owner->Redo();
		}, redo->GetId());
}
void TextPad::UpdateOwnContextMenu(wxMenu* menu) {
	auto owner = m_tctrl;
	auto copy = menu->FindItem(wxID_COPY);
	{
		if (copy) {
			copy->Enable(owner->CanCopy());
		}
	}
	auto cut = menu->FindItem(wxID_CUT);
	{
		if (cut) {
			cut->Enable(owner->CanCut());
		}
	}
	auto paste = menu->FindItem(wxID_PASTE);
	{
		if (paste) {
			paste->Enable(owner->CanPaste());
		}
	}
	auto undo = menu->FindItem(wxID_UNDO);
	{
		if (undo) {
			undo->Enable(owner->CanUndo());
		}
	}
	auto redo = menu->FindItem(wxID_REDO);
	{
		if (redo) {
			redo->Enable(owner->CanRedo());
		}
	}
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
	json += "content:[" + JSON_START_MARK + m_tctrl->GetValue() + JSON_END_MARK + "]";
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
	//adjust the width as the user typing too long text and mark the note holder file as changed
	auto window = m_tctrl;
	if (window) {
		if (!m_isFixedWidth) {// is possible to expand on width
			int charWidth = window->GetCharWidth() * 2; //idk but * 2 works just fine
			int maxWidth = GetLineMaxWidth(window, window->GetValue());

			if ((maxWidth + charWidth) >= window->GetSize().x) {//overflow on width
				wxSize newSize = window->GetSize();
				newSize.x += charWidth * 2.0f;
				this->PrepareToSendSizeRequest(newSize);
			}
		}
		
		CHILD_CHANGED;
	}
	evt.Skip();
}
#pragma endregion



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
#pragma endregion
/**************************************************************************
*							STATIC MEMBER								  *
**************************************************************************/
