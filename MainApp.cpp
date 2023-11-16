#include "MainApp.h"



/*
 TODO: 
	Move the TextPad context menu to the parent which is the note panel
	Move set fixed width and height of the TextPad to the base class which will apply for all other notepad
	Adjust the note holder panel scroll speed
	
	Add note control list

	 
*/

/*
* Problems need to be optimized:


*/

bool MainApp::OnInit(void) {
	wxInitAllImageHandlers();
	MainFrame* mainFrame = new MainFrame(nullptr, -1, "Note Holder", APP_POS, APP_SIZE);
	mainFrame->Show();
	return true;
}