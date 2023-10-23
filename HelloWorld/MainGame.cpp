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

};


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

