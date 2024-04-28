#ifndef  _PD_API_H_
#define _PD_API_H_
#include "pd_api.h"
#endif

#ifndef  _MSMATH_H_
#define _MSMATH_H_
#include "msMath.h"
#endif

#ifndef  _GAMEPRIMITIVES_H_
#define _GAMEPRIMITIVES_H_
#include "gamePrimitives.h"
#endif

#define NUMBER_OF_WALL_COLLISIONS 3

void eulerIntegration(Point* point, float dt)
{
	Vec2 tempVel;
	vec2MulScalar(point->velocity, dt, &tempVel);
	vec2Add(point->position, tempVel, &point->position);
}

void updateWallCollisions(Collision* wallCollisions, Point* point, float dt)
{
	Collision collision;
	if (findWallCollision(wallCollisions, *point, &collision))
	{
		{
			// resolve the constraint
			Vec2 temp;
			vec2MulScalar(collision.normal, collision.depth, &temp);
			vec2Add(point->position, temp, &point->position);
		}

		// compute the normal & tangential velocity
		Vec2 normalVelocity;
		vec2MulScalar(collision.normal, vec2Dot(collision.normal, point->velocity), &normalVelocity);
		Vec2 tangentialVelocity;
		vec2Sub(point->velocity, normalVelocity, &tangentialVelocity);

		float elasticity = 1.0f;

		// apply bouncing
		vec2MulScalar(normalVelocity, -elasticity, &normalVelocity);

		float friction = 0.0f;
		float frictionDelta = expf(-friction * dt);
		// apply friction
		//vec2MulScalar(tangentialVelocity, frictionDelta, &tangentialVelocity);

		// add up the new velocity
		vec2Add(normalVelocity, tangentialVelocity, &point->velocity);
	}
}

int findWallCollision(Collision* wallCollisions, Point point, Collision* output)
{
	for (int i = 0; i < NUMBER_OF_WALL_COLLISIONS; ++i)
	{
		Collision collision = wallCollisions[i];

		Vec2 collisionPosition;
		// move wall collision by the point radius size
		vec2MulScalar(collision.normal, point.radius / 2, &collisionPosition);
		vec2Add(collisionPosition, collision.position, &collisionPosition);

		Vec2 directionOfCollision;
		// updated directionOfCollision
		vec2Sub(point.position, collisionPosition, &directionOfCollision);
		vec2Normalize(&directionOfCollision);

		if (vec2Dot(directionOfCollision, collision.normal) < 0)
		{
			*output = collision;

			Vec2 temp = VEC2(point.position.x * fabsf(collision.normal.x), point.position.y * fabsf(collision.normal.y));
			output->depth = vec2Distance(temp, collisionPosition);

			return 1;
		}
	}

	return 0;
}

int doesLineAndPointCollide(Vec2 linePoint1, Vec2 linePoint2, Vec2 point)
{
	// get distance from the point to the two ends of the line
	float d1 = vec2Distance(point, linePoint1);
	float d2 = vec2Distance(point, linePoint2);

	// get the length of the line
	float lineLen = vec2Distance(linePoint1, linePoint2);

	// since floats are so minutely accurate, add
	// a little buffer zone that will give collision
	float buffer = 0.0001f;    // higher # = less accurate

	// if the two distances are equal to the line's
	// length, the point is on the line!
	// note we use the buffer here to give a range,
	// rather than one #
	if (d1 + d2 >= lineLen - buffer && d1 + d2 <= lineLen + buffer)
	{
		return 1;
	}
	return 0;
}

int doesPointAndCircleCollide(Vec2 point, Vec2 circleCenter, float radius)
{
	// get distance between the point and circle's center
	float distance = vec2Distance(point, circleCenter);

	// if the distance is less than the circle's
	// radius the point is inside!
	if (distance <= radius)
	{
		return 1;
	}

	return 0;
}

int doesLineAndCircleCollide(Vec2 linePoint1, Vec2 linePoint2, Vec2 circleCenter, float radius)
{
	// is either end INSIDE the circle?
	// if so, return true immediately
	int inside1 = doesPointAndCircleCollide(linePoint1, circleCenter, radius);
	int inside2 = doesPointAndCircleCollide(linePoint2, circleCenter, radius);

	if (inside1 || inside2)
	{
		return 1;
	}

	// get length of the line
	float len = vec2Distance(linePoint1, linePoint2);

	// get dot product of the line and circle
	float dot = (((circleCenter.x - linePoint1.x) * (linePoint2.x - linePoint1.x)) 
				+ ((circleCenter.y - linePoint1.y) * (linePoint2.y - linePoint1.y)))
				/ powf(len, 2);

	// find the closest point on the line
	Vec2 closestPoint = VEC2(
		linePoint1.x + (dot * (linePoint2.x - linePoint1.x)), 
		linePoint1.y + (dot * (linePoint2.y - linePoint1.y)));

	// is this point actually on the line segment?
	// if so keep going, but if not, return false
	int onSegment = doesLineAndPointCollide(linePoint1, linePoint2, closestPoint);
	if (!onSegment)
	{
		return 0;
	}

	// get distance to closest point
	float distance = vec2Distance(closestPoint, circleCenter);

	if (distance <= radius)
	{
		return 1;
	}

	return 0;
}

int findPointRectCollision(Point point, Rect rect, Collision* output)
{
	Collision collision;
	collision.normal = VEC2(0, 0);

	float nx = 0.0f;
	float ny = 0.0f;

	Vec2 div2;
	vec2DivScalar(rect.size, 2.0f, &div2);

	Vec2 a = VEC2(rect.center.x - div2.x, rect.center.y + div2.y);
	Vec2 b = VEC2(rect.center.x + div2.x, rect.center.y + div2.y);
	Vec2 c = VEC2(rect.center.x - div2.x, rect.center.y - div2.y);
	Vec2 d = VEC2(rect.center.x + div2.x, rect.center.y - div2.y);

	/*
	 A--------------B
	 -				-
	 -				-
	 -		.		-
	 -				-
	 -				-
	 C--------------D
	*/

	if (doesLineAndCircleCollide(a, b, point.position, point.radius))
	{
		collision.position = VEC2(point.position.x, a.y + point.radius);
		collision.normal = VEC2(0, 1);
		*output = collision;
		return 1;
	}

	if (doesLineAndCircleCollide(b, d, point.position, point.radius))
	{
		collision.position = VEC2(b.x + point.radius, point.position.y);
		collision.normal = VEC2(1, 0);
		*output = collision;
		return 1;
	}

	if (doesLineAndCircleCollide(c, a, point.position, point.radius))
	{
		collision.normal = VEC2(-1, 0);
		collision.position = VEC2(c.x - point.radius, point.position.y);
		*output = collision;
		return 1;
	}

	if (doesLineAndCircleCollide(c, d, point.position, point.radius))
	{
		collision.normal = VEC2(0, -1);
		collision.position = VEC2(point.position.x, c.y - point.radius);
		*output = collision;
		return 1;
	}

	return 0;
}