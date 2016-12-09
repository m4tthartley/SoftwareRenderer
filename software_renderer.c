
#undef near
#undef far

#include <math.h>
#include <malloc.h>
#include <stdlib.h>

typedef float real;
typedef double real64;

#define Assert(expression) if (!(expression)) { *((int*)0) = 0; }

typedef struct {
	float r;
	float g;
	float b;
	float a;
} Color;

typedef struct {
	float x;
	float y;
} Vec2;

typedef union {
	struct {
		float x;
		float y;
		float z;
	};
	struct {
		Vec2 xy;
		// float z;
	};
	struct {
		float e[3];
	};
} Vec3;

typedef struct {
	float x;
	float y;
	float z;
} EulerAngle;

typedef struct {
	int x;
	int y;
} IVec2;

typedef struct {
	int msaa;
} DebugPixel;
typedef struct {
	DebugPixel *video;
	bool enabled;
} DebugData;

typedef struct {
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
} State;

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

Color ColorAdd (Color c0, Color c1) {
	Color result;
	result.r = c0.r + c1.r;
	result.g = c0.g + c1.g;
	result.b = c0.b + c1.b;
	result.a = c0.a + c1.a;
	return result;
}
void ColorAddP (Color *c0, Color c1) {
	c0->r += c1.r;
	c0->g += c1.g;
	c0->b += c1.b;
	c0->a += c1.a;
}
/*Color operator+ (Color c0, Color c1) {
	Color result;
	result.r = c0.r + c1.r;
	result.g = c0.g + c1.g;
	result.b = c0.b + c1.b;
	result.a = c0.a + c1.a;
	return result;
}*/

/*void operator+= (Color &c0, Color c1) {
	c0.r += c1.r;
	c0.g += c1.g;
	c0.b += c1.b;
	c0.a += c1.a;
}*/

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
	state->video = (Color*)malloc(videoMemorySize+debugVideoSize); //VirtualAlloc(NULL, videoMemorySize+debugVideoSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	state->debug.video = (DebugPixel*)(state->video + (state->backBufferSize.x*state->backBufferSize.y));

	state->lastTime = GetSeconds();
}

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

void PerspectiveProjection (State *state, Vec3 *vec, float fov) {
	float f = 1.0f / tanf(Rads(fov) / 2.0f);
	float near = -10.0f;
	float far = 10.0f;
	float aspect = (float)state->backBufferSize.x/(float)state->backBufferSize.y;
	float m[] = {
		f / aspect, 0, 0, 0,
		0, f, 0, 0,
		0, 0, (far + near) / (near - far), -1,
		0, 0, (2.0f * far * near) / (near - far), 0,
	};

	Vec3 backup = *vec;
	float w = 0.0f;

	vec->x = m[0]*backup.x + m[1]*backup.y + m[2]*backup.z;
	vec->y = m[4]*backup.x + m[5]*backup.y + m[6]*backup.z;
	vec->z = m[8]*backup.x + m[9]*backup.y + m[10]*backup.z;

	vec->x *= 1.0f / backup.z;
	vec->y *= 1.0f / backup.z;
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

void RasterizeTriangle (State *state, float *verts, int vectorSize, int count, float *colors) {
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
							color = ColorAdd(ColorAdd(ColorMul(c[0], c0), ColorMul(c[1], c1)), ColorMul(c[2], c2));
						} else {
							// color = {1, 1, 1, 1};
							color.r = 1;
							color.g = 1;
							color.b = 1;
							color.a = 1;
						}

						ColorAddP(&state->video[VideoMemoryOffset(x, y)], ColorMul(color, /*1.0f*/(float)fillSamples / 4.0f));

						if (state->video[VideoMemoryOffset(x, y)].r > 1.0f) state->video[VideoMemoryOffset(x, y)].r = 1.0f;
						if (state->video[VideoMemoryOffset(x, y)].g > 1.0f) state->video[VideoMemoryOffset(x, y)].g = 1.0f;
						if (state->video[VideoMemoryOffset(x, y)].b > 1.0f) state->video[VideoMemoryOffset(x, y)].b = 1.0f;
						if (state->video[VideoMemoryOffset(x, y)].a > 1.0f) state->video[VideoMemoryOffset(x, y)].a = 1.0f;

						state->debug.video[VideoMemoryOffset(x, y)].msaa = fillSamples;
					}
				}
			}
		}

		v += 3;
		c += 3;
	}
}

