#pragma once
// If Windows and not in Debug, this will run without a console window
// You can use this to output information when debugging using cout or cerr
#ifdef WIN32 
#ifndef _DEBUG
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif
#endif
#define MUNCHIECOUNT 50
#define GHOSTCOUNT 4

#include "S2D/S2D.h"	// Just need to include main header file

// Reduces the amount of typing by including all classes in S2D namespace
using namespace S2D;
// Declares the Pacman class which inherits from the Game class.
// This allows us to overload the Game class methods to help us
// load content, draw and update our game.
struct Player
{
	float speedMultiplier;
	int currentFrameTime;	int frame;
	Rect* sourceRect;
	Texture2D* texture;
	Vector2* position;
	bool dead;
	Player() { texture = new Texture2D(); position = new Vector2(350.0f, 350.0f);	dead = false;	sourceRect = new Rect(0, 0, 32, 32);	frame = 0;	currentFrameTime = 0;	speedMultiplier = 0.1f; }
	void Initialise_Player() { texture->Load("Textures/Pacman.tga", false);	dead = false;	position->X = position->Y = 350.0f;	sourceRect->X = sourceRect->Y = 0;	sourceRect->Height = sourceRect->Width = 32;	frame = 0;	currentFrameTime = 0; speedMultiplier = 0.1f; }
};
struct Collectable
{
	int currentFrameTime;	int frame;
	int frameCount;		int frameTime;
	Rect* sourceRect;
	Texture2D* texture;
	Vector2* position;
};
int Generate_rand_num_within_PlayableArea_Height();
struct MovingEnemy
{
	Vector2* position;
	Texture2D* texture;
	Rect* sourceRect;
	int direction;	float speed;
	void Initialise_Ghost() {
		texture = new Texture2D();	texture->Load("Textures/GhostBlue.png", false);	direction = 0;	speed = 0.2f;
		position = new Vector2((rand() % Graphics::GetViewportWidth()), Generate_rand_num_within_PlayableArea_Height());		sourceRect = new Rect(0, 0, 20, 20);
		direction = rand() % 4;
	}
};

enum direction { Right, Down, Left, Up };
class Pacman : public Game
{
private:
	int High_score;			int Score;		// New variables for game state, score, and high score
	S2D::SoundEffect* _startSound;
	Player* Pacman_Player;	int Power_Time;		direction pacmanDirection;// Data to represent Pacman
	Collectable* Munchies[MUNCHIECOUNT];	// Data to represent Munchie
	MovingEnemy* Ghosts[GHOSTCOUNT];
	Collectable* Cherry;	Texture2D* _cherryInvertedTexture;

	int _frameCount;

	bool _showStartMessage;		int _startMessageDisplayTime; // Declaration of the variable here

	Texture2D* _menuBackground;	Rect* _menuRectangle;	Vector2* _menuStringPosition;		// Data for Menu
	bool Paused{ false }, Game_Compeleted{ false };	bool _pKeyDown;

	void Input(int elapsedTime, Input::KeyboardState* state);		//Input methods
	void CheckPaused(Input::KeyboardState* state, Input::Keys pauseKey);		//Check methods
	void CheckViewportCollision();		//Check methods
	void UpdatePacman(int elapsedTime);	//Update methods
	void CheckGhostCollisions();			void UpdateGhost(MovingEnemy*, int elapsedTime);
	void UpdateMunchies(Collectable*);		bool Munchies_Collision(Collectable* Munchie);
	void UpdateCherryCollision();	// Function to handle cherry collision
	void DrawEndGame();		// function to show the "End Game" screen (Player dead / Game complete Screen)
	void UpdateScore();	// Function to update the score and high score
	void LoadHighScore();	// Function to load high score from a file
	void SaveHighScore();	// Function to save high score to a file
public:
	Pacman(int argc, char* argv[]);	//Constructs the Pacman class.
	virtual ~Pacman();					//Destroys any data associated with Pacman class.
	void virtual LoadContent();				//All content should be loaded in this method.
	void virtual Update(int elapsedTime);	//Called every frame - update game logic here
	void virtual Draw(int elapsedTime);		//Called every frame - draw game here
};