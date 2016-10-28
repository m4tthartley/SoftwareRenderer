
#undef near
#undef far

#include <math.h>
#include <malloc.h>

typedef float real;
typedef double real64;

#define Assert(expression) if (!(expression)) { *((int*)0) = 0; }

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

union Vec3 {
	struct {
		float x;
		float y;
		float z;
	};
	struct {
		Vec2 xy;
		float z;
	};
	struct {
		float e[3];
	};
};

struct IVec2 {
	int x;
	int y;
};

struct DebugPixel {
	int msaa;
};
struct DebugData {
	DebugPixel *video;
	bool enabled;
};

struct State {
	Color *video;
	struct {
		int x;
		int y;
	} backBufferSize;
	double dt;
	double lastTime;
	DebugData debug;

	struct {
		bool rightDown;
		bool leftDown;
	} input;
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
	result.r = c0.r + c1.r;
	result.g = c0.g + c1.g;
	result.b = c0.b + c1.b;
	result.a = c0.a + c1.a;
	return result;
}

void operator+= (Color &c0, Color c1) {
	c0.r += c1.r;
	c0.g += c1.g;
	c0.b += c1.b;
	c0.a += c1.a;
}

float FloatFract (float f) {
	int i = f;
	float result = f - (float)i;
	return result;
}

#define PI 3.14159265359f

float Rads (float degs) {
	float result = (degs / 180.0f) * PI;
	return result;
}

void Start (State *state) {
	state->backBufferSize.x = 480; // 1920 960 480
	state->backBufferSize.y = 270; // 1080 540 270
	int videoMemorySize = sizeof(Color)*state->backBufferSize.x*state->backBufferSize.y;
	int debugVideoSize = sizeof(DebugPixel)*state->backBufferSize.x*state->backBufferSize.y;
	state->video = (Color*)VirtualAlloc(NULL, videoMemorySize+debugVideoSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	state->debug.video = (DebugPixel*)(state->video + (state->backBufferSize.x*state->backBufferSize.y));

	state->lastTime = GetSeconds();
}

/*void GetPixelCoords (State *state, float x, float y, int *outX, int *outY) {
	*outX = (x/2.0f + 0.5f) * state->backBufferSize.x;
	*outY = (y/2.0f + 0.5f) * state->backBufferSize.y;
}*/

Vec2 GetPixelCoords (State *state, float x, float y) {
	Vec2 result;
	result.x = (x/2.0f + 0.5f) * state->backBufferSize.x;
	result.y = (y/2.0f + 0.5f) * state->backBufferSize.y;
	return result;
}

Vec2 GetScreenCoords (State *state, float x, float y) {
	Vec2 result;
	result.x = ((float)x/state->backBufferSize.x)*2.0f - 1.0f;
	result.y = ((float)y/state->backBufferSize.y)*2.0f - 1.0f;
	return result;
}

void OrthoProjection (State *state, Vec3 *v) {
	float aspect = (float)state->backBufferSize.x / (float)state->backBufferSize.y;
	v->x = v->x / aspect;
	int x = 0;
}

void PerspectiveProjection (State *state, Vec3 *v, float fov) {
	float f = 1.0f / tanf(Rads(fov) / 2.0f);
	float near = -1.0f;
	float far = 1.0f;
	float aspect = state->backBufferSize.x/state->backBufferSize.y;
	float m[] = {
		f / aspect, 0, 0, 0,
		0, f, 0, 0,
		0, 0, (far + near) / (near - far), -1,
		0, 0, (2.0f * far * near) / (near - far), 0,
	};

	float w = 0.0f;
	// v->x = m[0]*v->x + m[1]*v->y + m[2]*v->z + w*m[3];
	// v->y = m[4]*v->x + m[5]*v->y + m[6]*v->z + w*m[7];
	// v->z = m[8]*v->x + m[9]*v->y + m[10]*v->z + w*m[11];
	v->x = m[0]*v->x + m[4]*v->y + m[8]*v->z;
	v->y = m[1]*v->x + m[5]*v->y + m[9]*v->z;
	v->z = m[2]*v->x + m[6]*v->y + m[10]*v->z;
}

#define VideoMemoryOffset(coordX, coordY) ((coordY)*state->backBufferSize.x + (coordX))
#define InVideoMemoryBounds(coordX, coordY) (coordY >= 0 && coordY < state->backBufferSize.y &&\
											 coordX >= 0 && coordX < state->backBufferSize.x)

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
	return sqrtf(d.x*d.x + d.y*d.y);
}