void DrawTriangle2D (State *state, float *verts, int vectorSize, int count, float *colors, float rotation) {
	Color *c = (Color*)colors;

	// todo: Don't malloc please
	Vec3 *vs = (Vec3*)malloc(sizeof(Vec3)*count);
	for (int i = 0; i < count; ++i) {
		// vs[i] = {};
		ZeroStruct(vs[i]);
		for (int j = 0; j < vectorSize && j < 3; ++j) {
			vs[i].e[j] = verts[i*vectorSize + j];
		}
		vs[i].xy = RotateVector(vs[i].xy, rotation);
		OrthoProjection(state, &vs[i]);
	}

	RasterizeTriangle(state, (float*)vs, vectorSize, count, colors);

	free(vs);
}

void EulerMatrixRotation (Vec3 *vec, float xRads, float yRads, float zRads) {
	// todo: This is horrible. fix it.

	float x[] = {
		1, 0, 0, 0,
		0, cosf(xRads), -sinf(xRads), 0,
		0, sinf(xRads), cosf(xRads), 0,
		0, 0, 0, 1,
	};

	float y[] = {
		 cosf(yRads), 0, sinf(yRads), 0,
		 0, 1, 0, 0,
		-sinf(yRads), 0, cosf(yRads), 0,
		 0, 0, 0, 1,
	};

	float z[] = {
		cosf(zRads), -sinf(zRads), 0, 0,
		sinf(zRads), cosf(zRads), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1,
	};

	float xy[] = {
		x[0]*y[0] + x[1]*y[4] + x[2]*y[8] + x[3]*y[12],
		x[0]*y[1] + x[1]*y[5] + x[2]*y[9] + x[3]*y[13],
		x[0]*y[2] + x[1]*y[6] + x[2]*y[10] + x[3]*y[14],
		x[0]*y[3] + x[1]*y[7] + x[2]*y[11] + x[3]*y[15],

		x[4]*y[0] + x[5]*y[4] + x[6]*y[8] + x[7]*y[12],
		x[4]*y[1] + x[5]*y[5] + x[6]*y[9] + x[7]*y[13],
		x[4]*y[2] + x[5]*y[6] + x[6]*y[10] + x[7]*y[14],
		x[4]*y[3] + x[5]*y[7] + x[6]*y[11] + x[7]*y[15],

		x[8]*y[0] + x[9]*y[4] + x[10]*y[8] + x[11]*y[12],
		x[8]*y[1] + x[9]*y[5] + x[10]*y[9] + x[11]*y[13],
		x[8]*y[2] + x[9]*y[6] + x[10]*y[10] + x[11]*y[14],
		x[8]*y[3] + x[9]*y[7] + x[10]*y[11] + x[11]*y[15],

		x[12]*y[0] + x[13]*y[4] + x[14]*y[8] + x[15]*y[12],
		x[12]*y[1] + x[13]*y[5] + x[14]*y[9] + x[15]*y[13],
		x[12]*y[2] + x[13]*y[6] + x[14]*y[10] + x[15]*y[14],
		x[12]*y[3] + x[13]*y[7] + x[14]*y[11] + x[15]*y[15],
	};

	float xyz[] = {
		xy[0]*z[0] + xy[1]*z[4] + xy[2]*z[8] + xy[3]*z[12],
		xy[0]*z[1] + xy[1]*z[5] + xy[2]*z[9] + xy[3]*z[13],
		xy[0]*z[2] + xy[1]*z[6] + xy[2]*z[10] + xy[3]*z[14],
		xy[0]*z[3] + xy[1]*z[7] + xy[2]*z[11] + xy[3]*z[15],

		xy[4]*z[0] + xy[5]*z[4] + xy[6]*z[8] + xy[7]*z[12],
		xy[4]*z[1] + xy[5]*z[5] + xy[6]*z[9] + xy[7]*z[13],
		xy[4]*z[2] + xy[5]*z[6] + xy[6]*z[10] + xy[7]*z[14],
		xy[4]*z[3] + xy[5]*z[7] + xy[6]*z[11] + xy[7]*z[15],

		xy[8]*z[0] + xy[9]*z[4] + xy[10]*z[8] + xy[11]*z[12],
		xy[8]*z[1] + xy[9]*z[5] + xy[10]*z[9] + xy[11]*z[13],
		xy[8]*z[2] + xy[9]*z[6] + xy[10]*z[10] + xy[11]*z[14],
		xy[8]*z[3] + xy[9]*z[7] + xy[10]*z[11] + xy[11]*z[15],

		xy[12]*z[0] + xy[13]*z[4] + xy[14]*z[8] + xy[15]*z[12],
		xy[12]*z[1] + xy[13]*z[5] + xy[14]*z[9] + xy[15]*z[13],
		xy[12]*z[2] + xy[13]*z[6] + xy[14]*z[10] + xy[15]*z[14],
		xy[12]*z[3] + xy[13]*z[7] + xy[14]*z[11] + xy[15]*z[15],
	};

	float w = 0;
	Vec3 v = *vec;
	vec->x = xyz[0]*v.x + xyz[1]*v.y + xyz[2]*v.z + xyz[3]*w;
	vec->y = xyz[4]*v.x + xyz[5]*v.y + xyz[6]*v.z + xyz[7]*w;
	vec->z = xyz[8]*v.x + xyz[9]*v.y + xyz[10]*v.z + xyz[11]*w;
}

