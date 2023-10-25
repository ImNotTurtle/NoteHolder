#pragma once
#include <wx/wx.h>

#include "MainFrame.h"



class MainApp : public wxApp
{
public:
	bool OnInit(void);
};

wxIMPLEMENT_APP(MainApp);