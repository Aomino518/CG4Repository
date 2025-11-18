#include "SEFramework.h"
#include "Game.h"

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	SEFramework* seFw = new Game();
	seFw->Run();
	delete seFw;
}