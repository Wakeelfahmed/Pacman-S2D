#pragma once

// If Windows and not in Debug, this will run without a console window
// You can use this to output information when debugging using cout or cerr
#ifdef WIN32 
#ifndef _DEBUG
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif
#endif
#define MUNCHIECOUNT 50
#define GHOSTCOUNT 1

// Just need to include main header file
#include "S2D/S2D.h"

// Reduces the amount of typing by including all classes in S2D namespace
using namespace S2D;

// Declares the Pacman class which inherits from the Game class.
// This allows us to overload the Game class methods to help us
// load content, draw and update our game.
struct Player
{
	float speedMultiplier;
	int currentFrameTime;
	int frame;
	Rect* sourceRect;
	Texture2D* texture;
	Vector2* position;
	bool dead;
	void Initialise_Player() { texture = new Texture2D();	texture->Load("Textures/Pacman.tga", false);	dead = false;	position = new Vector2(350.0f, 350.0f);	sourceRect = new Rect(0, 0, 32, 32); }
};
struct Collectable
{
	int currentFrameTime;
	int frame;
	int frameCount;
	int frameTime;
	Rect* sourceRect;
	Texture2D* texture;
	Vector2* position;
};
struct MovingEnemy
{
	Vector2* position;
	Texture2D* texture;
	Rect* sourceRect;
	int direction;
	float speed;
	void Initialise_Ghost() {
		texture = new Texture2D();	texture->Load("Textures/GhostBlue.png", false);	direction = 0;	speed = 0.2f;
		position = new Vector2((rand() % Graphics::GetViewportWidth()), (rand() % Graphics::GetViewportHeight()));		sourceRect = new Rect(0, 0, 20, 20);
	}
};
class Pacman : public Game
{
private:
	//Input methods
	void Input(int elapsedTime, Input::KeyboardState* state);

	//Check methods
	void CheckPaused(Input::KeyboardState* state, Input::Keys pauseKey);
	void CheckViewportCollision();
	// New variables for game state, score, and high score
	int _highScore;
	//Update methods
	void UpdatePacman(int elapsedTime);
	void UpdateMunchies(Collectable*);
	bool Munchies_Collision(Collectable* Munchie);
	S2D::SoundEffect* _startSound;
	// Data to represent Pacman
	Player* Pacman_Player;
	int _pacmanFrame;
	int _pacmanCurrentFrameTime;

	MovingEnemy* _ghosts[GHOSTCOUNT];
	void CheckGhostCollisions();
	void UpdateGhost(MovingEnemy*, int elapsedTime);
	//Constant data for Game Variables 
	float _cPacmanSpeed;

	enum direction { Right, Down, Left, Up };	direction pacmanDirection;

	// Data to represent Munchie
	Collectable* _munchies[MUNCHIECOUNT];
	int _frameCount;
	Rect* _munchieRect;
	Texture2D* _munchieBlueTexture;		Texture2D* _munchieInvertedTexture;
	int _munchieFrame;
	int _munchieCurrentFrameTime;
	Texture2D* _cherryInvertedTexture;	Texture2D* _cherryTexture;	Vector2* _cherryPosition;
	float _cherrySpeed;   // Add this variable to control cherry's movement speed
	// Position for String
	Vector2* _stringPosition;
	bool _showStartMessage;
	int _startMessageDisplayTime; // Declaration of the variable here
	int _score;
	// Data for Menu
	Texture2D* _menuBackground;
	Rect* _menuRectangle;
	Vector2* _menuStringPosition;
	bool _paused;
	bool _pKeyDown;
	// New function to show the "End Game" screen
	void DrawEndGame();
	// Constants
	int _cPacmanFrameTime;
	int _cMunchieFrameTime;
	// Other member variables...

	// Function to handle cherry collision
	void UpdateCherryCollision();
	// Function to update the score and high score
	void UpdateScore();

	// Function to load high score from a file
	void LoadHighScore();

	// Function to save high score to a file
	void SaveHighScore();
public:
	// <summary> Constructs the Pacman class. </summary>
	Pacman(int argc, char* argv[]);

	// <summary> Destroys any data associated with Pacman class. </summary>
	virtual ~Pacman();

	// <summary> All content should be loaded in this method. </summary>
	void virtual LoadContent();

	// <summary> Called every frame - update game logic here. </summary>
	void virtual Update(int elapsedTime);

	// <summary> Called every frame - draw game here. </summary>
	void virtual Draw(int elapsedTime);
};