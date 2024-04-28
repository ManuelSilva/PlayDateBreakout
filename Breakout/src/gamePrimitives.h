#ifndef  _PD_API_H_
#define _PD_API_H_
#include "pd_api.h"
#endif

#ifndef  _MSMATH_H_
#define _MSMATH_H_
#include "msMath.h"
#endif

#ifndef  _GFX_WRAPPER_H_
#define _GFX_WRAPPER_H_
#include "gfxWrapper.h"
#endif

typedef struct Collision
{
	Vec2 normal;
	Vec2 position;// used for wall only

	float depth;

} Collision;

typedef struct Point
{
	Vec2 position;
	Vec2 velocity;
	int radius;
} Point;

typedef struct Rect
{
	Vec2 center;
	Vec2 size;
} Rect;

const Point _Point_Default = { 0.0f, 0.0f, 0.0f, 0.0f, 8 };

void renderArrow(PlaydateAPI* pd, Camera* camera, Vec2 location, Vec2 vectorToDraw)
{
	msgfx_drawLineArgs arg1 =
	{
		location.x, location.y,
		vectorToDraw.x, vectorToDraw.y,
		1,
		kColorBlack
	};
	msgfx_drawLine(pd, camera, arg1);

	msgfx_fillEllipseArgs args2 =
	{
		location.x, location.y,
		3,
		3,
		0,
		360,
		kColorBlack
	};
	msfx_fillEllipse(pd, camera, args2);

	Vec2 normal;
	vec2Sub(location, vectorToDraw, &normal);
	vec2Normalize(&normal);

	Mat3x3 rotateArrow1;
	mat3x3RotationMatrix(PI / 5, &rotateArrow1);

	Mat3x3 rotateArrow2;
	mat3x3RotationMatrix(-PI / 5, &rotateArrow2);

	Vec2 whisker1;
	mat3x3MulVec2(rotateArrow1, normal, &whisker1);
	vec2MulScalar(whisker1, 10, &whisker1);
	vec2Add(vectorToDraw, whisker1, &whisker1);

	Vec2 whisker2;
	mat3x3MulVec2(rotateArrow2, normal, &whisker2);
	vec2MulScalar(whisker2, 10, &whisker2);
	vec2Add(vectorToDraw, whisker2, &whisker2);

	msgfx_drawLineArgs args3 =
	{
		vectorToDraw.x, vectorToDraw.y,
		whisker1.x, whisker1.y,
		2,
		kColorBlack
	};
	msgfx_drawLine(pd, camera, args3);

	msgfx_drawLineArgs args4 =
	{
		vectorToDraw.x, vectorToDraw.y,
		whisker2.x, whisker2.y,
		2,
		kColorBlack
	};
	msgfx_drawLine(pd, camera, args4);
}

void renderPoint(PlaydateAPI* pd, Camera* camera, Point* point)
{
	// used to debug velocity
#if _DEBUG
	if (vec2LenSquared(point->velocity) > 3.0f)
	{
		Vec2 amplifiedSpeedForDebug;
		vec2MulScalar(point->velocity, 1, &amplifiedSpeedForDebug);
		vec2Add(point->position, point->velocity, &amplifiedSpeedForDebug);
		renderArrow(pd, camera, point->position, amplifiedSpeedForDebug);
	}
#endif

	msgfx_fillEllipseArgs args =
	{
		point->position.x, 
		point->position.y,
		point->radius*2,
		point->radius*2,
		0,
		360,
		kColorBlack
	};

	msfx_fillEllipse(pd, camera, args);
}

void renderRect(PlaydateAPI* pd, Camera* camera, Rect* rect)
{
	msgfx_fillRectArgs args = {
		rect->center.x,
		rect->center.y,
		rect->size.x,
		rect->size.y,
		kColorBlack
	};

	msgfx_fillRect(pd, camera, args);
}

void renderSoftBody(PlaydateAPI* pd, Camera* camera, Point* softBodyPoints, int size)
{
	for (int i = 0; i < size; ++i)\
	{
		renderPoint(pd, camera, &softBodyPoints[i]);
	}
}

void initSoftBodyPoints(Point* softBodyPoints, int size, float radius, float pointSize) 
{
	Mat3x3 rm;
	mat3x3RotationMatrix(PI / 4.0f, &rm);
	for (int i = 0; i < size; ++i)
	{
		Vec2 temp = { 
			radius * cosf(2 * PI * i / (float)(size)),
			radius * sinf(2 * PI * i / (float)(size))
		};
		softBodyPoints[i].velocity = VEC2( 0, 0 );
		softBodyPoints[i].radius = pointSize;

		mat3x3MulVec2(rm, temp, &softBodyPoints[i].position);
	}
}