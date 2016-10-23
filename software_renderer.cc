
#include <math.h>

struct Color {
	float r;
	float g;
	float b;
	float a;
};

struct Vec2 {
	float x;
	float y;
};

struct IVec2 {
	int x;
	int y;
};

struct State {
	Color *video;
	struct {
		int x;
		int y;
	} backBufferSize;
};

float RandomFloat () {
	int r = rand() % 1000;
	float result = (float)r / 1000.0f;
	return result;
}

int Min (int a, int b) {
	if (a < b) {
		return a;
	} else {
		return b;
	}
}

int Max (int a, int b) {
	if (a > b) {
		return a;
	} else {
		return b;
	}
}

float Min3 (float a, float b, float c) {
	if (a < b) {
		if (a < c) {
			return a;
		} else {
			return c;
		}
	} else {
		if (b < c) {
			return b;
		} else {
			return c;
		}
	}
}

float Max3 (float a, float b, float c) {
	if (a > b) {
		if (a > c) {
			return a;
		} else {
			return c;
		}
	} else {
		if (b > c) {
			return b;
		} else {
			return c;
		}
	}
}

// todo: Not sure this lerp technique is correct
Color ColorLerp (Color c0, Color c1, float t) {
	Color result;
	result.r = c0.r*(1.0f-t) + c1.r*t;
	result.g = c0.g*(1.0f-t) + c1.g*t;
	result.b = c0.b*(1.0f-t) + c1.b*t;
	return result;
}

Color ColorMul (Color c, float mul) {
	Color result;
	result.r = c.r * mul;
	result.g = c.g * mul;
	result.b = c.b * mul;
	result.a = c.a * mul;
	return result;
}

Color operator+ (Color c0, Color c1) {
	Color result;
	result.r = c0.r+c1.r;
	result.g = c0.g+c1.g;
	result.b = c0.b+c1.b;
	result.a = c0.a+c1.a;
	return result;
}

void Start (State *state) {
	state->backBufferSize.x = 640; // 1920 960 480
	state->backBufferSize.y = 360; // 1080 540 270
	state->video = (Color*)VirtualAlloc(NULL, sizeof(Color)*state->backBufferSize.x*state->backBufferSize.y, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
}

void GetPixelCoords (State *state, float x, float y, int *outX, int *outY) {
	*outX = (x/2.0f + 0.5f) * state->backBufferSize.x;
	*outY = (y/2.0f + 0.5f) * state->backBufferSize.y;
}

IVec2 GetPixelCoords (State *state, float x, float y) {
	IVec2 result;
	result.x = (x/2.0f + 0.5f) * state->backBufferSize.x;
	result.y = (y/2.0f + 0.5f) * state->backBufferSize.y;
	return result;
}

Vec2 GetScreenCoords (State *state, int x, int y) {
	Vec2 result;
	result.x = ((float)x/state->backBufferSize.x)*2.0f - 1.0f;
	result.y = ((float)y/state->backBufferSize.y)*2.0f - 1.0f;
	return result;
}

#define VideoMemoryOffset(x, y) (y*state->backBufferSize.x + x)
#define InVideoMemoryBounds(x, y) (y >= 0 && y < state->backBufferSize.y &&\
								   x >= 0 && x < state->backBufferSize.x)

void DrawRectangle (State *state, float x, float y, float x2, float y2, Color color) {
	int startX;
	int startY;
	int endX;
	int endY;
	GetPixelCoords(state, x, y, &startX, &startY);
	GetPixelCoords(state, x2, y2, &endX, &endY);

	int sx = Min(startX, endX);
	int ex = Max(startX, endX);
	int sy = Min(startY, endY);
	int ey = Max(startY, endY);

	for (int y = sy; y < ey; ++y) {
		for (int x = sx; x < ex; ++x) {
			if (y >= 0 && y < state->backBufferSize.y &&
				x >= 0 && x < state->backBufferSize.x) {
				state->video[y*state->backBufferSize.x + x] = color;
			}
		}
	}
}

