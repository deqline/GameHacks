#pragma once

const MARGINS Margin = { -1 };
int Width;
int Height;

IDirect3D9Ex* p_Object = 0;
IDirect3DDevice9Ex* p_Device = 0;
D3DPRESENT_PARAMETERS p_Params;

ID3DXFont* pFontSmall = 0;
ID3DXLine* p_Line;

HWND TargetHwnd;

int DrawString(LPCSTR String, int x, int y, int r, int g, int b, ID3DXFont* ifont)
{
	RECT ShadowPos;
	ShadowPos.left = x + 1;
	ShadowPos.top = y + 1;
	RECT FontPos;
	FontPos.left = x;
	FontPos.top = y;
	ifont->DrawTextA(0, String, strlen(String), &ShadowPos, DT_NOCLIP, D3DCOLOR_ARGB(255, r / 3, g / 3, b / 3));
	ifont->DrawTextA(0, String, strlen(String), &FontPos, DT_NOCLIP, D3DCOLOR_ARGB(255, r, g, b));
	return 0;
}

void DrawBox(float x, float y, float width, float height, int r, int g, int b, int a)
{
	D3DXVECTOR2 points[5];
	points[0] = D3DXVECTOR2(x, y);
	points[1] = D3DXVECTOR2(x + width, y);
	points[2] = D3DXVECTOR2(x + width, y + height);
	points[3] = D3DXVECTOR2(x, y + height);
	points[4] = D3DXVECTOR2(x, y);
	p_Line->SetWidth(1);
	p_Line->Draw(points, 5, D3DCOLOR_RGBA(r, g, b, a));
}

class DirectOverlay {
public:
	DirectOverlay() {};
	HWND INITWINDOW(HINSTANCE hInstance, WNDPROC WindowProc) {
		WNDCLASSEXA wc{};
		HWND hWnd;

		LPCSTR window_name = "Render";
		LPCSTR class_name = "RenderClass";

		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = WindowProc;
		wc.hInstance = hInstance;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
		wc.lpszClassName = class_name;
		wc.lpszMenuName = window_name;

		RegisterClassExA(&wc);

		//Find deceit window
		TargetHwnd = FindWindowA(0, "Deceit");
		if (!TargetHwnd) {
			ExitProcess(998);
		}

		RECT rc;
		GetClientRect(TargetHwnd, &rc);

		Width = rc.right - rc.left;
		Height = rc.bottom - rc.top;

		DWORD dwStyle = GetWindowLong(TargetHwnd, GWL_STYLE);
		if (dwStyle & WS_BORDER)
		{
			rc.top += 23;
			Height -= 23;
		}

		hWnd = CreateWindowExA(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED, class_name , window_name, WS_POPUP, 1, 1, Width, Height, 0, 0, 0, 0);
		MoveWindow(hWnd, rc.left, rc.top, Width, Height, true);

		//Set layered window opacity and color
		SetLayeredWindowAttributes(hWnd, 0, 1.0f, LWA_ALPHA);
		SetLayeredWindowAttributes(hWnd, 0, RGB(0, 0, 0), LWA_COLORKEY);

		ShowWindow(hWnd, SW_SHOW);
		
		return hWnd;
	}

	void CreateDirectXDevice(HWND hWnd) {
		if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &p_Object)))
			exit(1);

		ZeroMemory(&p_Params, sizeof(p_Params));

		p_Params.BackBufferCount = 1;
		p_Params.BackBufferWidth = Width;
		p_Params.BackBufferHeight = Height;
		p_Params.BackBufferFormat = D3DFMT_A8R8G8B8;

		p_Params.SwapEffect = D3DSWAPEFFECT_DISCARD;
		p_Params.MultiSampleQuality = D3DMULTISAMPLE_NONE;

		p_Params.hDeviceWindow = hWnd;
		p_Params.Windowed = TRUE;
		
		p_Params.EnableAutoDepthStencil = TRUE;
		p_Params.AutoDepthStencilFormat = D3DFMT_D16;

		p_Params.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

		HRESULT res = p_Object->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &p_Params, 0, &p_Device);
		if (FAILED(res))
			exit(1);

		if (!p_Line)
			D3DXCreateLine(p_Device, &p_Line);

		D3DXCreateFontA(p_Device, 18, 0, 0, 0, false, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Courier New", &pFontSmall);

		for (;;)
		{
			MSG msg;
			if (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE))
			{
				DispatchMessage(&msg);
				TranslateMessage(&msg);
			}
			Sleep(1);
		}
	}
};