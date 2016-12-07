
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <time.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

typedef struct {
	Display *display;
	Window window;
	GC gc;
	XImage *image;
	int windowWidth;
	int windowHeight;
	int backBufferWidth;
	int backBufferHeight;
	bool windowOpen;
} OSState;

double GetSeconds () {
	struct timespec time;
	clock_gettime(CLOCK_REALTIME, &time);
	// printf("sec %li nsec %li \n", time.tv_sec, time.tv_nsec);
	double secs = time.tv_sec + ((double)time.tv_nsec / 1000000000.0);
	// printf("secs %f \n", secs);
	return secs;
}

int XlibErrorHandler (Display *xDisplay, XErrorEvent *xError) {
	printf("fuck off! \n");

	char str[1024];
	XGetErrorText(xDisplay, xError->error_code, str, 1024);
	printf("%s\n", str);
	return 0;
}

void StartSoftwareGraphics (OSState *os, int windowWidth, int windowHeight, int backBufferWidth, int backBufferHeight) {
	Display *xDisplay = XOpenDisplay(NULL);
	if (xDisplay != NULL) {
		int depth = 24;
		XVisualInfo visualInfo;
		XMatchVisualInfo(xDisplay, DefaultScreen(xDisplay), depth, DirectColor, &visualInfo);
		Visual *xVisual = visualInfo.visual;

		Colormap cmap = XCreateColormap(xDisplay, RootWindow(xDisplay, DefaultScreen(xDisplay)), xVisual, AllocAll);
		XSetWindowAttributes attribs = {0};
		attribs.event_mask = KeyPressMask|KeyReleaseMask|ExposureMask;
		attribs.colormap = cmap;
		Window xWindow = XCreateWindow(xDisplay,
									   RootWindow(xDisplay, DefaultScreen(xDisplay)),
									   0, 0, windowWidth, windowHeight,
									   0,
									   depth,
									   InputOutput,
									   xVisual,
									   CWEventMask|CWColormap,
									   &attribs);

		XMapWindow(xDisplay, xWindow);
		XSetErrorHandler(XlibErrorHandler);

		if (!xVisual) {
			printf("XMatchVisualInfo error \n");
		}

		GC gc = XCreateGC(xDisplay, xWindow, 0, 0);
		XImage *image = XCreateImage(xDisplay,
									 xVisual,
									 depth,
									 ZPixmap,
									 0,
									 (char*)malloc(windowWidth*windowHeight*sizeof(int)),
									 windowWidth, windowHeight,
									 32,
									 windowWidth*sizeof(int));
		if (!image) {
			printf("fucking error \n");
		}

#if 0
		while (true) {
			if (XPending(xDisplay)) {
				XEvent event;
				XNextEvent(xDisplay, &event);

				if (event.type == Expose) {
					printf("Expose\n");
				}
				if (event.type == KeyPress) {
					printf("KeyPress\n");
				}
				if (event.type == DestroyNotify) {
					printf("Destroy window\n");
				}
			}

			uint *data = (uint*)image->data;
			for (int i = 0; i < (windowWidth*windowHeight); ++i) {
				uint r = rand()%255;
				uint g = rand()%255;
				uint b = rand()%255;
				uint a = 255;
				/*r = 127;
				g = 0;
				b = 255;*/
				data[i] = (a<<24)|(r<<16)|(g<<8)|b;
			}
			XPutImage(xDisplay, xWindow, gc, image, 0, 0, 0, 0, windowWidth, windowHeight);
			XSync(xDisplay, false);
			// usleep(1000);
		}
#endif

		os->display = xDisplay;
		os->window = xWindow;
		os->gc = gc;
		os->image = image;
		os->windowWidth = windowWidth;
		os->windowHeight = windowHeight;
		os->backBufferWidth = backBufferWidth;
		os->backBufferHeight = backBufferHeight;
		os->windowOpen = true;
	} else {
		printf("Error creating x display \n");
	}
}

typedef enum {
	PIXEL_FORMAT_FLOAT,
	PIXEL_FORMAT_UBYTE,
} SoftwarePixelFormat;

