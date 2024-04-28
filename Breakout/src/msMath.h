#include <math.h>

#define PI 3.141592f 
#define Degree2Radians PI / 180.0f
#define VEC2(x, y) (Vec2){ x, y }
#define SIZE_OF_ARRAY(x) sizeof(x) / sizeof(x[0])

typedef struct Vec2
{
	float x;
	float y;
} Vec2;

const Vec2 _Vec2_ZERO = { 0.0f, 0.0f };

int msfsign(float x)
{
	return (x > 0.0f) - (x < 0.0f);
}

void vec2Add(Vec2 v1, Vec2 v2, Vec2* output)
{
	output->x = v1.x + v2.x;
	output->y = v1.y + v2.y;
}

void vec2Sub(Vec2 v1, Vec2 v2, Vec2* output)
{
	output->x = v1.x - v2.x;
	output->y = v1.y - v2.y;
}

void vec2MulScalar(Vec2 v1, float scalar, Vec2* output)
{
	output->x = v1.x * scalar;
	output->y = v1.y * scalar;
}

void vec2DivScalar(Vec2 v1, float scalar, Vec2* output)
{
	output->x = v1.x / scalar;
	output->y = v1.y / scalar;
}

float vec2Dot(Vec2 v1, Vec2 v2)
{
	return v1.x * v2.x + v1.y * v2.y;
}

float vec2Len(Vec2 v)
{
	return sqrtf(vec2Dot(v, v));
}

float vec2LenSquared(Vec2 v)
{
	return vec2Dot(v, v);
}

float vec2Distance(Vec2 v1, Vec2 v2)
{
	Vec2 temp;
	vec2Sub(v1, v2, &temp);
	return vec2Len(temp);
}

void vec2GetNormalizedVector(Vec2 v, Vec2* output)
{
	vec2DivScalar(v, vec2Len(v), output);
}

void vec2Normalize(Vec2* output)
{
	vec2DivScalar(*output, vec2Len(*output), output);
}

typedef struct Mat3x3
{
	float a; float b; float c; 
	float d; float e; float f;
	float g; float h; float i;
} Mat3x3;

const Mat3x3 _Mat3x3_Identity =
{
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f
};

const Mat3x3 _Mat3x3_MirrorX = {
		1.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, 0.0f, 1.0f
};

void mat3x3Mul(Mat3x3 m1, Mat3x3 m2, Mat3x3* output)
{
	output->a = m1.a * m2.a + m1.b * m2.d + m1.c * m2.g;
	output->b = m1.a * m2.b + m1.b * m2.e + m1.c * m2.h;
	output->c = m1.a * m2.c + m1.b * m2.f + m1.c * m2.i;

	output->d = m1.d * m2.a + m1.e * m2.d + m1.f * m2.g;
	output->e = m1.d * m2.b + m1.e * m2.e + m1.f * m2.h;
	output->f = m1.d * m2.c + m1.e * m2.f + m1.f * m2.i;

	output->g = m1.g * m2.a + m1.h * m2.d + m1.i * m2.g;
	output->h = m1.g * m2.b + m1.h * m2.e + m1.i * m2.h;
	output->i = m1.g * m2.c + m1.h * m2.f + m1.i * m2.i;
}

void mat3x3MulVec2(Mat3x3 m, Vec2 v, Vec2* output)
{
	output->x = m.a * v.x + m.b * v.y + m.c;
	output->y = m.d * v.x + m.e * v.y + m.f;
}

void mat3x3MulScalar(Mat3x3 m, float scalar, Mat3x3* output)
{
	output->a = m.a * scalar;
	output->b = m.b * scalar;
	output->c = m.c * scalar;
	output->d = m.d * scalar;
	output->e = m.e * scalar;
	output->f = m.f * scalar;
	output->g = m.g * scalar;
	output->h = m.h * scalar;
	output->i = m.i * scalar;
}

void mat3x3DivScalar(Mat3x3 m, float scalar, Mat3x3* output)
{
	output->a = m.a / scalar;
	output->b = m.b / scalar;
	output->c = m.c / scalar;
	output->d = m.d / scalar;
	output->e = m.e / scalar;
	output->f = m.f / scalar;
	output->g = m.g / scalar;
	output->h = m.h / scalar;
	output->i = m.i / scalar;
}

float mat3x3Determinant(Mat3x3 m)
{
	return m.a * (m.e * m.i - m.f * m.h)
		 + m.b * (m.d * m.i - m.f * m.g)
		 + m.c * (m.d * m.h - m.e * m.g);
}

void mat3x3TranslationMatrix(float x, float y, Mat3x3* output)
{
	*output = (Mat3x3){
		1.0f, 0.0f, x,
		0.0f, 1.0f, y,
		0.0f, 0.0f, 1.0f
	};
}

void mat3x3TranslationMatrixV(Vec2 v, Mat3x3* output)
{
	mat3x3TranslationMatrix(v.x,v.y, output);
}

void mat3x3ScaleMatrix(float scale, Mat3x3* output)
{
	*output = (Mat3x3){
		scale, 0.0f, 0.0f,
		0.0f, scale, 0.0f,
		0.0f, 0.0f, scale
	};
}

void mat3x3RotationMatrix(float theta, Mat3x3* output)
{
	*output = (Mat3x3){
		cosf(theta), sinf(theta), 0.0f,
		-sinf(theta), cosf(theta), 0.0f,
		0.0f, 0.0f, 1.0f
	};
}

void mat3x3InverseMatrix(Mat3x3 m, Mat3x3* output)
{
	float inverseDeterminant = 1.0f / mat3x3Determinant(m);

	 output->a = (m.e * m.i - m.h * m.f) * inverseDeterminant;
	 output->b = (m.c * m.h - m.b * m.i) * inverseDeterminant;
	 output->c = (m.b * m.f - m.c * m.e) * inverseDeterminant;
	 output->d = (m.f * m.g - m.d * m.i) * inverseDeterminant;
	 output->e = (m.a * m.i - m.c * m.g) * inverseDeterminant;
	 output->f = (m.d * m.c - m.a * m.f) * inverseDeterminant;
	 output->g = (m.d * m.h - m.g * m.e) * inverseDeterminant;
	 output->h = (m.g * m.b - m.a * m.h) * inverseDeterminant;
	 output->i = (m.a * m.e - m.d * m.b) * inverseDeterminant;
}