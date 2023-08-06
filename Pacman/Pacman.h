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
	Player() { texture = new Texture2D(); }
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
};
class Pacman : public Game
{
private:
	//Input methods
	void Input(int elapsedTime, Input::KeyboardState* state);
	Texture2D* _cherryTexture;
	Vector2* _cherryPosition;
	//Check methods
	void CheckPaused(Input::KeyboardState* state, Input::Keys pauseKey);
	void CheckViewportCollision();
	// New variables for game state, score, and high score
	//bool _isGameOver;
	int _highScore;
	//Update methods
	void UpdatePacman(int elapsedTime);
	void UpdateMunchies(Collectable*);
	bool Munchies_Collision(Collectable* Munchie);
	S2D::SoundEffect* _startSound;
	// Data to represent Pacman
	Player* Pacman_Player;
	//Vector2* _pacmanPosition;
	//Rect* _pacmanSourceRect;
	int _pacmanFrame;
	int _pacmanCurrentFrameTime;

	MovingEnemy* _ghosts[GHOSTCOUNT];
	void CheckGhostCollisions();

	void UpdateGhost(MovingEnemy*, int elapsedTime);
	//Constant data for Game Variables 
	float _cPacmanSpeed;

	enum direction { Right, Down, Left, Up };
	direction pacmanDirection;

	// Data to represent Munchie
	Collectable* _munchies[MUNCHIECOUNT];
	int _frameCount;
	Rect* _munchieRect;
	Texture2D* _munchieBlueTexture;
	Texture2D* _munchieInvertedTexture;

	int _munchieFrame;
	int _munchieCurrentFrameTime;
	Texture2D* _cherryInvertedTexture;
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
	bool _isCherryVisible;

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