/*
	This edge function is from The ryg blog.
	https://fgiesen.wordpress.com/2013/02/08/triangle-rasterization-in-practice/
*/
float LineOrientation (Vec2 l0, Vec2 l1, Vec2 p) {
	float result = (l1.x-l0.x)*(p.y-l0.y) - (l1.y-l0.y)*(p.x-l0.x);
	return result;
}

Vec2 RotateVector (Vec2 v, float rads) {
	Vec2 result;
	result.x = v.x*cosf(rads) + v.y*sinf(rads);
	result.y = v.y*cosf(rads) - v.x*sinf(rads);
	return result;
}

float LineLength (Vec2 v0, Vec2 v1) {
	Vec2 d;
	d.x = v1.x - v0.x;
	d.y = v1.y - v0.y;
	return sqrt(d.x*d.x + d.y*d.y);
}

// todo: Is this the best way?
float TriangleArea (Vec2 v0, Vec2 v1, Vec2 v2) {
	float a = LineLength(v0, v1);
	float b = LineLength(v1, v2);
	float c = LineLength(v2, v0);
	float s = (a+b+c) / 2;
	float result = sqrt(s*(s-a)*(s-b)*(s-c));
	return result;
}

void DrawTriangle2 (State *state, float *verts, float *colors, Color color, float rotation = 0.0f) {
	Vec2 *v = (Vec2*)verts;
	Color *c = (Color*)colors;

	Vec2 v0 = RotateVector(v[0], rotation);
	Vec2 v1 = RotateVector(v[1], rotation);
	Vec2 v2 = RotateVector(v[2], rotation);

	float left = Min3(v0.x, v1.x, v2.x);
	float right = Max3(v0.x, v1.x, v2.x);
	float top = Max3(v0.y, v1.y, v2.y);
	float bottom = Min3(v0.y, v1.y, v2.y);

	IVec2 start = GetPixelCoords(state, left, bottom);
	IVec2 end = GetPixelCoords(state, right, top);

	float triArea = TriangleArea(v0, v1, v2);

	for (int y = start.y; y < end.y; ++y) {
		for (int x = start.x; x < end.x; ++x) {
			if (InVideoMemoryBounds(x, y)) {
				Vec2 p = GetScreenCoords(state, x, y);
				if (LineOrientation(v0, v1, p) <= 0 &&
					LineOrientation(v1, v2, p) <= 0 &&
					LineOrientation(v2, v0, p) <= 0) {
					float c0 = TriangleArea(p, v1, v2) / triArea;
					float c1 = TriangleArea(p, v2, v0) / triArea;
					float c2 = TriangleArea(p, v0, v1) / triArea;
					// Color c = ColorLerp({1, 0, 0}, {0, 1, 0}, area0/triArea);
					Color color = ColorMul(c[0], c0) + ColorMul(c[1], c1) + ColorMul(c[2], c2);
					state->video[VideoMemoryOffset(x, y)] = color;
				}
			}
		}
	}
}

void ClearBackBuffer (State *state, Color color) {
	for (int i = 0; i < state->backBufferSize.x*state->backBufferSize.y; ++i) {
		state->video[i] = color;
	}
}

void Update (State *state) {
	Color red = {255, 0, 0, 255};
	/*for (int i = 0; i < 640*360; ++i) {
		// state->video[i] = {RandomFloat(), RandomFloat(), RandomFloat(), 255};
		state->video[i] = {0, 1, 0, 1};
	}*/

	// DrawRectangle(state, -0.5f, 0.5f, 0.5f, -0.5f, {0, 1, 0, 1});

	ClearBackBuffer(state, {0, 0, 0, 0});

	float triVerts[] = {
		-0.5f, 0.5f,
		0.5f, 0.0f,
		-0.25, -0.5f,
	};
	float colors[] = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
	};
	static float rotation = 0.0f;
	rotation += 0.005f;
	DrawTriangle2(state, triVerts, colors, {0, 1, 1, 1}, rotation);
}