#include "defs.hpp"
#include "internals.hpp"
#include "memory.hpp"
#include "struct.hpp"
#include "maths.hpp"
#include "overlay.hpp"

#pragma comment(lib, "ntdll")
#pragma comment(lib,"Advapi32.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "D3d9.lib")
#pragma comment(lib, "Dwmapi.lib")

#pragma warning(disable:4477) //format string type warnings

/* DEFINES */
PVOID base_addy;
PVOID d3d11_base;
DWORD64 MVPaddy;

CPlayerEnt localPlayer{};
DWORD64 ent_base;
DWORD64 localPlayerAddy;

unsigned screen_width;
unsigned screen_height;

std::vector<DWORD64> Player_addies;

extern HANDLE hProc;

#define _DEBUG 0

void Refresh_player_addies() {
	Player_addies.clear();

	DWORD64 current_ptr = ent_base;

	unsigned i = 0;

	while (i < LobbySize) {
		CPlayerEnt enemy{};
		DWORD64 addy = read<DWORD64>(current_ptr);
		ReadStruct(localPlayerAddy, &localPlayer,sizeof(CPlayerEnt));
		ReadStruct(addy, &enemy, sizeof(CPlayerEnt));

		SIZE_T wrote;

		if (enemy.TeamFlag1 == 0x2 || enemy.TeamFlag2 == 0x1) {
			/*
			BOOL e = WriteProcessMemory(hProc,(LPVOID)(localPlayerAddy+offsetof(CPlayerEnt, Position)), &enemy.Position,sizeof(Vector3),&wrote);
			printf("TELEPORTING %f %f %f => 0x%p\n",enemy.Position.x, enemy.Position.y, enemy.Position.z, addy);
			Sleep(3000);
			*/
			Player_addies.push_back(addy);
			i++;
			
		}
		current_ptr += 8;
	}
}

void MainHackFunc() {
	p_Device->Clear(0, 0, D3DCLEAR_TARGET, 0, 1.0f, 0);
	p_Device->BeginScene();

	if (GetForegroundWindow() == TargetHwnd)
	{
		//text without shadow
		DrawString("DECEIT DEEZ NUTS V1.0", 0, 0, 240, 240, 250, pFontSmall);

		//update screen_width in case it changed
		//screen_width = read<DWORD>((DWORD_PTR)d3d11_base + ResolutionManager);
		//screen_height = read<DWORD>((DWORD_PTR)d3d11_base + (ResolutionManager + 0x4));

		CPlayerEnt ent{};
		//printf("FOUND %d entities\n", Player_addies.size());

		//update ViewProjectionMatrix

		Mat4x4 ViewMatrix;
		ReadStruct(MVPaddy, &ViewMatrix, sizeof(Mat4x4));

		//update localPlayer struct
		ReadStruct(localPlayerAddy, &localPlayer, sizeof(CPlayerEnt));

		for (DWORD64 player_address : Player_addies) {

			//Update ent struct
			ReadStruct(player_address, &ent, sizeof(CPlayerEnt));

			Vector2 Pos{};

			Maths::WorldToScreen(ent.Position,Pos,ViewMatrix, screen_width, screen_height);
			
			//assert(Pos.x > screen_width || Pos.x < 0);
			
			printf("POS: (%f,%f)\n", Pos.x,Pos.y);



			//Sleep(50);
			
			DrawBox(screen_width-Pos.x, screen_height-Pos.y, 50, 50, 255, 0, 0, 255);
		}

		Refresh_player_addies();

	}
finish_render:
	p_Device->EndScene();
	p_Device->PresentEx(0, 0, 0, 0, 0);
}

LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_PAINT:
		MainHackFunc();
		break;
	case WM_CREATE:
		DwmExtendFrameIntoClientArea(hWnd, &Margin);
		break;

	case WM_DESTROY:
		PostQuitMessage(1);
		return 0;
	default:
		return DefWindowProc(hWnd, Message, wParam, lParam);
	}
	return 0;
}

void SetupConsole()
{
	BOOL bCreated = AllocConsole();
	if (!bCreated)
		return; // We already have a console.

	FILE* pNewStdout = nullptr;
	FILE* pNewStderr = nullptr;
	FILE* pNewStdin = nullptr;

	::freopen_s(&pNewStdout, "CONOUT$", "w", stdout);
	::freopen_s(&pNewStderr, "CONOUT$", "w", stderr);
	::freopen_s(&pNewStdin, "CONIN$", "r", stdin);

}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	SetupConsole();                                             
                                                       
	base_addy  = GetProcessBase(L"Deceit.exe");
	//d3d11_base = GetInnerProcessModuleBase(L"CryRenderD3D11.dll");

	if (!base_addy)
		exit(666);
	//if (!d3d11_base)
	//	exit(777);

	printf("[ + ] rendering engine base: %p\n", d3d11_base);

	//read the address of the ent base
 	ent_base = read<DWORD64>((DWORD_PTR)base_addy+EntityManager, entbase[0] );
	if(!ent_base)
		exit(888);
	ent_base += 0x30E*8; //player entities start

 	printf("[ + ] entbase %p .\n", ent_base);		
	
	localPlayerAddy = read<DWORD64>((DWORD_PTR)base_addy+LocalPlayerManager, playerbase[0], playerbase[1]);
 	if(!localPlayerAddy)
		exit(999);

	printf("[ + ] playerbase %p.\n\n", localPlayerAddy);

	//TODO: Find player name ptr		
	ReadStruct(localPlayerAddy, &localPlayer, sizeof(CPlayerEnt));	

	printf("[ + ] LocalPlayer position { %f, %f, %f}\n", localPlayer.Position.x, localPlayer.Position.y, localPlayer.Position.z);	

	//update game resolution
	screen_width = read<DWORD>((DWORD_PTR)base_addy + ResolutionManager);
	screen_height = read<DWORD>((DWORD_PTR)base_addy + (ResolutionManager + 0x4));

	if(!screen_height)
		exit(1111);
	if(!screen_width)
		exit(2222);

	printf("[ + ] Found game res: %dx%d\n", screen_width, screen_height);

	MVPaddy = read<DWORD64>((DWORD_PTR)base_addy + ViewProjectionMatrix);
	if(!MVPaddy)
		exit(3333);

	MVPaddy += 0x24;

	printf("[ + ] MVP address %p\n", MVPaddy);

	Refresh_player_addies();

	/* Begin drawing */
	DirectOverlay Overlay;
	HWND cur_hwnd = Overlay.INITWINDOW(hInstance, WinProc);
	Overlay.CreateDirectXDevice(cur_hwnd);

	FreeConsole();
	CloseHandle(hProc);	
	return 0;
}	
