#ifndef  _MSMATH_H_
#define _MSMATH_H_
#include "msMath.h"
#endif

#ifndef  _PD_API_H_
#define _PD_API_H_
#include "pd_api.h"
#endif

#define MAX_WIDTH 400
#define MAX_HEIGHT 240

#define MAX_W_WIDTH MAX_WIDTH / 2 
#define MAX_W_HEIGHT MAX_HEIGHT / 2

typedef struct Camera
{
	Mat3x3 cameraPosition;
	Mat3x3 worldToScreenCorrection;

	Mat3x3 cameraMatrixCache;
	int isCacheValid;
} Camera;

void msgfx_moveCamera(Camera* camera, Vec2 movement)
{
	Mat3x3 move;
	mat3x3TranslationMatrix(movement.x, movement.y, &move);
	mat3x3Mul(move, camera->cameraPosition, &camera->cameraPosition);
	camera->isCacheValid = 0;
}

void msgfx_initCamera(Camera* camera)
{
	camera->cameraPosition = _Mat3x3_Identity;
	camera->worldToScreenCorrection;
	camera->isCacheValid = 0;

	// translates from world space (center at 0,0 and x flipped) to screen space (use only to draw things)
	mat3x3TranslationMatrix(MAX_W_WIDTH, MAX_W_HEIGHT, &camera->worldToScreenCorrection);
	mat3x3Mul(camera->worldToScreenCorrection, _Mat3x3_MirrorX, &camera->worldToScreenCorrection);
}

void msgfx_getCameraOutputV(Camera* camera, Vec2 worldCoord, Vec2* screenCoord)
{
	if (!camera->isCacheValid)
	{
		mat3x3Mul(camera->cameraPosition, camera->worldToScreenCorrection, &camera->cameraMatrixCache);
		camera->isCacheValid = 1;
	}

	mat3x3MulVec2(camera->cameraMatrixCache, worldCoord, screenCoord);
}

void msgfx_getCameraOutput(Camera* camera, float x, float y, Vec2* screenCoord)
{
	Vec2 vec = { x, y };
	msgfx_getCameraOutputV(camera, vec, screenCoord);
}

typedef struct msgfx_drawLineArgs
{
	int x1;
	int y1;
	int x2;
	int y2;
	int width;
	LCDColor color;
} msgfx_drawLineArgs;

void msgfx_drawLine(PlaydateAPI* pd, Camera* camera, msgfx_drawLineArgs args)
{
	Vec2 p1;
	msgfx_getCameraOutput(camera, args.x1, args.y1, &p1);
	Vec2 p2;
	msgfx_getCameraOutput(camera, args.x2, args.y2, &p2);

	pd->graphics->drawLine(p1.x, p1.y, p2.x, p2.y, args.width, args.color);
}

typedef struct msgfx_fillEllipseArgs
{
	int x;
	int y;
	int width; 
	int height;
	float startAngle;
	float endAngle;
	LCDColor color;
} msgfx_fillEllipseArgs;

void msfx_fillEllipse(PlaydateAPI* pd, Camera* camera, msgfx_fillEllipseArgs args)
{
	Vec2 p;
	msgfx_getCameraOutput(camera, args.x, args.y, &p);
	pd->graphics->fillEllipse(p.x - args.width/2, p.y - args.height/2, args.width, args.height, args.startAngle, args.endAngle, args.color);
}

typedef struct msgfx_fillRectArgs
{
	int x;
	int y;
	int width;
	int height;
	LCDColor color;
} msgfx_fillRectArgs;

void msgfx_fillRect(PlaydateAPI* pd, Camera* camera, msgfx_fillRectArgs args)
{
	Vec2 p;
	msgfx_getCameraOutput(camera, args.x, args.y, &p);
	pd->graphics->fillRect(p.x - args.width / 2, p.y - args.height / 2, args.width, args.height, args.color);
}