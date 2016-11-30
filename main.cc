
#include "linux.cc"
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

	SwapBuffers(os);
}

int main () {
	// StartGPUGraphics();
	// StartCPUGraphics();

	// StartHardwareGraphics();

	OSState os = {};
	StartHardwareGraphics(&os, 1280, 720);

	State state = {};
	Start(&state);	

	while (true) {
		PollEvents(&os);

		Update(&state);

		PresentBackBufferOGL(&os, &state);
	}
}