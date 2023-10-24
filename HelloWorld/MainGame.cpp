#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

int DISPLAY_WIDTH = 1280;
int DISPLAY_HEIGHT = 720;
int DISPLAY_SCALE = 1;

enum Agent8State
{
	STATE_APPEAR =0,
	STATE_HALT,
	STATE_PLAY,
	STATE_DEAD,
};


struct GameState 
{
	int score = 0;
	Agent8State agentState = STATE_APPEAR;
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

// Function to handle coins and stars spawning in
void UpdateCoinsAndStars();

// Function to handle lasers
void UpdateLasers();

// Function to fade out objects that are destroyed
void UpdateDestroyed();

// Function to handle states of Agent8
void UpdateAgent8();

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
	UpdateAgent8();
	UpdateFan();
	UpdateTools();
	UpdateCoinsAndStars();
	UpdateLasers();
	UpdateDestroyed();
	
	// Draws instructions onscreen.
	Play::DrawFontText("64px", "ARROW KEYS TO MOVE UP AND DOWN AND SPACE TO FIRE",
		{ DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 30 }, Play::CENTRE);

	// Draws score onscreen.
	Play::DrawFontText("132px", "SCORE: " + std::to_string(gameState.score),
		{ DISPLAY_WIDTH / 2, 50 }, Play::CENTRE);
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
		if (obj_agent8.velocity.y > 5) 
		{
			gameState.agentState = STATE_HALT;
			Play::SetSprite(obj_agent8, "agent8_halt", 0.333f);
			obj_agent8.acceleration = { 0,0 };
		}
		else
		{
			Play::SetSprite(obj_agent8, "agent8_hang", 0.0f);
			obj_agent8.velocity *= 0.5f;
			obj_agent8.acceleration = { 0,0 };
		}
		
	}

	if (Play::KeyPressed(VK_SPACE))
	{
		Vector2D firePos = obj_agent8.pos + Vector2D(155, -75);
		int id = Play::CreateGameObject(TYPE_LASER, firePos, 30, "laser");
		Play::GetGameObject(id).velocity = { 32,0 };
		Play::PlayAudio("shoot");
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
	
	// Generates a number between 1-150, if == 1 it creates a coin for the player to grab
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

		if (gameState.agentState != STATE_DEAD && Play::IsColliding(obj_tool,obj_agent8))
		{
			Play::StopAudioLoop("music");
			Play::PlayAudio("die");
			gameState.agentState = STATE_DEAD;
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

void UpdateCoinsAndStars()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	std::vector<int> vCoins = Play::CollectGameObjectIDsByType(TYPE_COIN);

	for (int id_coin : vCoins)
	{
		GameObject& obj_coin = Play::GetGameObject(id_coin);
		bool hasCollided = false;

		// Checking if the coin has collided with the player
		if (Play::IsColliding(obj_coin, obj_agent8)) 
		{
			for (float rad{ 0.25f }; rad < 2.0f; rad += 0.5f)
			{
				// Spawns in stars evenly around the coin once the coin has been collided with.
				int id = Play::CreateGameObject(TYPE_STAR, obj_agent8.pos, 0, "star");
				GameObject& obj_star = Play::GetGameObject(id);
				obj_star.rotSpeed = 0.1f;
				obj_star.acceleration = { 0.0f,0.5f };
				Play::SetGameObjectDirection(obj_star, 16, rad * PLAY_PI);
			}

			hasCollided = true;
			// Adds score to player and plays audio.
			gameState.score += 500;
			Play::PlayAudio("collect");
		}

		Play::UpdateGameObject(obj_coin);
		Play::DrawObjectRotated(obj_coin);

		if (!Play::IsVisible(obj_coin) || hasCollided)
		{
			Play::DestroyGameObject(id_coin);
		}
	}

	std::vector<int> vStars = Play::CollectGameObjectIDsByType(TYPE_STAR);

	for (int id_star : vStars) 
	{
		GameObject& obj_star = Play::GetGameObject(id_star);

		Play::UpdateGameObject(obj_star);
		Play::DrawObjectRotated(obj_star);

		// if its not visible will destory the stars
		if (!Play::IsVisible(obj_star))
		{
			Play::DestroyGameObject(id_star);
		}
	}
}

void UpdateLasers()
{
	std::vector<int> vLasers = Play::CollectGameObjectIDsByType(TYPE_LASER);
	std::vector<int> vTools = Play::CollectGameObjectIDsByType(TYPE_TOOL);
	std::vector<int> vCoins = Play::CollectGameObjectIDsByType(TYPE_COIN);

	for (int id_laser : vLasers)
	{
		GameObject& obj_laser = Play::GetGameObject(id_laser);
		bool hasCollided = false;

		for (int id_tool : vTools)
		{
			GameObject& obj_tool = Play::GetGameObject(id_tool);
			if (Play::IsColliding(obj_laser, obj_tool))
			{
				hasCollided = true;
				obj_tool.type = TYPE_DESTROYED;
				gameState.score += 100;
			}
		}

		for (int id_coin : vCoins)
		{
			// If player shoots a coin -300 to score.
			GameObject& obj_coin = Play::GetGameObject(id_coin);
			if (Play::IsColliding(obj_laser, obj_coin))
			{
				hasCollided = true;
				obj_coin.type = TYPE_DESTROYED;
				Play::PlayAudio("error");
				gameState.score -= 300;
			}
		}
		if (gameState.score < 0)
		{
			gameState.score = 0;
		}

		Play::UpdateGameObject(obj_laser);
		Play::DrawObject(obj_laser);

		if (!Play::IsVisible(obj_laser) || hasCollided)
		{
			Play::DestroyGameObject(id_laser);
		}
	}
}

void UpdateDestroyed()
{
	std::vector<int> vDead = Play::CollectGameObjectIDsByType(TYPE_DESTROYED);

	for (int id_dead : vDead)
	{
		GameObject& obj_dead = Play::GetGameObject(id_dead);
		obj_dead.animSpeed = 0.2f;
		Play::UpdateGameObject(obj_dead);

		if (obj_dead.frame % 2)
		{
			Play::DrawObjectRotated(obj_dead, (10 - obj_dead.frame) / 10.0f);
		}

		if (!Play::IsVisible(obj_dead) || obj_dead.frame >= 10)
		{
			Play::DestroyGameObject(id_dead);
		}
	}
}

void UpdateAgent8()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);

	// Switch statement for different states of agent8
	switch (gameState.agentState)
	{
		case STATE_APPEAR:
			obj_agent8.velocity = { 0,12 };
			obj_agent8.acceleration = { 0,0.5f };
			Play::SetSprite(obj_agent8, "agent8_fall", 0);
			obj_agent8.rotation = 0;
			if (obj_agent8.pos.y >= DISPLAY_HEIGHT / 3)
			{
				gameState.agentState = STATE_PLAY;
			}
			break;
	
		case STATE_HALT:
			obj_agent8.velocity *= 0.9f;
			if (Play::IsAnimationComplete(obj_agent8))
			{
				gameState.agentState = STATE_PLAY;
			}
			break;

		case STATE_PLAY:
			HandlePlayerControls();
			break;

		case STATE_DEAD:
			obj_agent8.acceleration = { -0.3f,0.5f };
			obj_agent8.rotation += 0.25f;
			if (Play::KeyPressed(VK_SPACE) == true)
			{
				gameState.agentState = STATE_APPEAR;
				obj_agent8.pos = { 115,00 }; //offset for the laser
				obj_agent8.velocity = { 0,0 };
				obj_agent8.frame = 0;

				Play::StartAudioLoop("music");
				gameState.score = 0;

				for (int id_obj : Play::CollectGameObjectIDsByType(TYPE_TOOL))
				{
					Play::GetGameObject(id_obj).type = TYPE_DESTROYED;
				}
			}
			break;
	}

	Play::UpdateGameObject(obj_agent8);

	if (Play::IsLeavingDisplayArea(obj_agent8) && gameState.agentState != STATE_DEAD)
	{
		obj_agent8.pos = obj_agent8.oldPos;
	}

	Play::DrawLine({ obj_agent8.pos.x, 0 }, obj_agent8.pos, Play::cWhite);
	Play::DrawObjectRotated(obj_agent8);
}