#include "MainApp.h"



/*
 TODO: 
	
	 
*/

/*
* Problems need to be optimized:

*/


extern APP_STATE_e e_appState;

bool MainApp::OnInit(void) {
	e_appState = APP_STATE_e::INIT;
	wxInitAllImageHandlers();
	MainFrame* mainFrame = new MainFrame(nullptr, -1, "Note Holder", APP_POS, APP_SIZE);
	mainFrame->Show();
	e_appState = APP_STATE_e::IDLE;

	return true;
}