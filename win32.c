
#include <windows.h>
#include <GL/gl.h>
#include <dsound.h>

#include <stdio.h>
#include <math.h>

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
	KEYBOARD_ESCAPE = VK_ESCAPE,
} KeyID;

LARGE_INTEGER _globalPerformanceFrequency = {0};

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
		struct {
			int x;
			int y;
		} mouse;
	} input;
	bool soundDisabled;
} OSState;

OSState *_globalState;

#define PrintOut(...) fprintf(stdout, __VA_ARGS__);
#define PrintErr(...) fprintf(stderr, __VA_ARGS__);

int main (int argc, char**argv);

double GetSeconds () {
	if (!_globalPerformanceFrequency.QuadPart) {
		QueryPerformanceFrequency(&_globalPerformanceFrequency);
	}
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	double seconds = (double)time.QuadPart / (double)_globalPerformanceFrequency.QuadPart;
	return seconds;
}

int64 GetTime () {
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	return time.QuadPart;
}

float ConvertToSeconds (int64 time) {
	if (!_globalPerformanceFrequency.QuadPart) {
		QueryPerformanceFrequency(&_globalPerformanceFrequency);
	}
	float seconds = (double)time / (double)_globalPerformanceFrequency.QuadPart;
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

void WorkerThreadTest () {
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
}

void PollEvents (OSState *os) {
	// memset(&os->input.
	//os->input.keysLast[Message.wParam] = os->input.keys[Message.wParam];
	memcpy(os->input.keysLast, os->input.keys, sizeof(os->input.keys));

	POINT mouse;
	GetCursorPos(&mouse);
	ScreenToClient(os->_window, &mouse);
	os->input.mouse.x = mouse.x;
	os->input.mouse.y = mouse.y;

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

void InitSoftwareVideo (OSState *os, int windowWidth, int windowHeight, int backBufferWidth, int backBufferHeight) {
	_globalState = os;

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
			os->bitmapInfo.bmiHeader.biBitCount = 32;
			os->bitmapInfo.bmiHeader.biCompression = BI_RGB;

			HBITMAP hBitmap = CreateDIBSection (os->hdc, &os->bitmapInfo, DIB_RGB_COLORS, &os->videoMemory, 0, 0);
		} else {
			PrintErr("Error while creating window\n");
			goto error;
		}
	} else {
		PrintErr("Error while registering window class\n");
		goto error;
	}

	return;
error:
	MessageBox(os->_window, "There was an error initializing software video", NULL, MB_OK);
	exit(1);
}

void InitOpenglVideo (OSState *os, int windowWidth, int windowHeight) {
	_globalState = os;
	freopen("stdout.txt", "a", stdout);
	freopen("stderr.txt", "a", stderr);

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
		os->_window = CreateWindowExA(0, windowClass.lpszClassName, "OpenGL", WS_OVERLAPPEDWINDOW|WS_VISIBLE,
									  CW_USEDEFAULT, CW_USEDEFAULT,
									  windowRect.right-windowRect.left,
									  windowRect.bottom-windowRect.top,
									  0, 0, hInstance, 0);

		if (os->_window) {
			os->windowOpen = true;
			UpdateWindow(os->_window);

			os->hdc = GetDC(os->_window);

			{
				PIXELFORMATDESCRIPTOR pixelFormat = {0};
				pixelFormat.nSize = sizeof(PIXELFORMATDESCRIPTOR);
				pixelFormat.nVersion = 1;
				pixelFormat.iPixelType = PFD_TYPE_RGBA;
				pixelFormat.dwFlags = PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER;
				pixelFormat.cColorBits = 32;
				pixelFormat.cAlphaBits = 8;
				pixelFormat.iLayerType = PFD_MAIN_PLANE;

				int suggestedIndex = ChoosePixelFormat(os->hdc, &pixelFormat);
				if (!suggestedIndex) {
					PrintErr("ChoosePixelFormat failed\n");
					goto error;
				}
				PIXELFORMATDESCRIPTOR suggested;
				DescribePixelFormat(os->hdc, suggestedIndex, sizeof(PIXELFORMATDESCRIPTOR), &suggested);
				if (!SetPixelFormat(os->hdc, suggestedIndex, &suggested)) {
					PrintErr("SetPixelFormat failed\n");
					goto error;
				}

				HGLRC glContext = wglCreateContext(os->hdc);
				if (!glContext) {
					PrintErr("wglCreateContext failed\n");
					goto error;
				}
				if (!wglMakeCurrent(os->hdc, glContext)) {
					PrintErr("wglMakeCurrent failed\n");
					goto error;
				}
			}
		} else {
			PrintErr("Error while creating window\n");
			goto error;
		}
	} else {
		PrintErr("Error while registering window class\n");
		goto error;
	}

	return;
