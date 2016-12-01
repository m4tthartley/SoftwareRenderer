
#include "platform.cc"
#include "software_renderer.cc"

void PresentBackBufferOGL (OSState *os, State *state) {
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

	SwapGLBuffers(os);
}

#ifdef _WIN32
int CALLBACK WinMain (HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow)
#else
int main ()
#endif
{
	// StartGPUGraphics();
	// StartCPUGraphics();

	// StartHardwareGraphics();

	State state = {};
	Start(&state);

	OSState os = {};
	// StartHardwareGraphics(&os, 1280, 720);
	StartSoftwareGraphics(&os, 1280, 720, state.backBufferSize.x, state.backBufferSize.y);

	while (os.windowOpen) {
		state.input = {};

		PollEvents(&os);

		Update(&state);

#if 0
		PresentBackBufferOGL(&os, &state);
#else
		state.video[0] = {1, 0, 0, 1};
		state.video[1] = {0, 1, 0, 1};
		state.video[2] = {0, 0, 1, 1};
		PresentSoftwareBackBuffer(&os, state.video, PIXEL_FORMAT_FLOAT, 4);
#endif
	}
}