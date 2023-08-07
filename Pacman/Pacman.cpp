#include "Pacman.h"
#include <time.h>
#include <sstream>
#include <fstream> // Include the header file for file stream operations (ifstream and ofstream)
#include <iostream>
#include <iomanip>
int PLAYABLE_HEIGHT;
int PLAYABLE_WIDTH;
Pacman::Pacman(int argc, char* argv[]) : Game(argc, argv), _cPacmanSpeed(0.1f), _cPacmanFrameTime(250)
{
	Pacman_Player = new Player();		_ghosts[0] = new MovingEnemy();		Cherry = new Collectable();
	_frameCount = 0;

	_paused = false;
	srand(time(NULL));
	for (int i = 0; i < MUNCHIECOUNT; i++)
	{
		_munchies[i] = new Collectable();
		_munchies[i]->frameCount = rand() % 1;
		_munchies[i]->currentFrameTime = 0;
		_munchies[i]->frameTime = rand() % 500 + 50;
	}
	Graphics::Initialise(argc, argv, this, 1024, 768, false, 25, 25, "Pacman", 60); 	//Initialise important Game aspects
	Input::Initialise();
	Graphics::StartGameLoop();	// Start the Game Loop - This calls Update and Draw in game loop 
}
Pacman::~Pacman()
{
	delete Pacman_Player->texture;	delete Pacman_Player->sourceRect;	delete Pacman_Player->position;
	delete Cherry->texture;	delete Cherry->sourceRect;	delete Cherry->position;
	delete _munchies[0]->texture;	delete _munchies[1]->texture;
	for (int nCount = 0; nCount < MUNCHIECOUNT; nCount++)
	{
		delete _munchies[nCount]->position;
		delete _munchies[nCount]->texture;
		delete _munchies[nCount]->sourceRect;
		delete _munchies[nCount];
	}
	delete[] _munchies;	//Order of operations is important, array deleted last
}
int generate_PlayableArea_Height() {
	return  ((rand() % PLAYABLE_HEIGHT) + 25);	//leave 25 from top and 5 from bottom
}
void Pacman::LoadContent()
{
	PLAYABLE_HEIGHT = (((Graphics::GetViewportHeight() - 30)) - 25) + 1;		PLAYABLE_WIDTH = (Graphics::GetViewportWidth() - 15) - 3 + 1;
	Pacman_Player->Initialise_Player();		_ghosts[0]->Initialise_Ghost();//Player & Ghost all initial settings	
	_score = 0;	_highScore = 0;	LoadHighScore();		// Initialize game state, score, and high score

	_cherryInvertedTexture = new Texture2D();	_cherryInvertedTexture->Load("Textures/CherryInverted.png", false);		// Load Cherry Inverted
	Cherry->texture = new Texture2D();	Cherry->texture->Load("Textures/Cherry.png", false);
	Cherry->position = new Vector2((rand() % PLAYABLE_WIDTH) + 3, (rand() % PLAYABLE_HEIGHT) + 25);

	_startMessageDisplayTime = 3000; /*3 seconds*/		_showStartMessage = true;

	_startSound = new S2D::SoundEffect();	_startSound->Load("sound/pop.wav");

	_munchies[0]->texture = new Texture2D();	_munchies[0]->texture->Load("Textures/Munchie.tga", true);// Load Munchies		keeping Muchie texture on 0 and MunchieInverted on 1, using only these in Draw()
	_munchies[1]->texture = new Texture2D();	_munchies[1]->texture->Load("Textures/MunchieInverted.tga", true);// Load Munchies
	for (int i = 0; i < MUNCHIECOUNT; i++) {
		_munchies[i]->sourceRect = new Rect(((rand() % PLAYABLE_WIDTH) + 3), generate_PlayableArea_Height(), 12, 12);
		_munchies[i]->position = new Vector2(_munchies[i]->sourceRect->X, _munchies[i]->sourceRect->Y);
	}

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
	Pacman_Player->currentFrameTime += elapsedTime;
	Input::KeyboardState* keyboardState = Input::Keyboard::GetState();		// Gets the current state of the keyboard

	if (_showStartMessage && _startMessageDisplayTime == 3000) // Play sound only on the first frame when the message appears
		Audio::Play(_startSound);

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
			if (Pacman_Player->speedMultiplier == 1)
			{
				_cPacmanSpeed = 0.3f;
				Power_Time++;
				if (Power_Time >= 200) {
					Pacman_Player->speedMultiplier = 0;
					Power_Time = 0;
				}
			}
			else
				_cPacmanSpeed = 0.1f;
			_frameCount++;
			UpdateGhost(_ghosts[0], elapsedTime);
			CheckGhostCollisions();
			// Checks if key is pressed
			if (keyboardState->IsKeyDown(Input::Keys::RIGHT))
				pacmanDirection = direction::Right;
			if (keyboardState->IsKeyDown(Input::Keys::UP))
				pacmanDirection = direction::Up;
			if (keyboardState->IsKeyDown(Input::Keys::LEFT))
				pacmanDirection = direction::Left;
			if (keyboardState->IsKeyDown(Input::Keys::DOWN))
				pacmanDirection = direction::Down;

			if (pacmanDirection == direction::Right)
				Pacman_Player->position->X += _cPacmanSpeed * elapsedTime;
			else if (pacmanDirection == direction::Up)
				Pacman_Player->position->Y -= _cPacmanSpeed * elapsedTime;
			else if (pacmanDirection == direction::Left)
				Pacman_Player->position->X -= _cPacmanSpeed * elapsedTime;
			else if (pacmanDirection == direction::Down)
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

			if (Pacman_Player->currentFrameTime > _cPacmanFrameTime)
			{
				Pacman_Player->frame++;
				if (Pacman_Player->frame >= 2)
					Pacman_Player->frame = 0;

				Pacman_Player->currentFrameTime = 0;

				Pacman_Player->sourceRect->X = Pacman_Player->sourceRect->Width * Pacman_Player->frame;
			}

			// Check for collision with screen walls
			if (Pacman_Player->position->X < 0 || Pacman_Player->position->X > Graphics::GetViewportWidth() || Pacman_Player->position->Y < 0 || Pacman_Player->position->Y > Graphics::GetViewportHeight())
				Pacman_Player->dead = true;		//Player Collided with wall

			for (int i = 0; i < MUNCHIECOUNT; i++)
				if (Munchies_Collision(_munchies[i]))
					UpdateMunchies(_munchies[i]);	//Munchie is eaten
		}
	}
	else // Game is OVER!
	{
		// Check for restart input
		if (keyboardState->IsKeyDown(Input::Keys::R))
		{
			// Restart the game
			Pacman_Player->Initialise_Player();	// Reset Pacman Player
			Cherry->position->X = (rand() % Graphics::GetViewportWidth()); Cherry->position->Y = generate_PlayableArea_Height(); // Reset cherry's position
			_frameCount = 0;
			_paused = false;
			_showStartMessage = true;			_startMessageDisplayTime = 3000;
			_score = 0;			LoadHighScore(); // Load high score from file
		}
	}
}
void Pacman::Draw(int elapsedTime)
{
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
			Vector2 startMessagePosition((Graphics::GetViewportWidth() - 5) / 2.0f, Graphics::GetViewportHeight() / 2.0f);
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
			for (int i = 0; i < MUNCHIECOUNT; i++)			// Show munchie
			{
				if (_munchies[i]->position->X == -1)//Already eaten munchie
					continue;
				if (_frameCount < 30)
					SpriteBatch::Draw(_munchies[1]->texture, _munchies[i]->sourceRect, nullptr, Vector2::Zero, 1.0f, 0.0f, Color::White, SpriteEffect::NONE);	// Draw Red Munchie
				else
					SpriteBatch::Draw(_munchies[0]->texture, _munchies[i]->sourceRect, nullptr, Vector2::Zero, 1.0f, 0.0f, Color::White, SpriteEffect::NONE);		// Draw Blue Munchie
			}
			if (_frameCount < 30)
				SpriteBatch::Draw(_cherryInvertedTexture, Cherry->position);

			else
			{
				SpriteBatch::Draw(Cherry->texture, Cherry->position);

				if (_frameCount >= 60)
					_frameCount = 0;
			}
			SpriteBatch::Draw(_ghosts[0]->texture, _ghosts[0]->position, _ghosts[0]->sourceRect);
			// Draws String
			std::stringstream stream;	stream << "Pacman X: " << std::fixed << std::setprecision(1) << Pacman_Player->position->X << "\tY: " << std::fixed << std::setprecision(1) << Pacman_Player->position->Y << std::endl; //retain only 1st decimal
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
	int bottom1 = Pacman_Player->position->Y + Pacman_Player->sourceRect->Height;
	int bottom2 = 0;
	int left1 = Pacman_Player->position->X;
	int left2 = 0;
	int right1 = Pacman_Player->position->X + Pacman_Player->sourceRect->Width;
	int right2 = 0;
	int top1 = Pacman_Player->position->Y;
	int top2 = 0;
	for (int i = 0; i < GHOSTCOUNT; i++)
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
	Munchie->position->X = -1;		Munchie->position->Y = -1;
	Munchie->sourceRect->X = -1;	Munchie->sourceRect->Y = -1;
}
bool Pacman::Munchies_Collision(Collectable* Munchie) {
	if (Pacman_Player->position->X + Pacman_Player->sourceRect->Width > Munchie->position->X &&
		Pacman_Player->position->X < Munchie->position->X + _munchies[0]->texture->GetWidth() &&
		Pacman_Player->position->Y + Pacman_Player->sourceRect->Height > Munchie->position->Y &&
		Pacman_Player->position->Y < Munchie->position->Y + _munchies[0]->texture->GetHeight())

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
}
void Pacman::DrawEndGame()
{
	if (_score >= _highScore)	// Check and update high score if necessary
	{
		_highScore = _score;
		SaveHighScore(); // Save the new high score to the file
	}
	std::stringstream endGameStream; // Draw the "End Game" screen with score and high score table
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
		Pacman_Player->Initialise_Player();	// Reset Pacman's Player
		Cherry->position->X = (rand() % Graphics::GetViewportWidth()); Cherry->position->Y = generate_PlayableArea_Height(); // Reset cherry's position
		_frameCount = 0;
		_paused = false; 
		_showStartMessage = true;		_startMessageDisplayTime = 3000;
		_score = 0;		LoadHighScore(); //Reset Level Score & Load high score from file
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
			std::cout << "Error: Failed to write the high score." << std::endl;
	}
	else
		std::cout << "Error: Unable to open the highscore.txt file for writing." << std::endl;
}
void Pacman::UpdateCherryCollision()
{
	// Check for collision between Pacman and the cherry
	if (Pacman_Player->position->X + Pacman_Player->sourceRect->Width > Cherry->position->X &&
		Pacman_Player->position->X < Cherry->position->X + Cherry->texture->GetWidth() &&
		Pacman_Player->position->Y + Pacman_Player->sourceRect->Height > Cherry->position->Y &&
		Pacman_Player->position->Y < Cherry->position->Y + Cherry->texture->GetHeight())
	{
		// Pacman collided with the cherry
		UpdateScore();
		// Respawn the cherry at a random position within the visible area
		Cherry->position->X = rand() % (Graphics::GetViewportWidth() - Cherry->texture->GetWidth());
		Cherry->position->Y = generate_PlayableArea_Height();
		Pacman_Player->speedMultiplier = 1;		Pacman_Player->frame = _frameCount;
		Power_Time = 0;
	}
}