error:
	MessageBox(os->_window, "There was an error initializing OpenGL video", NULL, MB_OK);
	exit(1);
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

void FinishVideo (OSState *os) {
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
	void *data;
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
			dataChunk->data = f + 8;
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

#define SOUND_SAMPLES_PER_SEC (48000/1)
#define SOUND_MIX_FORWARD 0.05f

/* @todo:
		Need to lock - access to playingSounds
					   access to pause
		Remove critical section from around mixing loop
*/

LPDIRECTSOUND dsound;
LPDIRECTSOUNDBUFFER primaryBuffer;
LPDIRECTSOUNDBUFFER secondaryBuffer;
CRITICAL_SECTION playingSoundsLock;
void InitSound (OSState *os) {
	InitializeCriticalSectionAndSpinCount(&playingSoundsLock, 1024);

	HMODULE dsoundLib = LoadLibraryA("dsound.dll");
	if (!dsoundLib) {
		PrintErr("Error loading dsound.dll\n");
		goto error;
	}

	// HRESULT WINAPI (*)(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
	// HRESULT WINAPI (*DirectSoundCreate)(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
	// 	= GetProcAddress(dsoundLib, "DirectSoundCreate");
	// DirectSoundCreateProc DirectSoundCreate = GetProcAddress(dsoundLib, "DirectSoundCreate");
	DirectSoundCreateProc *DirectSoundCreate = (DirectSoundCreateProc*)GetProcAddress(dsoundLib, "DirectSoundCreate");
	if (!DirectSoundCreate) {
		PrintErr("Error loading DirectSoundCreate proc\n");
		goto error;
	}

	if (!SUCCEEDED(DirectSoundCreate(0, &dsound, 0))) {
		PrintErr("DirectSoundCreate error\n");
		goto error;
	}

	WAVEFORMATEX wave = {0};
	wave.wFormatTag = WAVE_FORMAT_PCM;
	wave.nChannels = 2;
	wave.nSamplesPerSec = SOUND_SAMPLES_PER_SEC;
	wave.wBitsPerSample = 16;
	wave.nBlockAlign = 4;
	wave.nAvgBytesPerSec = SOUND_SAMPLES_PER_SEC * 4;

	if (!SUCCEEDED(IDirectSound_SetCooperativeLevel(dsound, os->_window, DSSCL_PRIORITY))) {
		PrintErr("IDirectSound_SetCooperativeLevel error\n");
		goto error;
	}

	DSBUFFERDESC desc = {0};
	desc.dwSize = sizeof(DSBUFFERDESC);
	desc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	if (!SUCCEEDED(IDirectSound_CreateSoundBuffer(dsound, &desc, &primaryBuffer, 0))) {
		PrintErr("IDirectSound_CreateSoundBuffer error\n");
		goto error;
	}

	if (!SUCCEEDED(IDirectSoundBuffer_SetFormat(primaryBuffer, &wave))) {
		PrintErr("IDirectSoundBuffer_SetFormat error\n");
		goto error;
	}

	DSBUFFERDESC desc2 = {0};
	desc2.dwSize = sizeof(DSBUFFERDESC);
	desc2.dwFlags = DSBCAPS_GLOBALFOCUS|DSBCAPS_GETCURRENTPOSITION2;
	desc2.dwBufferBytes = SOUND_SAMPLES_PER_SEC * 4;
	desc2.lpwfxFormat = &wave;
	if (!SUCCEEDED(IDirectSound_CreateSoundBuffer(dsound, &desc2, &secondaryBuffer, 0))) {
		PrintErr("IDirectSound_CreateSoundBuffer error\n");
		goto error;
	}

	IDirectSoundBuffer_SetCurrentPosition(secondaryBuffer, 0);
	IDirectSoundBuffer_Play(secondaryBuffer, 0, 0, DSBPLAY_LOOPING);

	return;

error:
	PrintErr("Failed to initialize DirectSound\n");
	MessageBox(os->_window, "There was an error initializing audio, continuing without sound", NULL, MB_OK);
	os->soundDisabled = true;
}

int lastChunkWritten = 0;
typedef union {
	struct {
		int16 left;
		int16 right;
	};
	int16 channels[2];
} SoundSample;

typedef struct {
	Sound *sound;
	int numSamples;
	int cursor;
	float cursorFract;
	float fadeSpeed;
	float fade;
} PlayingSound;

enum PlayingSoundFlags {
	SOUND_FADE_IN = (1<<0),
};

PlayingSound playingSounds[64];
int playingSoundCount = 0;

void SoundPlay (Sound *sound, int flags) {
	EnterCriticalSection(&playingSoundsLock);
	if (sound->channels == 2 && sound->bitsPerSample == 16) {
		playingSounds[playingSoundCount].sound = sound;
		playingSounds[playingSoundCount].numSamples = sound->size/4;
		playingSounds[playingSoundCount].cursor = 0;
		playingSounds[playingSoundCount].fadeSpeed = 0.0f;
		playingSounds[playingSoundCount].fade = 1.0f;
		if (flags & SOUND_FADE_IN) {
			playingSounds[playingSoundCount].fadeSpeed = 0.005f;
			playingSounds[playingSoundCount].fade = 0.0f;
		}
		++playingSoundCount;
	} else {
		PrintErr("Currently only 2 channel 16bit audio is supported!\n");
	}
	LeaveCriticalSection(&playingSoundsLock);
}

void FadeOutSounds () {
	EnterCriticalSection(&playingSoundsLock);
	for (int i = 0; i < playingSoundCount; ++i) {
		playingSounds[i].fadeSpeed = -0.005f;
	}
	LeaveCriticalSection(&playingSoundsLock);
}

int GetSoundDeviceWriteCursor () {
	int playCursor;
	int writeCursor;
	HRESULT result;
	if (result = IDirectSoundBuffer_GetCurrentPosition(secondaryBuffer, &playCursor, &writeCursor) != DS_OK) {
		PrintErr("IDirectSoundBuffer_GetCurrentPosition error\n");
		// @todo handle failure
		return 0; // zero?
	}

	return writeCursor / 4; // convert to samples
}

void WriteToSoundDevice (void *buffer, int pos, int size) {
	void *region1;
	uint region1Size;
	void *region2;
	uint region2Size;
	HRESULT r;
	if (!size) {
		PrintErr("paint size is 0\n");
		return;
	}
	if (r = IDirectSoundBuffer_Lock(secondaryBuffer, (pos%SOUND_SAMPLES_PER_SEC)*4, size*4,
								&region1, &region1Size, &region2, &region2Size, 0) != DS_OK) {
		PrintErr("IDirectSoundBuffer_Lock error\n");
		return;
	}

	memcpy(region1, buffer, region1Size);
	if (region2) {
		memcpy(region2, (char*)buffer + (region1Size), region2Size);
	}

	IDirectSoundBuffer_Unlock(secondaryBuffer, region1, region1Size, region2, region2Size);
}

_inline long AtomicRead (volatile long *value) {
	return InterlockedExchangeAdd(value, 0);
}

_inline long AtomicCompareExchange (volatile long *value, long compare, long exchange) {
	return InterlockedCompareExchange(value, exchange, compare);
}

_inline long AtomicExchange (volatile long *value, long exchange) {
	return InterlockedExchange(value, exchange);
}

bool soundPaused = false;
void PauseSound () {
	// soundPaused = true;
	AtomicExchange((long*)&soundPaused, true);
}

void UnpauseSound () {
	// soundPaused = false;
	AtomicExchange((long*)&soundPaused, false);
}

void TogglePauseSound () {
	// if (!soundPaused) soundPaused = true;
	// else soundPaused = false;
	if (AtomicCompareExchange((long*)&soundPaused, false, true) != false) {
		AtomicExchange((long*)&soundPaused, false);
	}
}

// int16 debugSoundBuffer[SOUND_SAMPLES_PER_SEC];
int buffers = 0;
int oldWritePos = 0;
int paintedEnd = 0;
float volume = 0.25f;
int64 oldTime = 0;
SoundSample visualSamples[SOUND_SAMPLES_PER_SEC];
int visualCursor = 0;
void UpdateSound (OSState *os) {
	if (os->soundDisabled) return;

	int writePos = GetSoundDeviceWriteCursor();

	if (writePos < oldWritePos) {
		++buffers;
		// @todo: do the integer wrapping stuff
	}
	int pos = (buffers * SOUND_SAMPLES_PER_SEC) + writePos;
	oldWritePos = writePos;
	int paint = paintedEnd;
	if (paint < pos) paint = pos;

	float mixForward = SOUND_MIX_FORWARD;

	if (!oldTime) oldTime = GetTime();
	int64 time = GetTime();
	float timePassed = ConvertToSeconds(time-oldTime);
	float secondsPerFrame = 1.0f / 60.0f;
	while (timePassed > secondsPerFrame*2.0f) {
		mixForward *= 2.0f;
		secondsPerFrame *= 2.0f;
	}
	if (mixForward > 0.8f) mixForward = 0.8f;
	oldTime = time;

	int end = pos + (SOUND_SAMPLES_PER_SEC * mixForward);
	int paintSize = end - paint;
	if (paintSize < 0) paintSize = 0;

	EnterCriticalSection(&playingSoundsLock);
	for (int i = playingSoundCount-1; i >= 0; --i) {
		playingSounds[i].fade += playingSounds[i].fadeSpeed;
		if (playingSounds[i].fade > 1.0f) {
			playingSounds[i].fade = 1.0f;
			playingSounds[i].fadeSpeed = 0.0f;
		}
		int samplesToPlay = playingSounds[i].numSamples - playingSounds[i].cursor;
		if (samplesToPlay <= 0 || playingSounds[i].fade <= 0.0f) {
			playingSounds[i] = playingSounds[playingSoundCount-1];
			--playingSoundCount;
		}
	}
	
#define BUFFER_SIZE 512
	SoundSample buffer[BUFFER_SIZE];

	// @todo: since we are lerping forward samples you could read 1 sample too many off the end
	while (paintSize > 0) {
		memset(buffer, 0, sizeof(SoundSample)*BUFFER_SIZE);
		int count = paintSize <= BUFFER_SIZE ? paintSize : BUFFER_SIZE;
		bool paused = AtomicRead((long*)&soundPaused);
		if (!paused) {
			int soundCount = AtomicRead(&playingSoundCount);
			for (int i = 0; i < soundCount; ++i) {
				float sampleRateRatio = (double)playingSounds[i].sound->samplesPerSec / (double)SOUND_SAMPLES_PER_SEC;
				int samplesToPlay = (playingSounds[i].numSamples - playingSounds[i].cursor) /** sampleRateRatio*/;
				int size = count;
				if ((samplesToPlay*sampleRateRatio) < size) {
					size = (samplesToPlay*sampleRateRatio);
				}
				SoundSample *input = playingSounds[i].sound->data;
				for (int j = 0; j < size; ++j) {
					SoundSample sample;
					float amount0 = 1.0f - playingSounds[i].cursorFract;
					float amount1 = playingSounds[i].cursorFract;
					sample.left = (input[playingSounds[i].cursor].left*volume*amount0) + (input[playingSounds[i].cursor+1].left*volume*amount1);
					sample.right = (input[playingSounds[i].cursor].right*volume*amount0) + (input[playingSounds[i].cursor+1].right*volume*amount1);
					sample.left *= playingSounds[i].fade;
					sample.right *= playingSounds[i].fade;
					
					{
						playingSounds[i].cursorFract += sampleRateRatio;
						int cursorInt = playingSounds[i].cursorFract;
						playingSounds[i].cursorFract = playingSounds[i].cursorFract - cursorInt;
						playingSounds[i].cursor += cursorInt;
					}

					buffer[j].left += sample.left;
					buffer[j].right += sample.right;
				}
			}
		}
		WriteToSoundDevice(buffer, paint, count);
		paintSize -= count;
		paint += count;

		if (!paused) {
			for (int i = 0; i < count; ++i) {
				visualSamples[visualCursor].left = buffer[i].left;
				visualSamples[visualCursor].right = buffer[i].right;
				++visualCursor;
				visualCursor %= SOUND_SAMPLES_PER_SEC;
			}
		}
	}
	LeaveCriticalSection(&playingSoundsLock);

	paintedEnd = end;
}