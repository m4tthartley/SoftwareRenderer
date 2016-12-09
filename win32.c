
// Software renderer

#include <windows.h>
#include <GL/gl.h>
#include <dsound.h>

#include <stdio.h>

typedef enum {
	KEYBOARD_A = 'A',
	KEYBOARD_B = 'B',
	KEYBOARD_C = 'C',
	KEYBOARD_D = 'D',
	KEYBOARD_E = 'E',
	KEYBOARD_F = 'F',
	KEYBOARD_G = 'G',
	KEYBOARD_H = 'H',
	KEYBOARD_I = 'I',
	KEYBOARD_J = 'J',
	KEYBOARD_K = 'K',
	KEYBOARD_L = 'L',
	KEYBOARD_M = 'M',
	KEYBOARD_N = 'N',
	KEYBOARD_O = 'O',
	KEYBOARD_P = 'P',
	KEYBOARD_Q = 'Q',
	KEYBOARD_R = 'R',
	KEYBOARD_S = 'S',
	KEYBOARD_T = 'T',
	KEYBOARD_U = 'U',
	KEYBOARD_V = 'V',
	KEYBOARD_W = 'W',
	KEYBOARD_X = 'X',
	KEYBOARD_Y = 'Y',
	KEYBOARD_Z = 'Z',

	KEYBOARD_1 = '1',
	KEYBOARD_2 = '2',
	KEYBOARD_3 = '3',
	KEYBOARD_4 = '4',
	KEYBOARD_5 = '5',
	KEYBOARD_6 = '6',
	KEYBOARD_7 = '7',
	KEYBOARD_8 = '8',
	KEYBOARD_9 = '9',
	KEYBOARD_0 = '0',

	KEYBOARD_LEFT = VK_LEFT,
	KEYBOARD_RIGHT = VK_RIGHT,
	KEYBOARD_UP = VK_UP,
	KEYBOARD_DOWN = VK_DOWN,

	KEYBOARD_CTRL = VK_CONTROL,
	KEYBOARD_LSHIFT = VK_LSHIFT,
	KEYBOARD_RSHIFT = VK_RSHIFT,
	KEYBOARD_ALT = VK_MENU,
	KEYBOARD_CAPS = VK_CAPITAL,
	KEYBOARD_TAB = VK_TAB,
	KEYBOARD_SPACE = VK_SPACE,
	KEYBOARD_RETURN = VK_RETURN,
	KEYBOARD_BACKSPACE = VK_BACK,

} KeyID;

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
	struct {
		bool keys[256];
		bool keysLast[256];
	} input;
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
	// memset(&os->input.
	//os->input.keysLast[Message.wParam] = os->input.keys[Message.wParam];
	memcpy(os->input.keysLast, os->input.keys, sizeof(os->input.keys));

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

				os->input.keys[Message.wParam] = true;
			} break;

			case WM_SYSKEYUP:
			case WM_KEYUP: {
				os->input.keys[Message.wParam] = false;
			} break;

			case WM_QUIT: {
				_globalState->windowOpen = false;
			} break;
			default: {
				TranslateMessage(&Message);
				DispatchMessageA(&Message);
			}
			break;
		}
	}
}

bool KeyDown (OSState *os, KeyID key) {
	return os->input.keys[key];
}

bool KeyPressed (OSState *os, KeyID key) {
	return os->input.keys[key] && !os->input.keysLast[key];
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
	PIXEL_FORMAT_UBYTE,
} SoftwarePixelFormat;

