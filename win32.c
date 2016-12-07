
// Software renderer

#include <windows.h>
#include <GL/gl.h>

#include <stdio.h>

/*#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080*/

// bool globalRunning = true;
LARGE_INTEGER globalPerformanceFrequency = {0};

typedef struct {
	HWND _window;
	HDC hdc;
	BITMAPINFO bitmapInfo;
	void *videoMemory;
	int windowWidth;
	int windowHeight;
	int backBufferWidth;
	int backBufferHeight;
	bool windowOpen;
} OSState;

OSState *_globalState;

int main (int argc, char**argv);

double GetSeconds () {
	if (!globalPerformanceFrequency.QuadPart) {
		QueryPerformanceFrequency(&globalPerformanceFrequency);
	}
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	double seconds = (double)time.QuadPart / (double)globalPerformanceFrequency.QuadPart;
	return seconds;
}

LRESULT CALLBACK WindowCallback (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;
	switch (message) {
		case WM_CLOSE: {
			_globalState->windowOpen = false;
		} break;
		case WM_DESTROY: {
			_globalState->windowOpen = false;
		} break;
		default: {
			result = DefWindowProc(hwnd, message, wParam, lParam);
		} break;
	}
	return result;
}

typedef struct {
	void (*proc) (void *udata);
} WorkerThreadJob;
typedef struct {
	HANDLE semaphore;
	WorkerThreadJob jobs[1024];
	int jobCount;
} WorkerThreadPool;

// HANDLE semaphoreHandle;
struct {
	int id;
	int value;
} results[100];
int resultCount = 0;
int num = 0;
DWORD WorkerThreadProc (LPVOID udata) {
	WorkerThreadPool *workerThreadPool = (WorkerThreadPool*)udata;
	DWORD threadId = GetThreadId(GetCurrentThread());
	for (;;) {
		WaitForSingleObject(workerThreadPool->semaphore, INFINITE);
		if (workerThreadPool->jobCount > 0) {
			void (*proc) (void *udata) = workerThreadPool->jobs[workerThreadPool->jobCount-1].proc;
			workerThreadPool->jobCount--;
			proc(NULL);
			Sleep(10);
		} else {
			OutputDebugString("thread woke up when no jobs are available\n");
		}
	}
	return 0;
}

void CreateWorkerThreadPool (WorkerThreadPool *workerThreadPool) {
	ZeroStruct(*workerThreadPool);
	workerThreadPool->semaphore = CreateSemaphore(0, 0, 1024, NULL);
	for (int i = 0; i < 4; ++i) {
		DWORD id;
		CreateThread(0, 0, WorkerThreadProc, workerThreadPool, 0, &id);
	}
}

void AddWorkerThreadJob (WorkerThreadPool *workerThreadPool, void (*proc) (void *udata), void *udata) {
	workerThreadPool->jobs[workerThreadPool->jobCount].proc = proc;
	++workerThreadPool->jobCount;
	ReleaseSemaphore(workerThreadPool->semaphore, 1, NULL);
}

void AddResult (void *udata) {
	DWORD threadId = GetThreadId(GetCurrentThread());
	results[resultCount].id = threadId;
	results[resultCount].value = num;
	++resultCount;
	++num;
}

void PollEvents (OSState *os) {
	MSG Message;
	while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
		switch (Message.message) {
			case WM_SYSKEYDOWN:
			case WM_KEYDOWN: {
				switch (Message.wParam) {
					case VK_LEFT: {
						// state.input.leftDown = true;
					} break;
					case VK_RIGHT: {
						// state.input.rightDown = true;
					} break;
				}
			} break;

			case WM_QUIT: {
				_globalState->windowOpen = false;
			} break;
			default:
			{
				TranslateMessage(&Message);
				DispatchMessageA(&Message);
			}
			break;
		}
	}
}

void SwapGLBuffers (OSState *os) {
	SwapBuffers(os->hdc);
}

