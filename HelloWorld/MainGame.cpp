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

enum GameObjectTupe 
{
	TYPE_NULL = -1,
	TYPE_AGENT8,
	TYPE_FAN,
	TYPE_TOOL,
	TYPE_COIN,
	TYPE_STAR,
	TYPE_LASER,
	TYPE_DESTROYED,
};

// Function to handle player movement
void HandlePlayerControls();

// Function to update the fan in-game
void UpdateFan();

// Function to update the tools in-game
void UpdateTools();


// The entry point for a PlayBuffer program, Start/Awake
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	Play::CreateManager( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE );
	Play::CentreAllSpriteOrigins();
	Play::LoadBackground("Data\\Backgrounds\\background.png");
	Play::StartAudioLoop("music");
	Play::CreateGameObject(TYPE_AGENT8, { 115,0 }, 50, "agent8");
	
	// Instantiate in unity
	int id_fan = Play::CreateGameObject(TYPE_FAN, { 1140,217 }, 0, "fan");
	Play::GetGameObject(id_fan).velocity = { 0,3 };
	Play::GetGameObject(id_fan).animSpeed = 1.0f;

}

// Called by PlayBuffer every frame (60 times a second!), same as update in Unity
bool MainGameUpdate( float elapsedTime )
{
	Play::DrawBackground();
	HandlePlayerControls();
	UpdateFan();
	UpdateTools();
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


	// Spider like movement, down is much faster than going up.
	
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

void UpdateFan() 
{
	// Gets the fan
	GameObject& obj_fan = Play::GetGameObjectByType(TYPE_FAN);
	// Draws it into the scene
	Play::DrawObject(obj_fan);

	// Generates random number between 1-50 each frame, if its 50 it spawns driver and pushes towards the player.
	if (Play::RandomRoll(50) == 50)
	{
		int id = Play::CreateGameObject(TYPE_TOOL, obj_fan.pos, 50, "driver");
		GameObject& obj_tool = Play::GetGameObject(id);
		obj_tool.velocity = Point2f(-8, Play::RandomRollRange(-1, 1) * 6);

		// Then rolls a 50/50 to decide if its going to be a spanner instead of driver
		// Uses the obj_tool that is already created, but changes the sprite and radius to adjust for it being spanner.
		if (Play::RandomRoll(2) == 1)
		{
			Play::SetSprite(obj_tool, "spanner", 0);
			obj_tool.radius = 100;
			obj_tool.velocity.x = -4;
			obj_tool.rotSpeed = 0.1f;
		}
		Play::PlayAudio("tool");
	}
	if (Play::RandomRoll(150) == 1) 
	{
		int id = Play::CreateGameObject(TYPE_COIN, obj_fan.pos, 40, "coin");
		GameObject& obj_coin = Play::GetGameObject(id);
		obj_coin.velocity = { -3, 0 };
		obj_coin.rotSpeed = 0.1f;
	}

	Play::UpdateGameObject(obj_fan);

	if (Play::IsLeavingDisplayArea(obj_fan))
	{
		obj_fan.pos = obj_fan.oldPos;
		obj_fan.velocity.y *= -1;
	}
}

void UpdateTools() 
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	std::vector<int> vTools = Play::CollectGameObjectIDsByType(TYPE_TOOL);

	for (int id : vTools)
	{
		GameObject& obj_tool = Play::GetGameObject(id);

		if (Play::IsColliding(obj_tool, obj_agent8))
		{
			Play::StopAudioLoop("music");
			Play::PlayAudio("die");
			obj_agent8.pos = { -100, -100 };
		}
		Play::UpdateGameObject(obj_tool);

		if (Play::IsLeavingDisplayArea(obj_tool, Play::VERTICAL))
		{
			obj_tool.pos = obj_tool.oldPos;
			obj_tool.velocity.y *= -1;
		}

		Play::DrawObjectRotated(obj_tool);

		//Once it goes out of playable area, destroys the object
		if (!Play::IsVisible(obj_tool))
		{
			Play::DestroyGameObject(id);
		}
	}
}