// todo: Is this the best way?
float TriangleArea (Vec2 v0, Vec2 v1, Vec2 v2) {
	float a = LineLength(v0, v1);
	float b = LineLength(v1, v2);
	float c = LineLength(v2, v0);
	float s = (a+b+c) / 2;
	float square = s*(s-a)*(s-b)*(s-c);
	if (square > 0.0f) {
		return sqrtf(square);
	} else {
		return 0.0f;
	}
}

void RasterizeTriangle (State *state, float *verts, int vectorSize, int count, float *colors, float rotation = 0.0f) {
	Vec3 *v = (Vec3*)verts;
	Color *c = (Color*)colors;

	for (int i = 0; i < count/3; ++i) {
		Vec3 v0 = v[0];
		Vec3 v1 = v[1];
		Vec3 v2 = v[2];

		float left = Min3(v0.x, v1.x, v2.x);
		float right = Max3(v0.x, v1.x, v2.x);
		float top = Max3(v0.y, v1.y, v2.y);
		float bottom = Min3(v0.y, v1.y, v2.y);

		Vec2 start = GetPixelCoords(state, left, bottom);
		Vec2 end = GetPixelCoords(state, right, top);

		float triArea = TriangleArea(v0.xy, v1.xy, v2.xy);

		for (int y = (int)start.y; y < (int)end.y+1; ++y) {
			for (int x = (int)start.x; x < (int)end.x+1; ++x) {
				if (InVideoMemoryBounds(x, y)) {

					int fillSamples = 0;
					for (int sampleX = 0; sampleX < 2; ++sampleX) {
						for (int sampleY = 0; sampleY < 2; ++sampleY) {
							Vec2 p = GetScreenCoords(state, (x+0.5f)+((float)sampleX*0.5f), (y+0.5f)+((float)sampleY*0.5f));
							// float lo0 = LineOrientation(v0, v1, p);
							// float lo1 = LineOrientation(v1, v2, p);
							// float lo2 = LineOrientation(v2, v0, p);

							#define lo(l0x, l0y, l1x, l1y, px, py) ((l1x-l0x)*(py-l0y) - (l1y-l0y)*(px-l0x))
							float lo0 = lo(v0.x, v0.y, v1.x, v1.y, p.x, p.y);
							float lo1 = lo(v1.x, v1.y, v2.x, v2.y, p.x, p.y);
							float lo2 = lo(v2.x, v2.y, v0.x, v0.y, p.x, p.y);

							if (lo0 <= 0 && lo1 <= 0 && lo2 <= 0) {
								++fillSamples;
							}
						}
					}

					if (fillSamples > 0) {
						Vec2 p = GetScreenCoords(state, x, y);
						float c0 = TriangleArea(p, v1.xy, v2.xy) / triArea;
						float c1 = TriangleArea(p, v2.xy, v0.xy) / triArea;
						float c2 = TriangleArea(p, v0.xy, v1.xy) / triArea;
						Color color;
						if (c) {
							color = ColorMul(c[0], c0) + ColorMul(c[1], c1) + ColorMul(c[2], c2);
						} else {
							color = {1, 1, 1, 0};
						}

						state->video[VideoMemoryOffset(x, y)] += ColorMul(color, /*1.0f*/(float)fillSamples / 4.0f);

						if (state->video[VideoMemoryOffset(x, y)].r > 1.0f) state->video[VideoMemoryOffset(x, y)].r = 1.0f;
						if (state->video[VideoMemoryOffset(x, y)].g > 1.0f) state->video[VideoMemoryOffset(x, y)].g = 1.0f;
						if (state->video[VideoMemoryOffset(x, y)].b > 1.0f) state->video[VideoMemoryOffset(x, y)].b = 1.0f;
						if (state->video[VideoMemoryOffset(x, y)].a > 1.0f) state->video[VideoMemoryOffset(x, y)].a = 1.0f;

						/*int videoOffset = VideoMemoryOffset(x, y);
						if (fillSamples < state->debug.video[videoOffset].msaa-1 ||
							fillSamples > state->debug.video[videoOffset].msaa+1) {
							Assert(false);
						}*/
						state->debug.video[VideoMemoryOffset(x, y)].msaa = fillSamples;
					}
	#if 0
					Vec2 p = GetScreenCoords(state, x, y);
					float lo0 = LineOrientation(v0, v1, p);
					float lo1 = LineOrientation(v1, v2, p);
					float lo2 = LineOrientation(v2, v0, p);
					if (lo0 <= 0 && lo1 <= 0 && lo2 <= 0) {
						// todo: This means 12 sqrts per pixel! Not great
						float c0 = TriangleArea(p, v1, v2) / triArea;
						float c1 = TriangleArea(p, v2, v0) / triArea;
						float c2 = TriangleArea(p, v0, v1) / triArea;
						// Color c = ColorLerp({1, 0, 0}, {0, 1, 0}, area0/triArea);
						Color color = ColorMul(c[0], c0) + ColorMul(c[1], c1) + ColorMul(c[2], c2);

						// float interpX = 1.0f - modf(

						/*float orient = Max3(lo0, lo1, lo2);
						Vec2 fakeVec = GetPixelCoords(state, -1.0f + orient, 0);
						float colorMul = 1.0f;
						if (fakeVec.x > -1.0f) {
							colorMul = fakeVec.x * -1;
						}*/

						state->video[VideoMemoryOffset(x, y)] = color/*ColorMul(color, colorMul)*/;
					}
	#endif
				}
			}
		}

		v += 3;
	}
}

