#include "main.h"

#define NUMBER_OF_BREAKABLES_X 19
#define NUMBER_OF_BREAKABLES_Y 8
#define NUMBER_OF_BREAKABLES_TOTAL NUMBER_OF_BREAKABLES_X * NUMBER_OF_BREAKABLES_Y
#define MAX_VELOCITY 90
#define TRAIL_SIZE 5
#define TRAIL_REFRESH 3


typedef struct GameScene
{
	Camera camera;

	Point ball;
	Rect paddle;

	int trailTicks;
	int currentIndexToUpdate;
	Point trail[TRAIL_SIZE];

	Rect breakables[NUMBER_OF_BREAKABLES_TOTAL];
	Collision WallCollisions[NUMBER_OF_WALL_COLLISIONS];
} GameScene;

GameScene _GameScene;

const char* fontpath = "/System/Fonts/Asheville-Sans-14-Bold.pft";
LCDFont* font = NULL;

static int update(void* userdata);

#ifdef _WINDLL
__declspec(dllexport)
#endif
void initializeScene()
{
	// Init scene
	_GameScene.ball = _Point_Default;
	_GameScene.ball.position.y = -85;
	_GameScene.ball.velocity.y = 25;
	_GameScene.ball.velocity.x = 20;
	_GameScene.ball.radius = 3;

	_GameScene.trailTicks = TRAIL_REFRESH;
	_GameScene.currentIndexToUpdate = 0;
	for(int i = 0; i < TRAIL_SIZE; ++i)
	{
		_GameScene.trail[i] = _GameScene.ball;
		_GameScene.trail[i].position.x = -1000;
		_GameScene.trail[i].velocity.x = 0;
		_GameScene.trail[i].velocity.y = 0;
		_GameScene.trail[i].radius = 1;
	}

	_GameScene.paddle.center = VEC2(0, -90);
	_GameScene.paddle.size = VEC2(35, 6);

	//_GameScene.WallCollisions[3] = (Collision){ {0.0f, 1.0f}, {0, -MAX_W_HEIGHT} };
	_GameScene.WallCollisions[0] = (Collision){ {0.0f, -1.0f}, {0, MAX_W_HEIGHT} };
	_GameScene.WallCollisions[1] = (Collision){ {1.0f, 0.0f}, {-MAX_W_WIDTH, 0} };
	_GameScene.WallCollisions[2] = (Collision){ {-1.0f, 0.0f}, {MAX_W_WIDTH, 0} };

	int index = 0;
	for (int i = 0; i < NUMBER_OF_BREAKABLES_X; ++i)
	{
		for (int j = 0; j < NUMBER_OF_BREAKABLES_Y; ++j)
		{
			int startX = -175;
			int startY = 50;

			if (j % 2 == 0)
			{
				startX += -10;
			}

			int iMultiplier = 20;
			int jMultiplier = 9;
			int sizeX = 10;
			int sizeY = 8;

			_GameScene.breakables[index] = (Rect){ startX + iMultiplier * i, startY + jMultiplier * j, sizeX, sizeY };
			index++;
		}
	}

	msgfx_initCamera(&_GameScene.camera);
}

#ifdef _WINDLL
__declspec(dllexport)
#endif
void initFunc(PlaydateAPI* pd)
{
	// Init Font
	const char* err;
	font = pd->graphics->loadFont(fontpath, &err);

	if (font == NULL)
	{
		pd->system->error("%s:%i Couldn't load font %s: %s", __FILE__, __LINE__, fontpath, err);
	}

	initializeScene();

	// Set up Update callback
	// Note: If you set an update callback in the kEventInit handler, the system assumes the game is pure C and doesn't run any Lua code in the game
	pd->system->setUpdateCallback(update, pd);
}

#ifdef _WINDLL
__declspec(dllexport)
#endif
int eventHandler(PlaydateAPI* pd, PDSystemEvent event, uint32_t arg)
{
	(void)arg; // arg is currently only used for event = kEventKeyPressed

	if ( event == kEventInit )
	{
		initFunc(pd);
	}
	
	return 0;
}

