#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

int DISPLAY_WIDTH = 1280;
int DISPLAY_HEIGHT = 720;
int DISPLAY_SCALE = 1;

struct GameState 
{
	int score = 0;
};

GameState gameState;

enum GameObjectTupe {
	TYPE_NULL = -1,
	TYPE_AGENT8,
};

void HandlePlayerControls();

// The entry point for a PlayBuffer program, Start/Awake
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	Play::CreateManager( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE );
}

// Called by PlayBuffer every frame (60 times a second!), same as update in Unity
bool MainGameUpdate( float elapsedTime )
{
	// timer += Time.deltaTime in unity
	gameState.timer += elapsedTime;
	Play::ClearDrawingBuffer( Play::cOrange );

	// Changed where the sprites are being loaded for now.
	Play::DrawDebugText(
		{ DISPLAY_WIDTH / 3, DISPLAY_HEIGHT / 3 },
		Play::GetSpriteName(gameState.spriteId),
		Play::cWhite);

	Play::DrawSprite(gameState.spriteId, Play::GetMousePos(), gameState.timer);

	if (Play::KeyPressed(VK_SPACE)) {
		gameState.spriteId++;
	}

	Play::DrawDebugText( { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 }, "Hello World!" );
	Play::PresentDrawingBuffer();
	return Play::KeyDown( VK_ESCAPE );
}

// Gets called once when the player quits the game, Destroy
int MainGameExit( void )
{
	Play::DestroyManager();
	return PLAY_OK;
}

void HandlePlayerControls() 
{
	// Loads in the Agent GameObject
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);

	// If up arrow is pressed, the agent will change sprite to the climbing animation, as well as change velocity to go up.
	if (Play::KeyDown(VK_UP)) 
	{
		obj_agent8.velocity = { 0,-4 };
		Play::SetSprite(obj_agent8, "agent8_climb", 0.25f);
	}

	// If down arrow is velocity will change to accellerate at a rate of -1 on the y axis each frame, and change sprite to be falling animation.
	else if (Play::KeyDown(VK_DOWN)) 
	{
		obj_agent8.acceleration = { 0,1 };
		Play::SetSprite(obj_agent8, "agent8_fall",0);
	}

	// If up/down arrow is not pressed, the sprite will change to hang animation, then multiply the velocity by 50% each frame.
	else 
	{
		Play::SetSprite(obj_agent8, "agent8_hang", 0.0f);
		obj_agent8.velocity *= 0.5f;
		obj_agent8.acceleration = { 0,0 };
	}

	// Used to save new and old pos, can change the velocity,pos,rotation of the agent. As well as saves animation frame update speed.
	Play::UpdateGameObject(obj_agent8);

	if (Play::IsLeavingDisplayArea(obj_agent8)){
		obj_agent8.pos = obj_agent8.oldPos;
	}
	
	// Draws a debug line from the left to right
	Play::DrawLine({ obj_agent8.pos.x, 0 }, obj_agent8.pos, Play::cWhite);

	// Slower than DrawObject but is used if the rotation/scale is required.
	Play::DrawObjectRotated(obj_agent8);
}
