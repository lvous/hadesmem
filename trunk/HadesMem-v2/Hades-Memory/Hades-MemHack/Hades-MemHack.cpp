// Hades-MemHack.cpp : main project file.

#include "stdafx.h"
#include "FrmMain.h"

using namespace HadesMemHack;

[STAThreadAttribute]
int main(array<System::String ^> ^/*args*/)
{
	// Enabling Windows XP visual effects before any controls are created
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);

	// Create the main window and run it
	Application::Run(gcnew FrmMain());

	return 0;
}