void DrawTriangle2D (State *state, float *verts, int vectorSize, int count, float *colors, float rotation = 0.0f) {
	// Vec2 *v = (Vec2*)verts;
	Color *c = (Color*)colors;

	// note: Don't use too many vertices or the stack will overflow
	// todo: Do something better
	Vec3 *vs = (Vec3*)_alloca(sizeof(Vec3)*count);
	for (int i = 0; i < count; ++i) {
		vs[i] = {};
		for (int j = 0; j < vectorSize && j < 3; ++j) {
			vs[i].e[j] = verts[i*vectorSize + j];
		}
		vs[i].xy = RotateVector(vs[i].xy, rotation);
		OrthoProjection(state, &vs[i]);
	}

	RasterizeTriangle(state, (float*)vs, vectorSize, count, colors, rotation);
}

void DrawTriangle3D (State *state, float *verts, int vectorSize, int count, float *colors, float rotation = 0.0f) {
	Color *c = (Color*)colors;

	// note: Don't use too many vertices or the stack will overflow
	// todo: Do something better
	Vec3 *vs = (Vec3*)_alloca(sizeof(Vec3)*count);
	for (int i = 0; i < count; ++i) {
		vs[i] = {};
		for (int j = 0; j < vectorSize && j < 3; ++j) {
			vs[i].e[j] = verts[i*vectorSize + j];
		}
		vs[i].xy = RotateVector(vs[i].xy, rotation);
		PerspectiveProjection(state, &vs[i], 70);
	}

	RasterizeTriangle(state, (float*)vs, vectorSize, count, colors, rotation);
}

void DrawRectangle (State *state, float *verts, int vectorSize, int count, float *colors, float rotation = 0.0f) {
#if 0
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
#endif

	int quadVertCount = ((count/4)*4);
	int triVertCount = quadVertCount + (quadVertCount/2);

	float *v = (float*)_alloca(sizeof(float)*triVertCount*vectorSize);
	float *temp = v;
	for (int i = 0; i < quadVertCount/4; ++i) {
		for (int j = 0; j < vectorSize; ++j) {
			temp[0*vectorSize + j] = verts[(i*4*vectorSize) + 0*vectorSize + j];
		}
		for (int j = 0; j < vectorSize; ++j) {
			temp[1*vectorSize + j] = verts[(i*4*vectorSize) + 1*vectorSize + j];
		}
		for (int j = 0; j < vectorSize; ++j) {
			temp[2*vectorSize + j] = verts[(i*4*vectorSize) + 2*vectorSize + j];
		}
		for (int j = 0; j < vectorSize; ++j) {
			temp[3*vectorSize + j] = verts[(i*4*vectorSize) + 0*vectorSize + j];
		}
		for (int j = 0; j < vectorSize; ++j) {
			temp[4*vectorSize + j] = verts[(i*4*vectorSize) + 2*vectorSize + j];
		}
		for (int j = 0; j < vectorSize; ++j) {
			temp[5*vectorSize + j] = verts[(i*4*vectorSize) + 3*vectorSize + j];
		}

		temp += 6*vectorSize;
	}

	DrawTriangle3D(state, v, vectorSize, triVertCount, NULL, rotation);

	// Vec2 *v = (Vec2*)verts;
	// Color *c = (Color*)colors;

	// Vec2 verts0[] = {
	// 	v[0],
	// 	v[1],
	// 	v[2],
	// };
	// Color colors0[] = {
	// 	c[0],
	// 	c[1],
	// 	c[2],
	// };
	// Vec2 verts1[] = {
	// 	v[0],
	// 	v[2],
	// 	v[3],
	// };
	// Color colors1[] = {
	// 	c[0],
	// 	c[2],
	// 	c[3],
	// };
	// DrawTriangle2D(state, (float*)verts0, (float*)colors0, rotation);
	// DrawTriangle2D(state, (float*)verts1, (float*)colors1, rotation);
}

