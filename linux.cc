
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <time.h>

#include <stdio.h>
#include <stdlib.h>

struct OSState {
	Display *_display;
	Window _window;
};

double GetSeconds () {
	timespec time;
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

void StartSoftwareGraphics (OSState *os, int windowWidth, int windowHeight) {
	Display *xDisplay = XOpenDisplay(NULL);
	if (xDisplay != NULL) {
		// int screen = DefaultScreen(os->_display);
		Window xWindow = XCreateSimpleWindow(xDisplay,
										  RootWindow(xDisplay, DefaultScreen(xDisplay)),
										  0, 0, windowWidth, windowHeight,
										  0,
										  0,
										  0);
		
		XSelectInput(xDisplay, xWindow, ExposureMask|KeyPressMask);
		XMapWindow(xDisplay, xWindow);

		XVisualInfo visualInfos[64];
		int num = 64;
		XGetVisualInfo(xDisplay, 0, visualInfos, &num);
		//printf("num visual info %i \n", num);
		for (int i = 0; i < num; ++i) {
			printf("visual %i %lu, id %lu, depth %i \n", i, (unsigned long)visualInfos[i].visual, visualInfos[i].visualid, visualInfos[i].depth);
			// if (visualInfos[i].visual && visualInfos[i].visual->bits_per_rgb == 24) {
			// 	printf("found a good visual format \n");
			// }
		}

		XSetErrorHandler(XlibErrorHandler);

		int depth = 24;

#if 1
		XVisualInfo visualInfo;
		Visual *visual;
		XMatchVisualInfo(xDisplay, DefaultScreen(xDisplay), depth, DirectColor, &visualInfo);
		visual = visualInfo.visual;
		// visual = visualInfo[0].visual;

		// for (int i = 0; i < num; ++i) {
		// 	printf("visual %i %lu \n", i, (unsigned long)visualInfo[i].visual);
		// }

		if (!visual) {
			printf("XMatchVisualInfo error \n");
		}

		GC gc = XCreateGC(xDisplay, xWindow, 0, 0);
		XImage *image = XCreateImage(xDisplay,
									 visual,
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

		unsigned int *data = (unsigned int*)image->data;
		for (int i = 0; i < (windowWidth*windowHeight); ++i) {
			data[i] = 0xFFFFFFFF;
		}

		if (image) {
			XPutImage(xDisplay, xWindow, gc, image, 0, 0, 0, 0, windowWidth, windowHeight);
			XSync(xDisplay, false);
		} else {
			printf("XPutImage error \n");
		}
#endif

		// Pixmap pixmap = XCreatePixmap(os->_display, screen, windowWidth, windowHeight, 24);
		// XCopyArea(os->_display, pixmap, screen, gc, 0, 0, windowWidth, windowHeight, 0, 0);

		while (true) {
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
	} else {
		printf("Error creating x display \n");
	}
}

void PollEvents (OSState *os) {
	if (XPending(os->_display) > 0) {
		XEvent event;
		XNextEvent(os->_display, &event);

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
}

void SwapGLBuffers (OSState *os) {
	glXSwapBuffers(os->_display, os->_window);
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

			os->_display = xDisplay;
			os->_window = xWindow;

			// while (true) {
				

			// 	// glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
			// 	// glClear(GL_COLOR_BUFFER_BIT);

				

				
			// }
		}
	}
}