void DisplaySoftwareGraphics (OSState *os, void *data, SoftwarePixelFormat format, int numComponents) {
	uint *dest = (uint*)os->image->data;

#if 0
	for (int i = 0; i < (os->backBufferWidth*os->backBufferHeight); ++i, f+=4) {
		uint8 r = f[0]*255.0f;
		uint8 g = f[1]*255.0f;
		uint8 b = f[2]*255.0f;
		uint8 a = f[3]*255.0f;
		dest[i] = (a<<24)|(r<<16)|(g<<8)|b;
	}
#else
	if (format == PIXEL_FORMAT_FLOAT) {
		float *f = data;
		for (int y = 0; y < os->windowHeight; ++y) {
			for (int x = 0; x < os->windowWidth; ++x) {
				int yy = ((float)y*((float)os->backBufferHeight/(float)os->windowHeight));
				int xx = ((float)x*((float)os->backBufferWidth/(float)os->windowWidth));
				float *ff = f + ((yy*os->backBufferWidth + xx)*numComponents);
				uint8 c[4] = {0};
				c[3] = 255;
				int components = numComponents <= 4 ? numComponents : 4;
				for (int cc = 0; cc < components; ++cc) {
					c[cc] = ff[cc]*255.0f;
				}
				dest[y*os->windowWidth + x] = (c[3]<<24)|(c[0]<<16)|(c[1]<<8)|c[2];
			}
		}
	} else if (format == PIXEL_FORMAT_UBYTE) {
		uint8 *input = data;
		for (int y = 0; y < os->windowHeight; ++y) {
			for (int x = 0; x < os->windowWidth; ++x) {
				int yy = ((float)y*((float)os->backBufferHeight/(float)os->windowHeight));
				int xx = ((float)x*((float)os->backBufferWidth/(float)os->windowWidth));
				uint8 *ii = input + ((yy*os->backBufferWidth + xx)*numComponents);
				uint8 c[4] = {0};
				c[3] = 255;
				int components = numComponents <= 4 ? numComponents : 4;
				for (int cc = 0; cc < components; ++cc) {
					c[cc] = ii[cc];
				}
				dest[y*os->windowWidth + x] = (c[0]<<24)|(c[3]<<16)|(c[2]<<8)|c[1];
			}
		}
	}
#endif

	XPutImage(os->display, os->window, os->gc, os->image, 0, 0, 0, 0, os->windowWidth, os->windowHeight);
	XSync(os->display, false);
}

void PollEvents (OSState *os) {
	if (XPending(os->display) > 0) {
		XEvent event;
		XNextEvent(os->display, &event);

		if (event.type == Expose) {
			printf("Expose\n");
		}
		if (event.type == KeyPress) {
			printf("KeyPress\n");
		}
		if (event.type == DestroyNotify) {
			printf("Destroy window\n");
			os->windowOpen = false;
		}
	}
}

void SwapGLBuffers (OSState *os) {
	glXSwapBuffers(os->display, os->window);
}

void StartHardwareGraphics (OSState *os, int windowWidth, int windowHeight) {
	Display *xDisplay = XOpenDisplay(NULL);
	if (xDisplay != NULL) {
		GLint att[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, 0};
		XVisualInfo *vi = glXChooseVisual(xDisplay, 0, att);

		if (vi) {
			Window rootWindow = DefaultRootWindow(xDisplay);
			Colormap colorMap = XCreateColormap(xDisplay, rootWindow, vi->visual, AllocNone);
			XSetWindowAttributes swa;
			swa.colormap = colorMap;
			swa.event_mask = ExposureMask|KeyPressMask;

			// int screen = DefaultScreen(os->_display);
			Window xWindow = XCreateWindow(xDisplay,
											  rootWindow,
											  0, 0, windowWidth, windowHeight,
											  0,
											  vi->depth,
											  InputOutput,
											  vi->visual,
											  CWColormap|CWEventMask,
											  &swa);
			
			XSelectInput(xDisplay, xWindow, ExposureMask|KeyPressMask);
			XMapWindow(xDisplay, xWindow);

			GLXContext glc = glXCreateContext(xDisplay, vi, NULL, GL_TRUE);
			glXMakeCurrent(xDisplay, xWindow, glc);

			os->display = xDisplay;
			os->window = xWindow;
			os->windowOpen = true;

			// while (true) {
				

			// 	// glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
			// 	// glClear(GL_COLOR_BUFFER_BIT);

				

				
			// }
		}
	}
}