void DrawTriangle3D (State *state, float *verts, int vectorSize, int count, float *colors, EulerAngle rotation, bool drawBackFaces) {
	Color *c = (Color*)colors;

	// todo: Don't malloc please
	Vec3 *vs = (Vec3*)malloc(sizeof(Vec3)*count);
	for (int i = 0; i < count; ++i) {
		// vs[i] = {};
		ZeroStruct(vs[i]);
		for (int j = 0; j < vectorSize && j < 3; ++j) {
			vs[i].e[j] = verts[i*vectorSize + j];
		}

		Vec3 v = vs[i];

		// float xm = (cosf(rotation.y) + sinf(rotation.y))
		// 		  *(cosf(rotation.z) + sinf(rotation.z));
		// float ym = (cosf(rotation.x) + sinf(rotation.x))
		// 		  *(cosf(rotation.z) - sinf(rotation.z));
		// float zm = (cosf(rotation.x) - sinf(rotation.x))
		// 		  *(cosf(rotation.y) - sinf(rotation.y));

		EulerMatrixRotation(&vs[i], rotation.x, rotation.y, rotation.z);

		vs[i].z += 3.0f;

		PerspectiveProjection(state, &vs[i], 70);
	}

	// char shit[64];
	// sprintf(shit, "v0x %f \n", vs[0].x);
	// OutputDebugString(shit);

	RasterizeTriangle(state, (float*)vs, vectorSize, count, colors);

	if (drawBackFaces) {
		for (int i = 0; i < count/3; ++i) {
			Vec3 swap = vs[i*3 + 0];
			vs[i*3 + 0] = vs[i*3 + 2];
			vs[i*3 + 2] = swap;

			Color colorSwap = c[i*3 + 0];
			c[i*3 + 0] = c[i*3 + 2];
			c[i*3 + 2] = colorSwap;
		}

		RasterizeTriangle(state, (float*)vs, vectorSize, count, colors);
	}

	free(vs);
}

void DrawRectangle (State *state, float *verts, int vectorSize, int count, float *colors, EulerAngle rotation, float rx, float ry, float rz) {
	int quadVertCount = ((count/4)*4);
	int triVertCount = quadVertCount + (quadVertCount/2);

	float *v = (float*)malloc(sizeof(float)*triVertCount*vectorSize);
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

	float *c = (float*)malloc(sizeof(float)*triVertCount*4);
	temp = c;
	for (int i = 0; i < quadVertCount/4; ++i) {
		for (int j = 0; j < 4; ++j) {
			temp[0*4 + j] = colors[(i*4*4) + 0*4 + j];
		}
		for (int j = 0; j < 4; ++j) {
			temp[1*4 + j] = colors[(i*4*4) + 1*4 + j];
		}
		for (int j = 0; j < 4; ++j) {
			temp[2*4 + j] = colors[(i*4*4) + 2*4 + j];
		}
		for (int j = 0; j < 4; ++j) {
			temp[3*4 + j] = colors[(i*4*4) + 0*4 + j];
		}
		for (int j = 0; j < 4; ++j) {
			temp[4*4 + j] = colors[(i*4*4) + 2*4 + j];
		}
		for (int j = 0; j < 4; ++j) {
			temp[5*4 + j] = colors[(i*4*4) + 3*4 + j];
		}

		temp += 6*4;
	}

	DrawTriangle3D(state, v, vectorSize, triVertCount, c, rotation, false);

	free(v);
	free(c);
}