void DrawPoint (State *state, float x, float y, Color color) {
	Vec3 vec = {x, y};
	OrthoProjection(state, &vec);
	Vec2 screenPos = GetPixelCoords(state, vec.x, vec.y);

	Color blank = {};
	Color c0 = ColorLerp(blank, color, (1.0f-FloatFract(screenPos.x)) * (1.0f-FloatFract(screenPos.y)));
	Color c1 = ColorLerp(blank, color, (FloatFract(screenPos.x)) * (1.0f-FloatFract(screenPos.y)));
	Color c2 = ColorLerp(blank, color, (FloatFract(screenPos.x)) * (FloatFract(screenPos.y)));
	Color c3 = ColorLerp(blank, color, (1.0f-FloatFract(screenPos.x)) * (FloatFract(screenPos.y)));

	int px = screenPos.x;
	int py = screenPos.y;
	state->video[VideoMemoryOffset(px, py)] = c0;
	state->video[VideoMemoryOffset(px+1, py)] = c1;
	state->video[VideoMemoryOffset(px+1, py+1)] = c2;
	state->video[VideoMemoryOffset(px, py+1)] = c3;
	int i = 0;
}

void ClearBackBuffer (State *state, Color color) {
	for (int i = 0; i < state->backBufferSize.x*state->backBufferSize.y; ++i) {
		state->video[i] = color;
	}
}

void Update (State *state) {
	double time = GetSeconds();
	state->dt = time - state->lastTime;
	state->lastTime = time;
	double dt = state->dt;

	// Color red = {255, 0, 0, 255};
	/*for (int i = 0; i < 640*360; ++i) {
		// state->video[i] = {RandomFloat(), RandomFloat(), RandomFloat(), 255};
		state->video[i] = {0, 1, 0, 1};
	}*/

	// DrawRectangle(state, -0.5f, 0.5f, 0.5f, -0.5f, {0, 1, 0, 1});

	ClearBackBuffer(state, {0, 0, 0, 0});

	static float rotation = 0.0f;
	rotation += 0.2f * dt;

	// if (state->input.rightDown) {
	// 	rotation += 0.0002f /** dt*/;
	// }
	// if (state->input.leftDown) {
	// 	rotation -= 0.0002f /** dt*/;
	// }

	float triVerts[] = {
		0.0f, 0.5f,
		1.0f, 0.0f,
		0.25, -0.5f,

		-2.0f, 0.5f,
		-1.0f, 0.0f,
		-2.25, -0.5f,
	};
	float colors[] = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,

		1, 0, 1, 0,
		0, 1, 1, 0,
		1, 1, 0, 0,
	};
	// DrawTriangle2D(state, triVerts, 2, 6, colors, rotation);

	float quadVerts[] = {
		-0.5f, 0.5f, -10.0f,
		0.5f, 0.5f, -10.0f,
		0.5f, -0.5f, -10.0f,
		-0.5f, -0.5f, -10.0f,

		/*-0.5f, 0.0f, 0.5f,
		0.5f, 0.0f, 0.5f,
		0.5f, 0.0f, -0.5f,
		-0.5f, 0.0f, -0.5f,

		-0.5f, -0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,*/
	};
	float quadColors[] = {
		1, 1, 0, 0,
		0, 1, 1, 0,
		1, 0, 1, 0,
		1, 1, 1, 0,

		1, 1, 0, 0,
		0, 1, 1, 0,
		1, 0, 1, 0,
		1, 1, 1, 0,
	};
	DrawRectangle(state, quadVerts, 3, 4, quadColors, rotation);

	/*static Vec2 pos = {};
	pos.x = 0.5f*cosf(rotation) + 0.5f*sinf(rotation);
	pos.y = 0.5f*cosf(rotation) - 0.5f*sinf(rotation);
	DrawPoint(state, pos.x, pos.y, {1, 0, 0, 0});*/

#if 0
	for (int i = 0; i < state->backBufferSize.x*state->backBufferSize.y; ++i) {
		float c = state->debug.video[i].msaa / 4.0f;
		state->video[i] = {c, c, c, 0};
	}
#endif

	state->debug.enabled = true;
}