static int update(void* userdata)
{
	const float dt = 1.0f/10.0f;

	PlaydateAPI* pd = userdata;

	Point* ball = &_GameScene.ball;
	Rect* paddle = &_GameScene.paddle;


	float crankDelta = pd->system->getCrankChange();
	const float PADDLE_SPEED = .5f;

	if (fabsf(crankDelta) > 0.001f)
	{
		if (fabsf(paddle->center.x + crankDelta * PADDLE_SPEED) < MAX_W_WIDTH - 35/2)
		{
			paddle->center.x += crankDelta * PADDLE_SPEED;
		}
		else
		{
			paddle->center.x = msfsign(paddle->center.x) * (MAX_W_WIDTH - 35 / 2 - 0.001f);
		}
	}


	_GameScene.trailTicks--;
	if(_GameScene.trailTicks == 0)
	{
		_GameScene.trailTicks = TRAIL_REFRESH;
		_GameScene.trail[_GameScene.currentIndexToUpdate].position = ball->position;
		_GameScene.currentIndexToUpdate++;
		if(_GameScene.currentIndexToUpdate >= TRAIL_SIZE)
		{
			_GameScene.currentIndexToUpdate = 0;
		}
	}

	eulerIntegration(ball, dt);
	updateWallCollisions(_GameScene.WallCollisions, ball, dt);

	Collision collision;
	for (int i = 0; i < NUMBER_OF_BREAKABLES_TOTAL; ++i)
	{

		if (_GameScene.breakables[i].center.x > 9000)
		{
			continue;
		}


		if (findPointRectCollision(*ball, _GameScene.breakables[i], &collision))
		{
			{
				_GameScene.breakables[i].center = VEC2(10000, 0);
				// resolve the constraint
				ball->position = collision.position;
			}

			// compute the normal & tangential velocity
			Vec2 normalVelocity;
			vec2MulScalar(collision.normal, vec2Dot(collision.normal, ball->velocity), &normalVelocity);
			Vec2 tangentialVelocity;
			vec2Sub(ball->velocity, normalVelocity, &tangentialVelocity);

			float elasticity = 1.03f;

			// apply bouncing
			vec2MulScalar(normalVelocity, -elasticity, &normalVelocity);

			// add up the new velocity
			vec2Add(normalVelocity, tangentialVelocity, &ball->velocity);
		}
	}


	if (findPointRectCollision(*ball, *paddle, &collision))
	{
		{
			// resolve the constraint
			ball->position = collision.position;
		}

		Vec2 temp;
		vec2Sub(collision.position, paddle->center, &temp);
		vec2Normalize(&temp);
		vec2MulScalar(temp, 0.1f, &temp);

		vec2Add(collision.normal, temp, &collision.normal);
		vec2Normalize(&collision.normal);

		// compute the normal & tangential velocity
		Vec2 normalVelocity;
		vec2MulScalar(collision.normal, vec2Dot(collision.normal, ball->velocity), &normalVelocity);
		Vec2 tangentialVelocity;
		vec2Sub(ball->velocity, normalVelocity, &tangentialVelocity);

		float elasticity = 1.0f;

		// apply bouncing
		vec2MulScalar(normalVelocity, -elasticity, &normalVelocity);

		float friction = 1.0f;
		float frictionDelta = expf(-friction * dt);
		// apply friction
		//vec2MulScalar(tangentialVelocity, frictionDelta, &tangentialVelocity);

		// add up the new velocity
		vec2Add(normalVelocity, tangentialVelocity, &ball->velocity);
	}

	// render
	pd->graphics->clear(kColorWhite);

	msgfx_drawLineArgs arg1 =
	{
		MAX_W_WIDTH - 10, 0,
		-MAX_W_WIDTH + 10, 0,
		1,
		kColorBlack
	};

	msgfx_drawLineArgs arg2 =
	{
		0, MAX_W_HEIGHT - 10,
		0, -MAX_W_HEIGHT + 10,
		1,
		kColorBlack
	};

	//msgfx_drawLine(pd, &_GameScene.camera, arg1);
	//msgfx_drawLine(pd, &_GameScene.camera, arg2);

	for (int i = 0; i < TRAIL_SIZE; i++)
	{
		renderPoint(pd, &_GameScene.camera, &_GameScene.trail[i]);
	}

	renderPoint(pd, &_GameScene.camera, &_GameScene.ball);
	renderRect(pd, &_GameScene.camera, &_GameScene.paddle);



	for (int i = 0; i < NUMBER_OF_BREAKABLES_TOTAL; ++i)
	{
		renderRect(pd, &_GameScene.camera, &_GameScene.breakables[i]);
	}

	if (ball->position.y < -300)
	{
		initializeScene();
	}

	if (vec2Len(ball->velocity) > MAX_VELOCITY)
	{
		vec2Normalize(&ball->velocity);
		vec2MulScalar(ball->velocity, MAX_VELOCITY, &ball->velocity);
	}

	//pd->system->drawFPS(0, 0);
	pd->graphics->drawText("Simple Breakout by Manuel Silva", 32, kASCIIEncoding, 10, MAX_HEIGHT - 20);
	return 1;
}

