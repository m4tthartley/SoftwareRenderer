
// Software renderer

#include <windows.h>
#include <stdio.h>

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

bool globalRunning = true;
LARGE_INTEGER globalPerformanceFrequency;

double GetSeconds () {
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	double seconds = (double)time.QuadPart / (double)globalPerformanceFrequency.QuadPart;
	return seconds;
}

#include "software_renderer.cc"

LRESULT CALLBACK WindowCallback (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;
	switch (message) {
		case WM_CLOSE: {
			globalRunning = false;
		} break;
		case WM_DESTROY: {
			globalRunning = false;
		} break;
		default: {
			result = DefWindowProc(hwnd, message, wParam, lParam);
		} break;
	}
	return result;
}

int CALLBACK WinMain (HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow) {
	QueryPerformanceFrequency(&globalPerformanceFrequency);

	WNDCLASS windowClass = {};
	windowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
	windowClass.lpfnWndProc = WindowCallback;
	windowClass.hInstance = hInstance;
	windowClass.lpszClassName = "Win32 window class";
	windowClass.hCursor = LoadCursor(0, IDC_ARROW);

	RECT windowRect;
	windowRect.left = 0;
	windowRect.right = WINDOW_WIDTH;
	windowRect.top = 0;
	windowRect.bottom = WINDOW_HEIGHT;
	AdjustWindowRectEx(&windowRect, WS_OVERLAPPEDWINDOW|WS_VISIBLE, FALSE, 0);

	State state = {};

	if (RegisterClassA(&windowClass)) {
		HWND window = CreateWindowExA(0, windowClass.lpszClassName, "Software renderer", WS_OVERLAPPEDWINDOW|WS_VISIBLE,
			CW_USEDEFAULT, CW_USEDEFAULT,
			windowRect.right-windowRect.left,
			windowRect.bottom-windowRect.top,
			0, 0, hInstance, 0);

		if (window) {
			UpdateWindow(window);

			Start(&state);

			HDC hdc = GetDC(window);

			BITMAPINFO bitmapInfo = {};
			bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
			bitmapInfo.bmiHeader.biWidth = state.backBufferSize.x;
			bitmapInfo.bmiHeader.biHeight = state.backBufferSize.y;
			bitmapInfo.bmiHeader.biPlanes = 1;
			bitmapInfo.bmiHeader.biBitCount = 32; // note: DWORD aligned
			bitmapInfo.bmiHeader.biCompression = BI_RGB;

			void *videoMemory;
			HBITMAP hBitmap = CreateDIBSection (hdc, &bitmapInfo, DIB_RGB_COLORS, &videoMemory, 0, 0);

			while (globalRunning) {
				state.input = {};

#if 0
				POINT mouse;
				GetCursorPos(&mouse);
				ScreenToClient(window, &mouse);
				mouse.x /= (WINDOW_WIDTH/state.backBufferSize.x);
				mouse.y /= (WINDOW_HEIGHT/state.backBufferSize.y);
				char str[64];
				sprintf(str, "mouse %i %i \n", mouse.x, mouse.y);
				OutputDebugString(str);
#endif

				MSG Message;
				while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
					switch (Message.message) {
						case WM_SYSKEYDOWN:
						case WM_KEYDOWN: {
							switch (Message.wParam) {
								case VK_LEFT: {
									state.input.leftDown = true;
								} break;
								case VK_RIGHT: {
									state.input.rightDown = true;
								} break;
							}
						} break;

						case WM_QUIT: {
							globalRunning = false;
						} break;
						default:
						{
							TranslateMessage(&Message);
							DispatchMessageA(&Message);
						}
						break;
					}
				}

				Update(&state);

				/*unsigned char red[4] = {255, 0, 0, 0};
				unsigned int *pixels = (unsigned int*)videoMemory;
				for (int i = 0; i < 640*360; ++i) {
					unsigned char r = rand();
					unsigned char g = rand();
					unsigned char b = rand();
					pixels[i] = (0 << 24) | (r << 16) | (g << 8) | (b);
					// pixels[i] = *(unsigned int *)red;
				}*/

				// pixel[0] = red[0];
				// pixel[1] = red[1];
				// pixel[2] = red[2];
				// pixel[3] = red[3];
				// pixel[4] = red[4];
				// pixel[5] = red[5];
				// pixel[6] = red[6];
				// pixel[7] = red[7];

				PAINTSTRUCT paint;
				// HDC hdc = BeginPaint(window, &paint);

				unsigned int *pixels = (unsigned int*)videoMemory;
				for (int i = 0; i < state.backBufferSize.x*state.backBufferSize.y; ++i) {
					unsigned char a = state.video[i].a*255;
					unsigned char r = state.video[i].r*255;
					unsigned char g = state.video[i].g*255;
					unsigned char b = state.video[i].b*255;
					pixels[i] = (a << 24) | (r << 16) | (g << 8) | (b);
				}
				StretchDIBits(hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, state.backBufferSize.x, state.backBufferSize.y, videoMemory, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
				// BitBlt(hdc, 0, 0, 300, 300, hdc, 0, 0, SRCCOPY);

				// EndPaint(window, &paint);

			}

		}
	}
}