void StartSoftwareGraphics (OSState *os, int windowWidth, int windowHeight, int backBufferWidth, int backBufferHeight) {
	WorkerThreadPool workerThreads;
	CreateWorkerThreadPool(&workerThreads);
	/*semaphoreHandle = CreateSemaphore(0, 0, 1024, NULL);
	for (int i = 0; i < 4; ++i) {
		DWORD id;
		CreateThread(0, 0, WorkerThreadProc, NULL, 0, &id);
	}

	ReleaseSemaphore(semaphoreHandle, 1, NULL);
	ReleaseSemaphore(semaphoreHandle, 1, NULL);*/

	AddWorkerThreadJob(&workerThreads, AddResult, NULL);
	AddWorkerThreadJob(&workerThreads, AddResult, NULL);
	AddWorkerThreadJob(&workerThreads, AddResult, NULL);
	AddWorkerThreadJob(&workerThreads, AddResult, NULL);
	AddWorkerThreadJob(&workerThreads, AddResult, NULL);
	AddWorkerThreadJob(&workerThreads, AddResult, NULL);
	AddWorkerThreadJob(&workerThreads, AddResult, NULL);
	AddWorkerThreadJob(&workerThreads, AddResult, NULL);
	Sleep(1000);

	for (int i = 0; i < resultCount; ++i) {
		char str[64];
		sprintf(str, "value %i, thread %i\n", results[i].value, results[i].id);
		OutputDebugString(str);
	}

	_globalState = os;

	// QueryPerformanceFrequency(&globalPerformanceFrequency);

	WNDCLASS windowClass = {0};
	windowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
	windowClass.lpfnWndProc = WindowCallback;
	// @note: Apparently getting the hInstance this way can cause issues if used in a dll
	HMODULE hInstance = GetModuleHandle(NULL);
	windowClass.hInstance = hInstance;
	windowClass.lpszClassName = "Win32 window class";
	windowClass.hCursor = LoadCursor(0, IDC_ARROW);

	os->windowWidth = windowWidth;
	os->windowHeight = windowHeight;
	RECT windowRect;
	windowRect.left = 0;
	windowRect.right = windowWidth;
	windowRect.top = 0;
	windowRect.bottom = windowHeight;
	AdjustWindowRectEx(&windowRect, WS_OVERLAPPEDWINDOW|WS_VISIBLE, FALSE, 0);

	if (RegisterClassA(&windowClass)) {
		os->_window = CreateWindowExA(0, windowClass.lpszClassName, "Software renderer", WS_OVERLAPPEDWINDOW|WS_VISIBLE,
			CW_USEDEFAULT, CW_USEDEFAULT,
			windowRect.right-windowRect.left,
			windowRect.bottom-windowRect.top,
			0, 0, hInstance, 0);

		if (os->_window) {
			os->windowOpen = true;
			UpdateWindow(os->_window);

			os->hdc = GetDC(os->_window);

			ZeroStruct(os->bitmapInfo);
			os->bitmapInfo.bmiHeader.biSize = sizeof(os->bitmapInfo.bmiHeader);
			os->backBufferWidth = backBufferWidth;
			os->backBufferHeight = backBufferHeight;
			os->bitmapInfo.bmiHeader.biWidth = os->backBufferWidth;
			os->bitmapInfo.bmiHeader.biHeight = os->backBufferHeight;
			os->bitmapInfo.bmiHeader.biPlanes = 1;
			os->bitmapInfo.bmiHeader.biBitCount = 32; // note: DWORD aligned
			os->bitmapInfo.bmiHeader.biCompression = BI_RGB;

			HBITMAP hBitmap = CreateDIBSection (os->hdc, &os->bitmapInfo, DIB_RGB_COLORS, &os->videoMemory, 0, 0);

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

#if 0
			while (globalRunning) {
				state.input = {};



				

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
#endif

		}
	}
}

/*
	Only supporting 32 bit floats or 8 bit ints(signed or unsigned)
	Should you be able to have different rgba order?
*/
typedef enum {
	PIXEL_FORMAT_FLOAT,
	PIXEL_FORMAT_UINT8,
} SoftwarePixelFormat;

void PresentSoftwareBackBuffer (OSState *os, void *data, SoftwarePixelFormat format, int numComponents) {
	float *video = (float*)data;
	unsigned int *pixels = (unsigned int*)os->videoMemory;
	for (int i = 0; i < os->backBufferWidth*os->backBufferHeight; ++i) {
		unsigned char a = video[(i*numComponents)+3]*255;
		unsigned char r = video[(i*numComponents)+0]*255;
		unsigned char g = video[(i*numComponents)+1]*255;
		unsigned char b = video[(i*numComponents)+2]*255;
		pixels[i] = (a << 24) | (r << 16) | (g << 8) | (b);
		// pixels[i] = (b << 24) | (g << 16) | (r << 8) | (a);
		// pixels[i] = 0xFF00FF00;
		//               aarrggbb
	}
	StretchDIBits(os->hdc, 0, 0, os->windowWidth, os->windowHeight, 0, 0, os->backBufferWidth, os->backBufferHeight, os->videoMemory, &os->bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}

/*int CALLBACK WinMain (HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow) {
	
}*/

int CALLBACK WinMain (HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow) {
	OutputDebugString("YEAH\n");
	// @note: Hopefully these arg variable are always available
	main(__argc, __argv);
}