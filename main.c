
#include "platform.c"
#include "software_renderer.c"

void GLDisplayFrame (OSState *os, State *state) {
	static GLuint backBuffer = 0;
	if (!backBuffer) {
		glGenTextures(1, &backBuffer);
		glBindTexture(GL_TEXTURE_2D, backBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, state->backBufferSize.x, state->backBufferSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
	}

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, backBuffer);
	glTexSubImage2D(GL_TEXTURE_2D,
					0,
					0, 0, state->backBufferSize.x, state->backBufferSize.y,
					GL_RGBA,
					GL_FLOAT,
					state->video);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBegin(GL_QUADS);{
		glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, -1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, 1.0f);
	}glEnd();
}

//#ifdef _WIN32
//int CALLBACK WinMain (HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow)
//#else
int main (int argc, char**argv)
//#endif
{
	freopen("stdout.txt", "a", stdout);

	// StartGPUGraphics();
	// StartCPUGraphics();

	// StartHardwareGraphics();

	FILE *sound = fopen("Sleep_Away.wav", "rb");
	fseek(sound, 0, SEEK_END);
	int soundSize = ftell(sound);
	fseek(sound, 0, SEEK_SET);
	void *soundData = malloc(soundSize);
	fread(soundData, 1, soundSize, sound);
	LoadSoundFromMemory(soundData, soundSize);

	State state = {0};
	Start(&state);

	OSState os = {0};
#if 0
	StartHardwareGraphics(&os, 1280, 720);
#else
	StartSoftwareGraphics(&os, 1280, 720, state.backBufferSize.x, state.backBufferSize.y);
#endif

	InitSound(&os);

	while (os.windowOpen) {
		// state.input = {};
		ZeroStruct(state.input);

		PollEvents(&os);

		Update(&os, &state);

		UpdateSound(&os);

#if 0
		GLDisplayFrame(&os, &state);
		DisplayHardwareGraphics(&os);
#else

		/*static uint *testData = NULL;
		if (!testData) {
			testData = malloc(os.backBufferWidth*os.backBufferHeight*sizeof(uint));
			memset(testData, 0, os.backBufferWidth*os.backBufferHeight*sizeof(uint));
		}
		testData[0] = 0xFF00FFFF;
		testData[1] = 0x00FFFFFF;
		testData[2] = 0xFF0000FF;
		testData[3] = 0x00FF00FF;
		testData[4] = 0x0000FFFF;
		DisplaySoftwareGraphics(&os, testData, PIXEL_FORMAT_UBYTE, 4);*/

		DisplaySoftwareGraphics(&os, state.video, PIXEL_FORMAT_FLOAT, 4);
#endif
	}
}