void DrawPoint (State *state, float x, float y, Color color) {
	Vec3 vec = {x, y};
	OrthoProjection(state, &vec);
	Vec2 screenPos = GetPixelCoords(state, vec.x, vec.y);

	Color blank = {0};
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

void Update (OSState *os, State *state) {
	double time = GetSeconds();
	state->dt = time - state->lastTime;
	state->lastTime = time;
	double dt = state->dt;
	// printf("dt %f \n", dt*1000);

	Color clearColor = {0};
	ClearBackBuffer(state, clearColor);

	static EulerAngle rotation = {0};
	/*rotation.x += 0.4f * dt;
	rotation.y += 0.8f * dt;*/
	rotation.y += KeyPressed(os, KEYBOARD_RIGHT) * 0.03f;
	rotation.y -= KeyPressed(os, KEYBOARD_LEFT) * 0.03f;
	rotation.x += KeyDown(os, KEYBOARD_DOWN) * 0.03f;
	rotation.x -= KeyDown(os, KEYBOARD_UP) * 0.03f;

	rotation.y += KeyDown(os, KEYBOARD_1) * 0.03f;
	rotation.y -= KeyDown(os, KEYBOARD_2) * 0.03f;

	// rotation.z += 0.4f * dt;

	// if (state->input.rightDown) {
	// 	rotation += 0.0002f /** dt*/;
	// }
	// if (state->input.leftDown) {
	// 	rotation -= 0.0002f /** dt*/;
	// }

#if 0
	float triVerts[] = {
		-0.5f, 0.5f,
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
	DrawTriangle3D(state, triVerts, 2, 3, colors, rotation);
#endif

#if 0
	float triVerts2[] = {
		-0.5f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,

		-0.5f, 0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
	};
	float colors2[] = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,

		1, 0, 0, 0,
		0, 0, 1, 0,
		1, 1, 0, 0,
	};
	DrawTriangle3D(state, triVerts2, 3, 6, colors2, rotation);
#endif

	float quadVerts[] = {
		// front
		-0.5f, 0.5f, -0.5f,
		0.5f, 0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		// back
		0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,
		-0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		// left
		-0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, 0.5f,
		// right
		0.5f, 0.5f, -0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, -0.5f,
		// top
		-0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f,
		// bottom
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, 0.5f,
		-0.5f, -0.5f, 0.5f,
	};
	float quadColors[] = {
		1, 0, 0, 1,
		1, 0, 0, 1,
		1, 0, 0, 1,
		1, 0, 0, 1,

		0, 1, 0, 1,
		0, 1, 0, 1,
		0, 1, 0, 1,
		0, 1, 0, 1,

		0, 0, 1, 1,
		0, 0, 1, 1,
		0, 0, 1, 1,
		0, 0, 1, 1,

		1, 1, 0, 1,
		1, 1, 0, 1,
		1, 1, 0, 1,
		1, 1, 0, 1,

		0, 1, 1, 1,
		0, 1, 1, 1,
		0, 1, 1, 1,
		0, 1, 1, 1,

		1, 0, 1, 1,
		1, 0, 1, 1,
		1, 0, 1, 1,
		1, 0, 1, 1,
	};
	DrawRectangle(state, quadVerts, 3, 24, quadColors, rotation, rotation.x, rotation.y, rotation.z);
	
#if 0
	for (int i = 0; i < state->backBufferSize.x*state->backBufferSize.y; ++i) {
		float c = state->debug.video[i].msaa / 4.0f;
		state->video[i] = {c, c, c, 0};
	}
#endif

	state->debug.enabled = true;
}
