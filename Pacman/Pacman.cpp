#include "Pacman.h"
#include <time.h>
#include <sstream>
#include <fstream> // Include the header file for file stream operations (ifstream and ofstream)
#include <iostream>
#include <iomanip>

Pacman::Pacman(int argc, char* argv[]) : Game(argc, argv), _cPacmanSpeed(0.1f), _cPacmanFrameTime(250), _cMunchieFrameTime(500)
{
	Pacman_Player = new Player();	Pacman_Player->dead = false;
	_pacmanCurrentFrameTime = 0;
	_pacmanFrame = 0;
	_frameCount = 0;
	_munchieCurrentFrameTime = 0;
	// Add a bool variable to keep track of cherry visibility
	bool _isCherryVisible = true;
	_paused = false;
	srand(time(NULL));
	int i;
	for (i = 0; i < MUNCHIECOUNT; i++)
	{
		_munchies[i] = new Collectable();
		_munchies[i]->frameCount = rand() % 1;
		_munchies[i]->currentFrameTime = 0;
		_munchies[i]->frameTime = rand() % 500 + 50;
	}

	_ghosts[0] = new MovingEnemy();
	_ghosts[0]->direction = 0;
	_ghosts[0]->speed = 0.2f;

	//Initialise important Game aspects
	Graphics::Initialise(argc, argv, this, 1024, 768, false, 25, 25, "Pacman", 60);
	Input::Initialise();
	// Start the Game Loop - This calls Update and Draw in game loop 
	Graphics::StartGameLoop();
}
Pacman::~Pacman()
{
	delete Pacman_Player->texture;	/*delete _pacmanSourceRect;*/	delete Pacman_Player->position;
	delete _munchieBlueTexture;	delete _munchieInvertedTexture;	delete _munchieRect;

	//Clean up the Texture
	delete _munchies[0]->texture;
	int nCount = 0;
	for (nCount = 0; nCount < MUNCHIECOUNT; nCount++)
	{
		delete _munchies[nCount]->position;
		delete _munchies[nCount]->sourceRect;
		delete _munchies[nCount];
	}
	//Order of operations is important, array deleted last
	delete[] _munchies;
}
void Pacman::LoadContent()
{
	// Load Pacman
	Pacman_Player->texture->Load("Textures/Pacman.tga", false); Pacman_Player->sourceRect = new Rect(0,0, 32, 32);	Pacman_Player->position = new Vector2(350.0f, 350.0f);	//_pacmanSourceRect = new Rect(0.0f, 0.0f, 32, 32);
	// Initialize game state, score, and high score
	Pacman_Player->dead = false;
	_score = 50;	_highScore = 0;	LoadHighScore();
	// Load Cherry Inverted
	_cherryInvertedTexture = new Texture2D();	_cherryInvertedTexture->Load("Textures/CherryInverted.png", false);	_cherryPosition = new Vector2((rand() % Graphics::GetViewportWidth()), (rand() % Graphics::GetViewportHeight()));
	_cherryTexture = new Texture2D();			_cherryTexture->Load("Textures/Cherry.png", false);
	_cherrySpeed = 0.1f;
	//_cherryPosition = new Vector2((rand() % Graphics::GetViewportWidth()), (rand() % Graphics::GetViewportHeight()));
	// Set the duration (in milliseconds) for displaying the "Start Game" message
	_startMessageDisplayTime = 3000; // 3 seconds
	_showStartMessage = true;
	//Load Ghost
	_ghosts[0]->texture = new Texture2D();	_ghosts[0]->texture->Load("Textures/GhostBlue.png", false);	_ghosts[0]->position = new Vector2((rand() % Graphics::GetViewportWidth()), (rand() % Graphics::GetViewportHeight()));
	_ghosts[0]->sourceRect = new Rect(0, 0, 20, 20);

	_startSound = new S2D::SoundEffect();	_startSound->Load("sound/pop.wav");

	_munchieBlueTexture = new Texture2D();		_munchieBlueTexture->Load("Textures/Munchie.tga", true);// Load Munchies
	int i;
	for (i = 0; i < MUNCHIECOUNT; i++) {
		_munchies[i]->texture = _munchieBlueTexture;
		_munchies[i]->sourceRect = new Rect((rand() % Graphics::GetViewportWidth()), (rand() % Graphics::GetViewportHeight()), 12, 12);
		_munchies[i]->position = new Vector2(_munchies[i]->sourceRect->X, _munchies[i]->sourceRect->Y);
		//_munchies[i]->position = new Vector2((rand() % Graphics::GetViewportWidth()), (rand() % Graphics::GetViewportHeight()));
	}
	_munchieInvertedTexture = new Texture2D();	_munchieInvertedTexture->Load("Textures/MunchieInverted.tga", true);//	_munchieRect = new Rect(100.0f, 450.0f, 12, 12);

	// Set string position
	_stringPosition = new Vector2(10.0f, 25.0f);

	// Set Menu Paramters
	_menuBackground = new Texture2D();
	_menuBackground->Load("Textures/Transparency.png", false);
	_menuRectangle = new Rect(0.0f, 0.0f, Graphics::GetViewportWidth(), Graphics::GetViewportHeight());
	_menuStringPosition = new Vector2(Graphics::GetViewportWidth() / 2.0f, Graphics::GetViewportHeight() / 2.0f);
}
void Pacman::Update(int elapsedTime)
{
	_pacmanCurrentFrameTime += elapsedTime;
	// Gets the current state of the keyboard
	Input::KeyboardState* keyboardState = Input::Keyboard::GetState();
	if (_showStartMessage && _startMessageDisplayTime == 3000) // Play sound only on the first frame when the message appears
	{
		Audio::Play(_startSound);
	}
	if (_showStartMessage) //dont do anything until game starts
		return;

	if (!Pacman_Player->dead) // Only update the game if it's not over
	{
		UpdateCherryCollision();

		if (keyboardState->IsKeyDown(Input::Keys::P) && !_pKeyDown)
		{
			_pKeyDown = true;
			_paused = !_paused;
		}

		if (keyboardState->IsKeyUp(Input::Keys::P))
			_pKeyDown = false;

		if (!_paused)
		{
			_frameCount++;
			UpdateGhost(_ghosts[0], elapsedTime);
			CheckGhostCollisions();
			// Checks if key is pressed
			if (keyboardState->IsKeyDown(Input::Keys::RIGHT))
				pacmanDirection = Right;
			if (keyboardState->IsKeyDown(Input::Keys::UP))
				pacmanDirection = Up;
			if (keyboardState->IsKeyDown(Input::Keys::LEFT))
				pacmanDirection = Left;
			if (keyboardState->IsKeyDown(Input::Keys::DOWN))
				pacmanDirection = Down;

			if (pacmanDirection == Right)
				Pacman_Player->position->X += _cPacmanSpeed * elapsedTime;
			if (pacmanDirection == Up)
				Pacman_Player->position->Y -= _cPacmanSpeed * elapsedTime;
			if (pacmanDirection == Left)
				Pacman_Player->position->X -= _cPacmanSpeed * elapsedTime;
			if (pacmanDirection == Down)
				Pacman_Player->position->Y += _cPacmanSpeed * elapsedTime;

			Pacman_Player->sourceRect->Y = Pacman_Player->sourceRect->Height * pacmanDirection;	//Changing Pacman Texture Direction
			// Checks if Pacman is trying to disappear
			if (Pacman_Player->position->X > Graphics::GetViewportWidth() - 0.5 * Pacman_Player->sourceRect->Width)
				Pacman_Player->position->X = 0 - 0.5 * Pacman_Player->sourceRect->Width;
			if (Pacman_Player->position->X < 0 - 0.5 * Pacman_Player->sourceRect->Width)
				Pacman_Player->position->X = Graphics::GetViewportWidth() - 0.5 * Pacman_Player->sourceRect->Width;
			if (Pacman_Player->position->Y > Graphics::GetViewportHeight() - 0.5 * Pacman_Player->sourceRect->Width)
				Pacman_Player->position->Y = 0 - 0.5 * Pacman_Player->sourceRect->Height;
			if (Pacman_Player->position->Y < 0 - 0.5 * Pacman_Player->sourceRect->Height)
				Pacman_Player->position->Y = Graphics::GetViewportHeight() - 0.5 * Pacman_Player->sourceRect->Height;

			if (_pacmanCurrentFrameTime > _cPacmanFrameTime)
			{
				_pacmanFrame++;
				if (_pacmanFrame >= 2)
					_pacmanFrame = 0;

				_pacmanCurrentFrameTime = 0;

				Pacman_Player->sourceRect->X = Pacman_Player->sourceRect->Width * _pacmanFrame;
			}

			// Check for collision with screen walls
			if (Pacman_Player->position->X < 0 || Pacman_Player->position->X > Graphics::GetViewportWidth() ||
				Pacman_Player->position->Y < 0 || Pacman_Player->position->Y > Graphics::GetViewportHeight())
			{
				Pacman_Player->dead = true;
			}
			for (int i = 0; i < MUNCHIECOUNT; i++)
			{
				if (Munchies_Collision(_munchies[i]))
					UpdateMunchies(_munchies[i]);
			}
		}
	}
	else // Game is over
	{
		// Check for restart input
		if (keyboardState->IsKeyDown(Input::Keys::R))
		{
			// Restart the game
			Pacman_Player->dead = false;
			_score = 0;
			Pacman_Player->position = new Vector2(350.0f, 350.0f); // Reset Pacman's position
			_cherryPosition = new Vector2((rand() % Graphics::GetViewportWidth()), (rand() % Graphics::GetViewportHeight())); // Reset cherry's position
			_frameCount = 0;
			_pacmanFrame = 0;
			_pacmanCurrentFrameTime = 0;
			_paused = false;
			_showStartMessage = true;
			_startMessageDisplayTime = 3000;
			LoadHighScore(); // Load high score from file
		}
	}
}
void Pacman::Draw(int elapsedTime)
{
	std::stringstream stream;	stream << "Pacman X: " << std::fixed << std::setprecision(1) << Pacman_Player->position->X << "\tY: " << std::fixed << std::setprecision(1) << Pacman_Player->position->Y << std::endl; //retain only 1st decimal
	//stream << "Pacman X: " << Pacman_Player->position->X << "\tY: " << Pacman_Player->position->Y;
	SpriteBatch::BeginDraw(); // Starts Drawing
	if (Pacman_Player->dead) { DrawEndGame(); }
	else
	{
		// Show "Start Game" message
		if (_showStartMessage)
		{
			std::stringstream startMessageStream;
			startMessageStream << "Start Game!";

			// You can customize the position and color of the message as needed
			Vector2 startMessagePosition((Graphics::GetViewportWidth()-5) / 2.0f, Graphics::GetViewportHeight() / 2.0f);
			SpriteBatch::DrawString(startMessageStream.str().c_str(), &startMessagePosition, Color::Green);

			// Check if it's time to hide the message
			_startMessageDisplayTime -= elapsedTime;
			if (_startMessageDisplayTime <= 0)
				_showStartMessage = false;

		}
		else // "Start Game" message is not visible
		{
			// Draws Pacman and other game elements
			SpriteBatch::Draw(Pacman_Player->texture, Pacman_Player->position, Pacman_Player->sourceRect);
			// Show munchie
			for (int i = 0; i < MUNCHIECOUNT; i++)
			{
				if (_munchies[i]->position->X == -1)
					continue;
				if (_frameCount < 30)
					//SpriteBatch::Draw(_munchieInvertedTexture, _munchies[i]->position, _munchies[i]->sourceRect);
					SpriteBatch::Draw(_munchieInvertedTexture, _munchies[i]->sourceRect, nullptr, Vector2::Zero, 1.0f, 0.0f, Color::White, SpriteEffect::NONE);	// Draw Red Munchie
				else
					SpriteBatch::Draw(_munchieBlueTexture, _munchies[i]->sourceRect, nullptr, Vector2::Zero, 1.0f, 0.0f, Color::White, SpriteEffect::NONE);		// Draw Blue Munchie
			}
			if (_frameCount < 30) {
				SpriteBatch::Draw(_cherryInvertedTexture, _cherryPosition);
			}
			else
			{
				SpriteBatch::Draw(_cherryTexture, _cherryPosition);

				if (_frameCount >= 60)
					_frameCount = 0;
			}
			SpriteBatch::Draw(_ghosts[0]->texture, _ghosts[0]->position, _ghosts[0]->sourceRect);
			// Draws String
			SpriteBatch::DrawString(stream.str().c_str(), _stringPosition, Color::Green);

			// Show "PAUSED!" message if the game is paused
			if (_paused)
			{
				std::stringstream menuStream;
				menuStream << "PAUSED!";
				SpriteBatch::Draw(_menuBackground, _menuRectangle, nullptr);
				SpriteBatch::DrawString(menuStream.str().c_str(), _menuStringPosition, Color::Red);
			}
		}
		//if (_isCherryVisible)
		//{
			// Draw the cherries if they are visible
			//SpriteBatch::Draw(_cherryTexture, _cherryPosition);
			//SpriteBatch::Draw(_cherryInvertedTexture, _cherryInvertedPosition);
		//}

		// Draw the score
		std::stringstream scoreStream;
		scoreStream << "Score: " << _score;
		Vector2 scorePosition((Graphics::GetViewportWidth() - 3) / 2, 30.0f); // You can adjust the position of the score on the screen
		SpriteBatch::DrawString(scoreStream.str().c_str(), &scorePosition, Color::White);
	}
	SpriteBatch::EndDraw(); // Ends Drawing
}
void Pacman::CheckGhostCollisions() {
	// Local Variables
	int i = 0;
	int bottom1 = Pacman_Player->position->Y + Pacman_Player->sourceRect->Height;
	int bottom2 = 0;
	int left1 = Pacman_Player->position->X;
	int left2 = 0;
	int right1 = Pacman_Player->position->X + Pacman_Player->sourceRect->Width;
	int right2 = 0;
	int top1 = Pacman_Player->position->Y;
	int top2 = 0;
	for (i = 0; i < GHOSTCOUNT; i++)
	{
		// Populate variables with Ghost data
		bottom2 = _ghosts[i]->position->Y + _ghosts[i]->sourceRect->Height;
		left2 = _ghosts[i]->position->X;
		right2 = _ghosts[i]->position->X + _ghosts[i]->sourceRect->Width;
		top2 = _ghosts[i]->position->Y;
		if ((bottom1 > top2) && (top1 < bottom2) && (right1 > left2) && (left1 < right2))
		{
			Pacman_Player->dead = true;
			i = GHOSTCOUNT;
		}
	}
}
void Pacman::UpdateMunchies(Collectable* Munchie) {
	Munchie->position->X = -1;
	Munchie->position->Y = -1;
	Munchie->sourceRect->X = -1;
	Munchie->sourceRect->Y = -1;
}
bool Pacman::Munchies_Collision(Collectable* Munchie) {
	if (Pacman_Player->position->X + Pacman_Player->sourceRect->Width > Munchie->position->X &&
		Pacman_Player->position->X < Munchie->position->X + Munchie->texture->GetWidth() &&
		Pacman_Player->position->Y + Pacman_Player->sourceRect->Height > Munchie->position->Y &&
		Pacman_Player->position->Y < Munchie->position->Y + Munchie->texture->GetHeight())
	
		return true;
		return false;	
}
void Pacman::UpdateGhost(MovingEnemy* ghost, int elapsedTime)
{
	if (ghost->direction == 0) //Moves Right
		ghost->position->X += ghost->speed * elapsedTime;
	else if (ghost->direction == 1) //Moves Left
		ghost->position->X -= ghost->speed * elapsedTime;
	if (ghost->position->X + ghost->sourceRect->Width >= Graphics::GetViewportWidth()) //Hits Right edge
		ghost->direction = 1; //Change direction
	else if (ghost->position->X <= 0) //Hits left edge
		ghost->direction = 0; //Change direction
	//_pacmanSourceRect->Y = _pacmanSourceRect->Height * pacmanDirection;

	//ghost->sourceRect->Y = ghost->sourceRect->Height * ghost->direction;
	//_pacmanSourceRect->Y = _pacmanSourceRect->Height * pacmanDirection;

}
void Pacman::DrawEndGame()
{
	// Check and update high score if necessary
	if (_score >= _highScore)
	{
		_highScore = _score;
		SaveHighScore(); // Save the new high score to the file
	}
	// Draw the "End Game" screen with score and high score table
	std::stringstream endGameStream;
	endGameStream << "GAME OVER\n";
	endGameStream << "Game Stats are:\n";
	endGameStream << "Your Score: " << _score << "\n";
	endGameStream << "High Score: " << _highScore << "\n";
	endGameStream << "Press R to restart";

	// You can customize the position and color of the message as needed
	Vector2 endGamePosition(Graphics::GetViewportWidth() / 2.0f, Graphics::GetViewportHeight() / 2.0f);
	SpriteBatch::DrawString(endGameStream.str().c_str(), &endGamePosition, Color::Red);

	// Check for restart input
	Input::KeyboardState* keyboardState = Input::Keyboard::GetState();
	if (keyboardState->IsKeyDown(Input::Keys::R))
	{
		// Restart the game
		Pacman_Player->dead = false;
		_score = 0;
		Pacman_Player->position = new Vector2(350.0f, 350.0f); // Reset Pacman's position
		_cherryPosition = new Vector2((rand() % Graphics::GetViewportWidth()), (rand() % Graphics::GetViewportHeight())); // Reset cherry's position
		_frameCount = 0;
		_pacmanFrame = 0;
		_pacmanCurrentFrameTime = 0;
		_paused = false;
		_showStartMessage = true;
		_startMessageDisplayTime = 3000;
		LoadHighScore(); // Load high score from file
	}
}
void Pacman::UpdateScore()// Increment the score
{
	_score += 10; // Adjust the score increment as needed
}
void Pacman::LoadHighScore()
{
	std::ifstream file("highscore.txt");
	if (file.is_open())
	{
		file >> _highScore;
		file.close();
	}
}
void Pacman::SaveHighScore()
{
	std::ofstream file("highscore.txt");
	if (file.is_open())
	{
		//file << "highscore=" << _highScore;
		file << _highScore;
		if (!file.fail())
		{
			file.close();
			std::cout << "High score saved: " << _highScore << std::endl;
		}
		else
		{
			std::cout << "Error: Failed to write the high score." << std::endl;
		}
	}
	else
	{
		std::cout << "Error: Unable to open the highscore.txt file for writing." << std::endl;
	}
}
void Pacman::UpdateCherryCollision()
{
	// Check for collision between Pacman and the cherry
	if (Pacman_Player->position->X + Pacman_Player->sourceRect->Width > _cherryPosition->X &&
		Pacman_Player->position->X < _cherryPosition->X + _cherryTexture->GetWidth() &&
		Pacman_Player->position->Y + Pacman_Player->sourceRect->Height > _cherryPosition->Y &&
		Pacman_Player->position->Y < _cherryPosition->Y + _cherryTexture->GetHeight())
	{
		// Pacman collided with the cherry
		UpdateScore();

		// Respawn the cherry at a random position within the visible area
		_cherryPosition->X = rand() % (Graphics::GetViewportWidth() - _cherryTexture->GetWidth());
		_cherryPosition->Y = rand() % (Graphics::GetViewportHeight() - _cherryTexture->GetHeight());
	}
}
