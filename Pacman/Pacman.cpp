#include "Pacman.h"
#include <time.h>
#include <sstream>
#include <fstream> // Include the header file for file stream operations (ifstream and ofstream)
#include <iostream>

Pacman::Pacman(int argc, char* argv[]) : Game(argc, argv), _cPacmanSpeed(0.1f), _cPacmanFrameTime(250), _cMunchieFrameTime(500)
{
	_pacman = new Player();
	_pacman->dead = false;
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
	delete _pacmanTexture;	delete _pacmanSourceRect;	delete _pacmanPosition;
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
	_pacmanTexture = new Texture2D();	_pacmanTexture->Load("Textures/Pacman.tga", false);		_pacmanPosition = new Vector2(350.0f, 350.0f);	_pacmanSourceRect = new Rect(0.0f, 0.0f, 32, 32);
	// Initialize game state, score, and high score
	_isGameOver = false;
	_score = 50;	_highScore = 0;	LoadHighScore();
	// Load Cherry Inverted
	_cherryInvertedTexture = new Texture2D();	_cherryInvertedTexture->Load("Textures/CherryInverted.png", false);	_cherryInvertedPosition = new Vector2((rand() % Graphics::GetViewportWidth()), (rand() % Graphics::GetViewportHeight()));
	_cherryDirection = 0; // Start the cherry moving to the right
	_cherrySpeed = 0.1f;   
	_cherryTexture = new Texture2D();	_cherryTexture->Load("Textures/Cherry.png", false);
	//_cherryPosition = new Vector2((rand() % Graphics::GetViewportWidth()), (rand() % Graphics::GetViewportHeight()));
	_cherryPosition = _cherryInvertedPosition;
	// Set the duration (in milliseconds) for displaying the "Start Game" message
	_startMessageDisplayTime = 3000; // 3 seconds
	_showStartMessage = true;
	//Load Ghost
	_ghosts[0]->texture = new Texture2D();	_ghosts[0]->texture->Load("Textures/GhostBlue.png", false);	_ghosts[0]->position = new Vector2((rand() % Graphics::GetViewportWidth()), (rand() % Graphics::GetViewportHeight()));
	_ghosts[0]->sourceRect = new Rect(_ghosts[0]->position->X, _ghosts[0]->position->Y, 20, 20);
	_startSound = new S2D::SoundEffect();
	_startSound->Load("sound/pop.wav");
	// Load Munchie
	_munchieBlueTexture = new Texture2D();	_munchieBlueTexture->Load("Textures/Munchie.tga", true);
	Texture2D* munchieTex = new Texture2D();
	munchieTex->Load("Textures/Munchie.png", false);
	int i;
	for (i = 0; i < MUNCHIECOUNT; i++)
	{
		_munchies[i]->texture = munchieTex;
		_munchies[i]->sourceRect = new Rect((rand() % Graphics::GetViewportWidth()), (rand() % Graphics::GetViewportHeight()), 12, 12);
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
	if (_showStartMessage) // Play sound only on the first frame when the message appears
	{
		return;
	}
	if (!_isGameOver) // Only update the game if it's not over
	{
		UpdateCherryCollision();

		// Update cherry position
		//_cherryPosition->X += _cherrySpeed * elapsedTime; // Modify the value (0.1f) as needed to control cherry's movement speed			not needed

		// Wrap the cherry around the screen if it goes off the edges
		if (_cherryPosition->X > Graphics::GetViewportWidth())
		{
			_cherryPosition->X = 0;
			_cherryPosition->Y = rand() % Graphics::GetViewportHeight();
		}

		// Wrap the cherryInverted around the screen if it goes off the edges
		if (_cherryInvertedPosition->X < 0)
		{
			_cherryInvertedPosition->X = Graphics::GetViewportWidth();
			_cherryInvertedPosition->Y = rand() % Graphics::GetViewportHeight();
		}

		// Wrap the cherry around the screen if it goes off the edges
		if (_cherryPosition->X > Graphics::GetViewportWidth())
		{
			_cherryPosition->X = 0;
			_cherryPosition->Y = rand() % Graphics::GetViewportHeight();
		}

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

			// Checks if key is pressed
			if (keyboardState->IsKeyDown(Input::Keys::RIGHT))
			{
				pacmanDirection = Right;
			}
			if (keyboardState->IsKeyDown(Input::Keys::UP))
			{
				pacmanDirection = Up;
			}
			if (keyboardState->IsKeyDown(Input::Keys::LEFT))
			{
				pacmanDirection = Left;
			}
			if (keyboardState->IsKeyDown(Input::Keys::DOWN))
			{
				pacmanDirection = Down;
			}

			if (pacmanDirection == Right) {
				_pacmanPosition->X += _cPacmanSpeed * elapsedTime;
			}
			if (pacmanDirection == Up) {
				_pacmanPosition->Y -= _cPacmanSpeed * elapsedTime;
			}
			if (pacmanDirection == Left) {
				_pacmanPosition->X -= _cPacmanSpeed * elapsedTime;
			}
			if (pacmanDirection == Down) {
				_pacmanPosition->Y += _cPacmanSpeed * elapsedTime;
			}

			_pacmanSourceRect->Y = _pacmanSourceRect->Height * pacmanDirection;
			// Checks if Pacman is trying to disappear
			if (_pacmanPosition->X > Graphics::GetViewportWidth() - 0.5 * _pacmanSourceRect->Width)
			{
				_pacmanPosition->X = 0 - 0.5 * _pacmanSourceRect->Width;
			}
			if (_pacmanPosition->X < 0 - 0.5 * _pacmanSourceRect->Width)
			{
				_pacmanPosition->X = Graphics::GetViewportWidth() - 0.5 * _pacmanSourceRect->Width;
			}
			if (_pacmanPosition->Y > Graphics::GetViewportHeight() - 0.5 * _pacmanSourceRect->Width)
			{
				_pacmanPosition->Y = 0 - 0.5 * _pacmanSourceRect->Height;
			}
			if (_pacmanPosition->Y < 0 - 0.5 * _pacmanSourceRect->Height)
			{
				_pacmanPosition->Y = Graphics::GetViewportHeight() - 0.5 * _pacmanSourceRect->Height;
			}
			if (_pacmanCurrentFrameTime > _cPacmanFrameTime)
			{
				_pacmanFrame++;
				if (_pacmanFrame >= 2)
					_pacmanFrame = 0;

				_pacmanCurrentFrameTime = 0;

				_pacmanSourceRect->X = _pacmanSourceRect->Width * _pacmanFrame;
			}

			// Check for collision with screen walls
			if (_pacmanPosition->X < 0 || _pacmanPosition->X > Graphics::GetViewportWidth() ||
				_pacmanPosition->Y < 0 || _pacmanPosition->Y > Graphics::GetViewportHeight())
			{
				_isGameOver = true;
			}

		}
	}
	else // Game is over
	{
		// Check for restart input
		if (keyboardState->IsKeyDown(Input::Keys::R))
		{
			// Restart the game
			_isGameOver = false;
			_score = 0;
			_pacmanPosition = new Vector2(350.0f, 350.0f); // Reset Pacman's position
			_cherryPosition = new Vector2((rand() % Graphics::GetViewportWidth()), (rand() % Graphics::GetViewportHeight())); // Reset cherry's position
			_frameCount = 0;
			_pacmanFrame = 0;
			_pacmanCurrentFrameTime = 0;
			//now_pacmanDirection = 0;
			_paused = false;
			_showStartMessage = true;
			_startMessageDisplayTime = 3000;
			LoadHighScore(); // Load high score from file
		}
	}
}