void DisplaySoftwareGraphics (OSState *os, void *data, SoftwarePixelFormat format, int numComponents) {
	unsigned int *pixels = (unsigned int*)os->videoMemory;

	if (format == PIXEL_FORMAT_FLOAT) {
		float *video = data;
		for (int i = 0; i < os->backBufferWidth*os->backBufferHeight; ++i) {
			float *v = video + (i*numComponents);
			int components = numComponents >= 4 ? numComponents : 4;
			uint8 c[4] = {0};
			c[3] = 255;
			for (int cc = 0; cc < components; ++cc) {
				c[cc] = v[cc]*255.0f;
			}
			pixels[i] = (c[3] << 24) | (c[0] << 16) | (c[1] << 8) | (c[2]);
			// aarrggbb
		}
	} else if (format == PIXEL_FORMAT_UBYTE) {
		uint8 *video = data;
		for (int i = 0; i < os->backBufferWidth*os->backBufferHeight; ++i) {
			uint8 *v = video + (i*numComponents);
			int components = numComponents >= 4 ? numComponents : 4;
			uint8 c[4] = {0};
			c[3] = 255;
			for (int cc = 0; cc < components; ++cc) {
				c[cc] = v[cc];
			}
			pixels[i] = (c[0] << 24) | (c[3] << 16) | (c[2] << 8) | (c[1]);
			// aarrggbb
		}
	}
	StretchDIBits(os->hdc, 0, 0, os->windowWidth, os->windowHeight, 0, 0, os->backBufferWidth, os->backBufferHeight, os->videoMemory, &os->bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}

void DisplayHardwareGraphics (OSState *os) {
	SwapBuffers(os->hdc);
}

/*int CALLBACK WinMain (HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow) {
	
}*/

int CALLBACK WinMain (HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow) {
	// @note: Hopefully these arg variable are always available
	main(__argc, __argv);
}

// Audio

#pragma pack(push, 1)
typedef struct {
	char ChunkId[4];
	uint ChunkSize;
	char WaveId[4];
} WavHeader;
typedef struct {
	char id[4];
	uint size;
	uint16 formatTag;
	uint16 channels;
	uint samplesPerSec;
	uint bytesPerSec;
	uint16 blockAlign;
	uint16 bitsPerSample;
	uint16 cbSize;
	int16 validBitsPerSample;
	int channelMask;
	char subFormat[16];
} WavFormatChunk;
typedef struct {
	char id[4];
	uint size;
	int16 *data;
	char padByte;
} WavDataChunk;
/*typedef struct {
	WavHeader header;
	WavFormatChunk format;
	int16 *data;
	uint dataSize;
	file_data file;
} WavData;*/
#pragma pack(pop)

typedef struct {
	int channels;
	int samplesPerSec;
	int bitsPerSample;
	void *data;
	size_t size;
} Sound;

Sound LoadSoundFromMemory (void *data, size_t size) {
	WavHeader *header = data;
	WavFormatChunk *format;
	WavDataChunk *dataChunk;
	char *f = (char*)(header + 1);
	while (f < (char*)data + size) {
		int id = *(int*)f;
		uint size = *(uint*)(f+4);
		if (id == (('f'<<0)|('m'<<8)|('t'<<16)|(' '<<24))) {
			format = (WavFormatChunk*)f;
		}
		if (id == (('d'<<0)|('a'<<8)|('t'<<16)|('a'<<24))) {
			dataChunk = (WavDataChunk*)f;
		}
		f += size + 8;
	}

	Sound sound;
	sound.channels = format->channels;
	sound.samplesPerSec = format->samplesPerSec;
	sound.bitsPerSample = format->bitsPerSample;
	sound.data = dataChunk->data;
	sound.size = dataChunk->size;
	return sound;
}

typedef HRESULT WINAPI DirectSoundCreateProc (LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);

LPDIRECTSOUND dsound;
LPDIRECTSOUNDBUFFER primaryBuffer;
LPDIRECTSOUNDBUFFER secondaryBuffer;
void InitSound (OSState *os) {
	HMODULE dsoundLib = LoadLibraryA("dsound.dll");
	if (!dsoundLib) {
		printf("Error loading dsound.dll\n");
		goto error;
	}
		// HRESULT WINAPI (*)(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
		// HRESULT WINAPI (*DirectSoundCreate)(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
		// 	= GetProcAddress(dsoundLib, "DirectSoundCreate");
		// DirectSoundCreateProc DirectSoundCreate = GetProcAddress(dsoundLib, "DirectSoundCreate");
	DirectSoundCreateProc *DirectSoundCreate = (DirectSoundCreateProc*)GetProcAddress(dsoundLib, "DirectSoundCreate");
	if (!DirectSoundCreate) {
		printf("Error loading DirectSoundCreate proc\n");
		goto error;
	}

	if (!SUCCEEDED(DirectSoundCreate(0, &dsound, 0))) {
		printf("DirectSoundCreate error\n");
		goto error;
	}

	WAVEFORMATEX wave = {0};
	wave.wFormatTag = WAVE_FORMAT_PCM;
	wave.nChannels = 2;
	wave.nSamplesPerSec = 48000;
	wave.wBitsPerSample = 16;
	wave.nBlockAlign = 4;
	wave.nAvgBytesPerSec = 48000 * 4;

	if (!SUCCEEDED(IDirectSound_SetCooperativeLevel(dsound, os->_window, DSSCL_PRIORITY))) {
		printf("IDirectSound_SetCooperativeLevel error\n");
		goto error;
	}

	DSBUFFERDESC desc = {0};
	desc.dwSize = sizeof(DSBUFFERDESC);
	desc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	if (!SUCCEEDED(IDirectSound_CreateSoundBuffer(dsound, &desc, &primaryBuffer, 0))) {
		printf("IDirectSound_CreateSoundBuffer error\n");
		goto error;
	}

	if (!SUCCEEDED(IDirectSoundBuffer_SetFormat(primaryBuffer, &wave))) {
		printf("IDirectSoundBuffer_SetFormat error\n");
		goto error;
	}

	DSBUFFERDESC desc2 = {0};
	desc2.dwSize = sizeof(DSBUFFERDESC);
	desc2.dwFlags = DSBCAPS_GLOBALFOCUS|DSBCAPS_GETCURRENTPOSITION2;
	desc2.dwBufferBytes = 48000 * 4;
	desc2.lpwfxFormat = &wave;
	if (!SUCCEEDED(IDirectSound_CreateSoundBuffer(dsound, &desc2, &secondaryBuffer, 0))) {
		printf("IDirectSound_CreateSoundBuffer error\n");
		goto error;
	}

	IDirectSoundBuffer_SetCurrentPosition(secondaryBuffer, 0);
	IDirectSoundBuffer_Play(secondaryBuffer, 0, 0, DSBPLAY_LOOPING);

	printf("Initialized DirectSound\n");
	return;

error:
	printf("Failed to initialize DirectSound\n");
}

int lastChunkWritten = 0;
void UpdateSound (OSState *os) {
	uint playCursor;
	uint writeCursor;
	HRESULT result;
	if (result = IDirectSoundBuffer_GetCurrentPosition(secondaryBuffer, &playCursor, &writeCursor) != DS_OK) {
		printf("IDirectSoundBuffer_GetCurrentPosition error\n");
		return;
	}

	int chunkSize = 4800;
	int chunk = (writeCursor/4 / (chunkSize)) + 1;
	int lock = chunk*chunkSize*4;
	if (chunk != lastChunkWritten) {
		void *region1;
		uint region1Size;
		void *region2;
		uint region2Size;
		if (IDirectSoundBuffer_Lock(secondaryBuffer, lock, chunkSize*4,
									&region1, &region1Size, &region2, &region2Size, 0) != DS_OK) {
			printf("IDirectSoundBuffer_Lock error\n");
			return;
		}

		static float amp = 0.0f;
		int16 *sample = region1;
		for (int i = 0; i < region1Size/4; ++i) {
			amp += 0.01f;
			sample[0] = (sinf(amp) * 100/*((float)0x7FFF/20)*/);
			sample[1] = (sinf(amp) * 100/*((float)0x7FFF/20)*/);
			sample += 2;
		}
		if (region2) {
			int16 *sample = region2;
			for (int i = 0; i < region2Size/4; ++i) {
				amp += 0.01f;
				sample[0] = (sinf(amp) * 100/*((float)0x7FFF/20)*/);
				sample[1] = (sinf(amp) * 100/*((float)0x7FFF/20)*/);
				sample += 2;
			}
		}

		IDirectSoundBuffer_Unlock(secondaryBuffer, region1, region1Size, region2, region2Size);
		lastChunkWritten = chunk;
	}
}