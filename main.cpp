//**********************************************
//*
//* Copyright Thomas Freehill December 31 2016
//* This editor is intended to output resources
//* for use by the EngineOfEvil and Evil game
//*
//**********************************************

#include <Windows.h>
#include "Editor.h"

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
	editor.Init();
	while(editor.RunFrame())
		;
	editor.Shutdown();
	return 0;
}