void Pacman::Draw(int elapsedTime)
{
	std::stringstream stream;
	stream << "Pacman X: " << _pacmanPosition->X << "\nY: " << _pacmanPosition->Y;

	SpriteBatch::BeginDraw(); // Starts Drawing

	if (_isGameOver) { DrawEndGame(); }
	else
	{
		// Show "Start Game" message
		if (_showStartMessage)
		{
			std::stringstream startMessageStream;
			startMessageStream << "Start Game!";

			// You can customize the position and color of the message as needed
			Vector2 startMessagePosition(Graphics::GetViewportWidth() / 2.0f, Graphics::GetViewportHeight() / 2.0f);
			SpriteBatch::DrawString(startMessageStream.str().c_str(), &startMessagePosition, Color::Green);

			// Check if it's time to hide the message
			_startMessageDisplayTime -= elapsedTime;
			if (_startMessageDisplayTime <= 0)
				_showStartMessage = false;

		}
		else // "Start Game" message is not visible
		{
			// Draws Pacman and other game elements
			SpriteBatch::Draw(_pacmanTexture, _pacmanPosition, _pacmanSourceRect);
			// Show munchie
			for (int i = 0; i < MUNCHIECOUNT; i++)
			{
				if (_frameCount < 30)
					SpriteBatch::Draw(_munchieInvertedTexture, _munchies[i]->sourceRect, nullptr, Vector2::Zero, 1.0f, 0.0f, Color::White, SpriteEffect::NONE);				// Draw Red Munchie
				else
					SpriteBatch::Draw(_munchieBlueTexture, _munchies[i]->sourceRect, nullptr, Vector2::Zero, 1.0f, 0.0f, Color::White, SpriteEffect::NONE);				// Draw Blue Munchie
			}
			if (_frameCount < 30) {
				SpriteBatch::Draw(_cherryInvertedTexture, _cherryInvertedPosition);
			}
			else
			{
				SpriteBatch::Draw(_cherryTexture, _cherryPosition);

				if (_frameCount >= 60)
					_frameCount = 0;
			}
			//_ghosts[0]->texture->Load("Textures/GhostBlue.png", false);
			SpriteBatch::Draw(_ghosts[0]->texture, _ghosts[0]->sourceRect);//, nullptr, Vector2::Zero, 1.0f, 0.0f, Color::White, SpriteEffect::NONE);				// Draw ghost
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
		Vector2 scorePosition(300.0f, 30.0f); // You can adjust the position of the score on the screen
		SpriteBatch::DrawString(scoreStream.str().c_str(), &scorePosition, Color::White);
	}

	SpriteBatch::EndDraw(); // Ends Drawing
}

//void Pacman::UpdateMunchies(Collectable*, int elapsedTime) {}

void Pacman::UpdateGhost(MovingEnemy* ghost, int elapsedTime)
{
	if (ghost->direction == 0) //Moves Right
	{
		ghost->position->X += ghost->speed * elapsedTime;
	}
	else if (ghost->direction == 1) //Moves Left
	{
		ghost->position->X -= ghost->speed * elapsedTime;
	}
	if (ghost->position->X + ghost->sourceRect->Width >=
		Graphics::GetViewportWidth()) //Hits Right edge
	{
		ghost->direction = 1; //Change direction
	}
	else if (ghost->position->X <= 0) //Hits left edge
	{
		ghost->direction = 0; //Change direction
	}
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
	endGameStream << "Your Score: " << 50 << "\n";
	endGameStream << "High Score: " << 120 << "\n";
	endGameStream << "Press R to restart";

	// You can customize the position and color of the message as needed
	Vector2 endGamePosition(Graphics::GetViewportWidth() / 2.0f, Graphics::GetViewportHeight() / 2.0f);
	SpriteBatch::DrawString(endGameStream.str().c_str(), &endGamePosition, Color::Red);

	// Check for restart input
	Input::KeyboardState* keyboardState = Input::Keyboard::GetState();
	if (keyboardState->IsKeyDown(Input::Keys::R))
	{
		// Restart the game
		_isGameOver = false;
		_score = 0;
		_pacmanPosition = new Vector2(350.0f, 350.0f); // Reset Pacman's position
		_cherryPosition = new Vector2((rand() % Graphics::GetViewportWidth()), (rand() % Graphics::GetViewportHeight())); // Reset cherry's position
		_frameCount = 0;
		_pacmanFrame = 0;
		_pacmanCurrentFrameTime = 0;
		//now_pacmanDirection = 0;
		_paused = false;
		_showStartMessage = true;
		_startMessageDisplayTime = 3000;
		LoadHighScore(); // Load high score from file

	}
}

void Pacman::UpdateScore()
{
	// Increment the score
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
		file << "highscore=" << _highScore;
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
	// Local Variables
	//int i = 0;
	//int bottom1 = _pacmanPosition->Y + 32;
	//int bottom2 = 0;
	//int left1 = _pacmanPosition->X;
	//int left2 = 0;
	//int right1 = _pacmanPosition->X + 32;
	//int right2 = 0;
	//int top1 = _pacmanPosition->Y;
	//int top2 = 0;

	//bottom2 = _cherryPosition->Y + _cherryTexture->GetHeight();
	//left2 = _cherryPosition->X;
	//right2 =
	//	_cherryPosition->X + _cherryTexture->GetWidth();
	//top2 = _cherryPosition->Y;
	//if (_isCherryVisible && (bottom1 > top2) && (top1 < bottom2) && (right1 > left2)
	//	&& (left1 < right2))
	//{
	//	cout << "Collision\n";
	//	//_pacman->dead = true;
	//	//i = GHOSTCOUNT;
	//}

	//_pacmanSourceRect->Width;


	// Check for collision between Pacman and the cherry
	if (_pacmanPosition->X + _pacmanSourceRect->Width > _cherryPosition->X &&
		_pacmanPosition->X < _cherryPosition->X + _cherryTexture->GetWidth() &&
		_pacmanPosition->Y + _pacmanSourceRect->Height > _cherryPosition->Y &&
		_pacmanPosition->Y < _cherryPosition->Y + _cherryTexture->GetHeight())
	{
		// Pacman collided with the cherry
		UpdateScore();
		//_isCherryVisible = false; // Make the cherry invisible

		// Respawn the cherry at a random position within the visible area
		_cherryPosition->X = rand() % (Graphics::GetViewportWidth() - _cherryTexture->GetWidth());
		_cherryPosition->Y = rand() % (Graphics::GetViewportHeight() - _cherryTexture->GetHeight());
		_cherryInvertedPosition->X = _cherryPosition->X;
		_cherryInvertedPosition->Y = _cherryPosition->Y;
		// Set the cherry visibility flag to true
		//_isCherryVisible = true;
	}

	//if (_pacmanPosition->X + _pacmanSourceRect->Width > _cherryInvertedPosition->X &&
	// Check for collision between Pacman and the inverted cherry
	//	_pacmanPosition->X < _cherryInvertedPosition->X + _cherryInvertedTexture->GetWidth() &&
	//	_pacmanPosition->Y + _pacmanSourceRect->Height > _cherryInvertedPosition->Y &&
	//	_pacmanPosition->Y < _cherryInvertedPosition->Y + _cherryInvertedTexture->GetHeight())
	//{
	//	// Pacman collided with the inverted cherry
	//	UpdateScore();
	//	_isCherryVisible = false; // Make the inverted cherry invisible

	//	// Respawn the inverted cherry at a random position within the visible area
	//	_cherryInvertedPosition->X = rand() % (Graphics::GetViewportWidth() - _cherryInvertedTexture->GetWidth());
	//	_cherryInvertedPosition->Y = rand() % (Graphics::GetViewportHeight() - _cherryInvertedTexture->GetHeight());

	//	// Set the cherry visibility flag to true
	//	_isCherryVisible = true;
	//}
}
