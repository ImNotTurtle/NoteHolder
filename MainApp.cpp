#include "MainApp.h"


bool MainApp::OnInit(void) {
	wxInitAllImageHandlers();
	MainFrame* mainFrame = new MainFrame(nullptr, -1, "Note Holder", APP_POS, APP_SIZE);
	mainFrame->Show();
	return true;
}