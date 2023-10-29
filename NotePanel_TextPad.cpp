#include "NotePanel_TextPad.h"

#define JSON_START_MARK							(wxString("@#$"))
#define JSON_END_MARK							(wxString("$#@"))

#define CHILD_CHANGED							(m_parent->OnChildChanged())

extern void SetStatusText(wxString text, int index = 0);

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


TextPad::TextPad(NotePanel* parent)
	: NotePad(parent) {
#pragma region Init
	m_parent = parent;
	m_tctrl = new wxTextCtrl(this, -1, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_MULTILINE | wxTE_NO_VSCROLL | wxBORDER_SIMPLE);
	BuildContextMenu();
#pragma endregion


#pragma region Layout
	wxBoxSizer* mainSizerV = new wxBoxSizer(wxVERTICAL);
	mainSizerV->Add(m_tctrl, 1, wxEXPAND);
	this->SetSizer(mainSizerV);
	this->Layout();
#pragma endregion


#pragma region Binding
	m_tctrl->Bind(wxEVT_CHAR_HOOK, &TextPad::OnCharHook, this);
	m_tctrl->Bind(wxEVT_TEXT_PASTE, &TextPad::OnTextPaste, this);
	m_tctrl->Bind(wxEVT_CONTEXT_MENU, &TextPad::OnContextMenu, this);
	m_tctrl->Bind(wxEVT_RIGHT_DOWN, &TextPad::OnRightDown, this);
	m_tctrl->Bind(wxEVT_TEXT, &TextPad::OnTextChanged, this);
#pragma endregion

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

void TextPad::FromJson(wxString json) {
	if (SharedData::StartWith(json, JSON_START_MARK) && SharedData::EndWith(json, JSON_END_MARK)) {
		json.Remove(0, JSON_START_MARK.size());
		json.RemoveLast(JSON_END_MARK.size());
		m_tctrl->SetValue(wxString::FromUTF8(json));
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
void TextPad::OnCharHook(wxKeyEvent& evt) {//adjust the height when user press enter
	wxTextCtrl* tctrl = m_tctrl;
	if (evt.GetKeyCode() == WXK_RETURN) {
		//adjust the height to add new space
		int lineHeight = tctrl->GetCharHeight();
		int lineCount = tctrl->GetNumberOfLines() + 2;
		int heightDelta = lineCount * lineHeight - tctrl->GetSize().y;
		if (heightDelta > 0) {//overflow on height
			wxSize newSize = wxSize(tctrl->GetSize().x, lineHeight * lineCount);
			m_parent->OnChildSizeReport(newSize);
		}
		tctrl->WriteText("\n");
	}
	else {
		evt.Skip();
	}
}
void TextPad::OnTextChanged(wxCommandEvent& evt) {
	auto window = m_tctrl;
	if (window) {
		int charWidth = window->GetCharWidth() * 2;
		int maxWidth = GetLineMaxWidth(window, window->GetValue());
		
		if ((maxWidth + charWidth) >= window->GetSize().x) {//overflow on width
			wxSize newSize = window->GetSize();
			newSize.x += charWidth * 2.0f;
			m_parent->OnChildSizeReport(newSize);
		}
		CHILD_CHANGED;
	}
	evt.Skip();
}
void TextPad::OnTextPaste(wxClipboardTextEvent& evt) {//adjust row when user paste text 
	wxTextCtrl* window = m_tctrl;
	wxClipboard clipboard;
	if (clipboard.Open()) {
		if (clipboard.IsSupported(wxDF_TEXT)) {
			wxTextDataObject data;
			clipboard.GetData(data);
			wxString pastedText = data.GetText();//get pasted text

			//calculate number of row need to expand
			int lineHeight = window->GetCharHeight();
			int newLineCount = 0;
			wxStringTokenizer tokenizer(pastedText, "\n");
			while (tokenizer.HasMoreTokens()) {
				newLineCount++;
				wxString line = tokenizer.GetNextToken();
			}
			int clipboardHeight = lineHeight * newLineCount;

			//get current content height of the textctrl
			int contentHeight = window->GetCharHeight() * (window->GetNumberOfLines());
			//adjust the height when adding new content overflow on height 
			if (contentHeight + clipboardHeight > window->GetSize().y) {
				wxSize newSize = wxSize(window->GetSize().x, contentHeight + clipboardHeight);
				m_parent->OnChildSizeReport(newSize);
			}
			CHILD_CHANGED;
		}
		clipboard.Close();
	}
	evt.Skip();
}
void TextPad::OnContextMenu(wxContextMenuEvent& evt) {
	if (m_rightDown) {
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

#pragma endregion


void TextPad::BuildContextMenu(void) {
	auto owner = m_tctrl;
	m_contextMenu = new wxMenu();

	auto fitContent = m_contextMenu->Append(wxID_ANY, wxString::FromUTF8("Tùy chỉnh kích thước tự động"));
	auto eraseContent = m_contextMenu->Append(wxID_ANY, wxString::FromUTF8("Xóa nội dung"));
	m_contextMenu->AppendSeparator();
	auto selectAll = m_contextMenu->Append(wxID_ANY, wxString::FromUTF8("Chọn toàn bộ nội dung \t Ctrl + A"));
	auto copy = m_contextMenu->Append(wxID_ANY, wxString::FromUTF8("Sao chép \t Ctrl + C"));
	auto paste = m_contextMenu->Append(wxID_ANY, wxString::FromUTF8("Dán \t Ctrl + V"));
	auto cut = m_contextMenu->Append(wxID_ANY, wxString::FromUTF8("Cắt \t Ctrl + X"));


	this->Bind(wxEVT_MENU, [=](wxCommandEvent& evt) {
		//find the text size and fit it
		wxClientDC dc(owner);
		wxSize textSize = dc.GetMultiLineTextExtent(owner->GetValue());
		wxSize minSize = m_parent->GetMinSize();
		//add some padding
		textSize.x += owner->GetCharWidth() * 2;
		textSize.y += owner->GetCharHeight();
		//clamp the size
		textSize.x = SharedData::Max(textSize.x, minSize.x);
		textSize.y = SharedData::Max(textSize.y, minSize.y);
		m_parent->OnChildSizeReport(textSize);
		CHILD_CHANGED;
		//this->UpdateOnZoom();//update the note after set size
		}, fitContent->GetId());
	this->Bind(wxEVT_MENU, [=](wxCommandEvent& evt) {
		owner->SetValue("");
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
}
#pragma endregion
/**************************************************************************
*							STATIC MEMBER								  *
**************************************************************************/