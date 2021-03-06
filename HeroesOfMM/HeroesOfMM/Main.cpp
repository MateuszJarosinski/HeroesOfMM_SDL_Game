#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_ttf.h"
#include <Windows.h>

typedef unsigned char uchar;

const char fontPath[] = "OdibeeSans-Regular.ttf";
const int fontSize = 42;

const int screenWidth = 1920;
const int screenHeight = 1080;

const int gridHeight = 11;
const int gridWidth = 15;

const int gridElementPixelHeight = 98;
const int gridElementPixelWidth = 128;

//the board is 13/17 because I need space for the sides of the map (I need them later)
//in practice the player plays on 11/15 map
uchar battlefield[13][17];

SDL_Texture* SetTexture(SDL_Surface* surface, SDL_Renderer* renderer, const char* imagePath);

struct Vector2i
{
	int x;
	int y;
};

Vector2i MouseToGridPos(Vector2i mousePos)
{
	Vector2i gridPos = { mousePos.x / gridElementPixelWidth ,  mousePos.y / gridElementPixelHeight };
	return gridPos;
}

char* CastToArray(int number)
{
    int n = log10(number) + 1;
    int i;
    char* numberArray = (char*)calloc(n, sizeof(char));
    numberArray[n] = '\0';
    for (i = n - 1; i >= 0; --i, number /= 10)
    {
        numberArray[i] = (number % 10) + '0';
    }

    return numberArray;
}

void GrassfireAlgorithm()
{
	bool S = true;

	while (S)
	{
		S = false;
		for (int i = 0; i < 13; i++)
		{
			for (int j = 0; j < 17; j++)
			{
				uchar A = battlefield[i][j];

				if (A != 0 && A != 255)
				{
					uchar B = A + 1;
					uchar gridDown = battlefield[i + 1][j];
					uchar gridUp = battlefield[i - 1][j];
					uchar gridRight = battlefield[i][j + 1];
					uchar gridLeft = battlefield[i][j - 1];

					if (i < 11 && (gridDown != 255 && gridDown < B))
					{
						if (battlefield[i + 1][j] == 0)
						{
							battlefield[i + 1][j] = B;
							S = true;
						};
					}
					if (i > 0 && (gridUp != 255 && gridUp < B))
					{
						if (battlefield[i - 1][j] == 0)
						{
							battlefield[i - 1][j] = B;
							S = true;
						};
					}
					if (j < 15 && (gridRight != 255 && gridRight < B))
					{
						if (battlefield[i][j + 1] == 0)
						{
							battlefield[i][j + 1] = B;
							S = true;
						};
					}
					if (j > 0 && (gridLeft != 255 && gridLeft < B))
					{
						if (battlefield[i][j - 1] == 0)
						{
							battlefield[i][j - 1] = B;
							S = true;
						};
					}
				}
			}
		}
	}
}

void SetArraySides()
{
	for (int i = 0; i < 15; i++)
	{
		battlefield[0][i] = 255;
	}
	for (int i = 0; i < 11; i++)
	{
		battlefield[i][0] = 255;
	}

	for (int i = 0; i < 15; i++)
	{
		battlefield[12][i] = 255;
	}
	for (int i = 0; i < 11; i++)
	{
		battlefield[i][16] = 255;
	}
}

void SetAllGridElementsToZero()
{
	for (int i = 0; i < 13; i++)
	{
		for (int j = 0; j < 17; j++)
		{
			if (battlefield[i][j] != 255)
			{
				battlefield[i][j] = 0;
			}
		}
	}
}

void PrintArray()
{
	for (int rows = 0; rows < 13; rows++)
	{
		for (int columns = 0; columns < 17; columns++)
		{
			printf("%i\t  ", battlefield[rows][columns]);
		}
		printf("\n");
	}
	printf("\n");
}

struct Character
{
	bool isAlive = true;
	bool imageDisabled = false;
	bool updateHealth = false;

	int quantity = 0;
	int health = 0;
	int attackDamage = 0;

	SDL_Texture* texture;
	SDL_Texture* textTexture;
	SDL_Rect rect;
	SDL_Rect textRect;

	Vector2i position;
	Vector2i currentGrid;
	Vector2i destinationGrid;
	Vector2i pastGrid = {0,0};

	Character(Vector2i pos, SDL_Surface* sur, SDL_Renderer* rend, SDL_Surface* textSurface, const char* imagePath, TTF_Font* font, int quan, int hp, int attack);
	void PlaceCharacter(Vector2i pos);
	void Move(Vector2i dest);
	void Attack();
	void UpdateHealth(SDL_Renderer* rend, SDL_Surface* textSur, TTF_Font* font);
	void Attack(Character* enemy);
};

void Character::Attack(Character* enemy)
{
	int totalEnemyHealth = enemy->health * enemy->quantity;
	int totalHealth = health * quantity;

	int totalDamage = attackDamage * quantity;

	int newEnemyHealth = totalEnemyHealth -= totalDamage;

	if (newEnemyHealth > 0)
	{
		int enemyQuantityDead = totalDamage / enemy->health;
		enemy->quantity = enemy->quantity - enemyQuantityDead;
		enemy->updateHealth = true;

		int totalEnemyDamage = enemy->attackDamage * enemy->quantity;
		int newHealth = totalHealth -= totalEnemyDamage;
		if (newHealth > 0)
		{
			int quantityDead = totalEnemyDamage / health;
			quantity -= quantityDead;
			updateHealth = true;
		}
		else
		{
			isAlive = false;
			imageDisabled = true;
			battlefield[currentGrid.x][currentGrid.y] = 100;
		}
	}
	else
	{
		enemy->isAlive = false;
		enemy->imageDisabled = true;
		battlefield[enemy->currentGrid.x][enemy->currentGrid.y] = 100;
	}
}

void Character::UpdateHealth(SDL_Renderer* rend, SDL_Surface* textSur, TTF_Font* font)
{
	const char* text_ = CastToArray(quantity);

	if (isAlive)
	{
		textSur = TTF_RenderText_Solid(font, text_, { 255, 255, 255 });

		textTexture = SDL_CreateTextureFromSurface(rend, textSur);
	}

	SDL_FreeSurface(textSur);
}

void Character::PlaceCharacter(Vector2i pos)
{
	position.x = (pos.x - 1) * gridElementPixelWidth + gridElementPixelWidth / 2;
	position.y = (pos.y - 1) * gridElementPixelHeight + gridElementPixelHeight / 2;
}

Character::Character(Vector2i pos, SDL_Surface* sur, SDL_Renderer* rend, SDL_Surface* textSur, const char* imagePath, TTF_Font* font, int quan, int hp, int attack)
{
	quantity = quan;
	health = hp;
	attackDamage = attack;

	PlaceCharacter(pos);
	currentGrid = MouseToGridPos(position);
	currentGrid.x += 1; currentGrid.y += 1;
	battlefield[MouseToGridPos(position).y + 1][MouseToGridPos(position).x + 1] = 255;
	texture = SetTexture(sur, rend, imagePath);
	textSur = TTF_RenderText_Solid(font, CastToArray(quantity), { 255, 255, 255 });
	textTexture = SDL_CreateTextureFromSurface(rend, textSur);
}

void Character::Move(Vector2i dest)
{
	destinationGrid = dest;
	currentGrid = { position.x / gridElementPixelWidth ,position.y / gridElementPixelHeight };
	destinationGrid.x += 1;
	destinationGrid.y += 1;
	currentGrid.x += 1;
	currentGrid.y += 1;

	if (battlefield[destinationGrid.y][destinationGrid.x] != 255 && battlefield[destinationGrid.y][destinationGrid.x] != 200)
	{
		battlefield[destinationGrid.y][destinationGrid.x] = 1;
	}

	uchar destination = battlefield[destinationGrid.y][destinationGrid.x];
	uchar playerPos = battlefield[currentGrid.y][currentGrid.x];
	uchar down = battlefield[currentGrid.y + 1][currentGrid.x];
	uchar up = battlefield[currentGrid.y - 1][currentGrid.x];
	uchar right = battlefield[currentGrid.y][currentGrid.x + 1];
	uchar left = battlefield[currentGrid.y][currentGrid.x - 1];

	GrassfireAlgorithm();

	if (destination != 255 && destination != 200)
	{
		
		if (playerPos > up)
		{
			if (position.y - gridElementPixelHeight >= gridElementPixelHeight / 2)
			{
				position.y -= gridElementPixelHeight;
				pastGrid.y = currentGrid.y;
				currentGrid.y -= 1;
				battlefield[pastGrid.y][currentGrid.x] = 200;
			}
			Sleep(150);
		}
		else if (playerPos > down)
		{
			if (position.y + gridElementPixelHeight <= (screenHeight - gridElementPixelHeight / 2))
			{
				position.y += gridElementPixelHeight;
				pastGrid.y = currentGrid.y;
				currentGrid.y += 1;
				battlefield[pastGrid.y][currentGrid.x] = 200;
			}
			Sleep(150);
		}
		else if (playerPos > left)
		{
			if (position.x - gridElementPixelWidth >= gridElementPixelWidth / 2)
			{
				position.x -= gridElementPixelWidth;
				pastGrid.x = currentGrid.x;
				currentGrid.x -= 1;
				battlefield[currentGrid.y][pastGrid.x] = 200;
			}
			Sleep(150);
		}
		else if (playerPos > right)
		{
			if (position.x + gridElementPixelWidth <= (screenWidth - gridElementPixelWidth / 2))
			{
				position.x += gridElementPixelWidth;
				pastGrid.x = currentGrid.x;
				currentGrid.x += 1;
				battlefield[currentGrid.y][pastGrid.x] = 200;
			}
			Sleep(150);
		}
	}
}

struct Obstacle
{
	SDL_Texture* texture;
	SDL_Rect rectObstacle;

	Vector2i position;

	Obstacle(Vector2i pos, SDL_Surface* surface, SDL_Renderer* renderer, const char* imagePath);
	void PlaceObstacle();
};

Obstacle::Obstacle(Vector2i pos, SDL_Surface* surface, SDL_Renderer* renderer, const char* imagePath)
{
	position = pos;
	PlaceObstacle();
	battlefield[MouseToGridPos(position).y + 1][MouseToGridPos(position).x + 1] = 255;
	texture = SetTexture(surface, renderer, imagePath);
}

void Obstacle::PlaceObstacle()
{
	position.x = (position.x - 1) * gridElementPixelWidth + gridElementPixelWidth / 2;
	position.y = (position.y - 1) * gridElementPixelHeight + gridElementPixelHeight / 2;
}

uint32_t DeltaTime(uint32_t* lastTickTime, uint32_t* tickTime)
{
	*tickTime = SDL_GetTicks();
	uint32_t deltaTime = *tickTime - *lastTickTime;
	*lastTickTime = *tickTime;
	return deltaTime;
}

TTF_Font* GetFont()
{
	if (TTF_Init() < 0)
		abort();
	TTF_Font* font = TTF_OpenFont(fontPath, fontSize);
	if (font)
		return font;
	else
		abort();
}

SDL_Texture* SetTexture(SDL_Surface* surface, SDL_Renderer* renderer, const char* imagePath)
{
	surface = IMG_Load(imagePath);
	if (!surface)
	{
		printf("Unable to load an image %s. Error: %s", imagePath, IMG_GetError());
		return NULL;
	}

	// Now we use the renderer and the surface to create a texture which we later can draw on the screen.
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	if (!texture)
	{
		printf("Unable to create a texture. Error: %s", SDL_GetError());
		return NULL;
	}
	SDL_FreeSurface(surface);

	return texture;
}

void SetRect(SDL_Rect* rect, Vector2i position)
{
	rect->x = (int)round(position.x - gridElementPixelWidth / 2); // Counting from the image's center but that's up to you
	rect->y = (int)round(position.y - gridElementPixelHeight / 2); // Counting from the image's center but that's up to you
	rect->w = (int)gridElementPixelWidth;
	rect->h = (int)gridElementPixelHeight;
}

void SetTextRect(SDL_Rect* rect, Vector2i position)
{
	rect->x = (int)round(position.x - gridElementPixelWidth / 2); // Counting from the image's center but that's up to you
	rect->y = (int)round(position.y - gridElementPixelHeight / 2); // Counting from the image's center but that's up to you
	rect->w = fontSize;
	rect->h = fontSize;
}

void DrawImage(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect rect)
{
	SDL_RenderCopyEx(renderer, // Already know what is that
		texture, // The image
		nullptr, // A rectangle to crop from the original image. Since we need the whole image that can be left empty (nullptr)
		&rect, // The destination rectangle on the screen.
		0, // An angle in degrees for rotation
		nullptr, // The center of the rotation (when nullptr, the rect center is taken)
		SDL_FLIP_NONE); // We don't want to flip the image
}

bool InitSDL(SDL_Renderer** renderer, SDL_Window** window)
{
	// Init SDL libraries
	SDL_SetMainReady(); // Just leave it be
	int result = 0;
	result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO); // Init of the main SDL library
	if (result) // SDL_Init returns 0 (false) when everything is OK
	{
		printf("Can't initialize SDL. Error: %s", SDL_GetError()); // SDL_GetError() returns a string (as const char*) which explains what went wrong with the last operation
		return false;
	}

	result = IMG_Init(IMG_INIT_PNG); // Init of the Image SDL library. We only need to support PNG for this project
	if (!(result & IMG_INIT_PNG)) // Checking if the PNG decoder has started successfully
	{
		printf("Can't initialize SDL image. Error: %s", SDL_GetError());
		return false;
	}

	// Creating the window 1920x1080 (could be any other size)
	*window = SDL_CreateWindow("HeroesOfMM",
		0, 0,
		screenWidth, screenHeight,
		SDL_WINDOW_SHOWN);

	if (!*window)
		return false;

	// Creating a renderer which will draw things on the screen
	*renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
	if (!*renderer)
		return false;

	// Setting the color of an empty window (RGBA). You are free to adjust it.
	SDL_SetRenderDrawColor(*renderer, 153, 255, 102, 255);

	return true;
}

int GetRandom11()
{
	int random = (rand() % (9 - 2 + 1)) + 2;
	return random;
}

int GetRandom15()
{
	int random = (rand() % (13 - 3 + 1)) + 3;
	return random;
}

int GetRandomCharacter(Character* characters[])
{
	int random = (rand() % (7 - 0 + 1)) + 0;
	if (characters[random]->isAlive)
	{
		return random;
	}
	else
	{
		GetRandomCharacter(characters);
	}
}

Vector2i GetRandomGrid()
{
	int randomX = GetRandom11();
	int randomY = GetRandom11();
	if (battlefield[randomX][randomY] != 255)
	{
		Vector2i vector = { randomX, randomY };
		return vector;
	}
	else
	{
		GetRandomGrid();
	}
}

Vector2i SetAiDestination(Vector2i vector)
{
	if (battlefield[vector.y + 1][vector.x + 2] != 255 && battlefield[vector.y + 1][vector.x + 2] != 200)
	{
		return Vector2i{ vector.x + 1, vector.y };
	}
	else if (battlefield[vector.y + 1][vector.x] != 255 && battlefield[vector.y + 1][vector.x] != 200)
	{
		return Vector2i{ vector.x - 1, vector.y };
	}
	else if (battlefield[vector.y + 2][vector.x + 1] != 255 && battlefield[vector.y + 2][vector.x + 1] != 200)
	{
		return Vector2i{ vector.x, vector.y + 1};
	}
	else if (battlefield[vector.y][vector.x + 1] != 255 && battlefield[vector.y][vector.x + 1] != 200)
	{
		return Vector2i{ vector.x, vector.y - 1 };
	}
}


void PlayTour(Character* playerCharacter, Character* aiCharacter, bool* playerIsMoving, bool* playerFinishMove, bool* aiIsMoving, int* tour, int nextTour, Vector2i mousePos, Character* aiTarget, bool* playerMarkedEnemy, Character* playerTarget, int random)
{
	if (*playerIsMoving)
	{
		if (playerCharacter->isAlive)
		{
			if (*playerMarkedEnemy)
			{
				playerCharacter->Move(SetAiDestination({ playerTarget->currentGrid.x - 1, playerTarget->currentGrid.y - 1 }));

				if (playerCharacter->currentGrid.x == playerCharacter->destinationGrid.x && playerCharacter->currentGrid.y == playerCharacter->destinationGrid.y)
				{
					playerCharacter->Attack(playerTarget);
					playerCharacter->destinationGrid.x = 0;
					playerCharacter->destinationGrid.y = 0;
					*playerIsMoving = false;
					*playerFinishMove = true;
					*aiIsMoving = true;
					*playerMarkedEnemy = false;
					SetAllGridElementsToZero();
					battlefield[playerCharacter->currentGrid.y][playerCharacter->currentGrid.x] = 255;
				}
			}
			else
			{
				playerCharacter->Move(MouseToGridPos(mousePos));

				if (playerCharacter->currentGrid.x == playerCharacter->destinationGrid.x && playerCharacter->currentGrid.y == playerCharacter->destinationGrid.y)
				{
					playerCharacter->destinationGrid.x = 0;
					playerCharacter->destinationGrid.y = 0;
					*playerIsMoving = false;
					*playerFinishMove = true;
					*aiIsMoving = true;
					SetAllGridElementsToZero();
					battlefield[playerCharacter->currentGrid.y][playerCharacter->currentGrid.x] = 255;
				}
			}
		}
		else if (!playerCharacter->isAlive)
		{
			playerCharacter->destinationGrid.x = 0;
			playerCharacter->destinationGrid.y = 0;
			*playerIsMoving = false;
			*playerFinishMove = true;
			*aiIsMoving = true;
			*playerMarkedEnemy = false;
			SetAllGridElementsToZero();
			battlefield[playerCharacter->currentGrid.y][playerCharacter->currentGrid.x] = 100;
		}
	}

	if (*playerFinishMove && *aiIsMoving)
	{
		if (aiCharacter->isAlive)
		{
			if (random < 6)
			{
				aiCharacter->Move(SetAiDestination({ aiTarget->currentGrid.x - 1, aiTarget->currentGrid.y - 1 }));
			}
			else
			{
				aiCharacter->Move(GetRandomGrid());
			}

			if (aiCharacter->currentGrid.x == aiCharacter->destinationGrid.x && aiCharacter->currentGrid.y == aiCharacter->destinationGrid.y)
			{
				if (random < 6)
				{
					aiCharacter->Attack(aiTarget);
				}
				aiCharacter->destinationGrid.x = 0;
				aiCharacter->destinationGrid.y = 0;
				*aiIsMoving = false;
				SetAllGridElementsToZero();
				battlefield[aiCharacter->currentGrid.y][aiCharacter->currentGrid.x] = 255;
				*tour = nextTour;
			}
		}
		else if(!aiCharacter->isAlive)
		{
			aiCharacter->destinationGrid.x = 0;
			aiCharacter->destinationGrid.y = 0;
			*aiIsMoving = false;
			SetAllGridElementsToZero();
			battlefield[aiCharacter->currentGrid.y][aiCharacter->currentGrid.x] = 100;
			*tour = nextTour;
		}
	}
}

int main()
{
	int random = 0;
	int aiTarget = 0;

	int tour = 0;
	bool playerIsMoving = false;
	bool playerFinishMove = false;
	bool aiIsMoving = false;
	bool playerMarkedEnemy = false;

	int enemyIndex = 0;

	srand(time(nullptr)); // Initializing the seed of the random generator with current time
	Vector2i mousePos = { 0, 0 };

	SDL_Event sdlEvent;
	SDL_Renderer* renderer = nullptr;
	SDL_Window* window = nullptr;
	SDL_Surface* surface = nullptr;

	SDL_Surface* textSurface = nullptr;
	TTF_Font* font = GetFont();

	bool initSDLResult = InitSDL(&renderer, &window);
	if (!initSDLResult)
	{
		return -1;
	}

	Character horseRider({ 1,2 }, surface, renderer, textSurface, "horseRider.png", font, 10, 18, 2);
	Character jester({ 1,3 }, surface, renderer, textSurface, "jester.png", font, 12, 16, 9);
	Character executioner({ 1,4 }, surface, renderer, textSurface, "executioner.png", font, 15, 3, 14);
	Character king({ 1,5 }, surface, renderer, textSurface, "king.png", font, 1, 50, 30);
	Character queen({ 1,6 }, surface, renderer, textSurface, "queen.png", font, 1, 30, 50);
	Character wizard({ 1,7 }, surface, renderer, textSurface, "wizard.png", font, 6, 18, 9);
	Character dragon({ 1,8 }, surface, renderer, textSurface, "dragon.png", font, 4, 13, 10);
	Character soldier({ 1,9 }, surface, renderer, textSurface, "soldier.png", font, 12, 18, 3);

	Character centaur({ 15,2 }, surface, renderer, textSurface, "centaur.png", font, 4, 18, 2);
	Character cthulhu({ 15,3 }, surface, renderer, textSurface, "cthulhu.png", font, 10, 10, 10);
	Character cyclops({ 15,4 }, surface, renderer, textSurface, "cyclops.png", font, 9, 12, 3);
	Character griffin({ 15,5 }, surface, renderer, textSurface, "griffin.png", font, 12, 14, 19);
	Character minotaur({ 15,6 }, surface, renderer, textSurface, "minotaur.png", font, 3, 14, 6);
	Character troll({ 15,7 }, surface, renderer, textSurface, "troll.png", font, 1, 12, 10);
	Character werewolf({ 15,8 }, surface, renderer, textSurface, "werewolf.png", font, 5, 13, 10);
	Character snake({ 15,9 }, surface, renderer, textSurface, "snake.png", font, 6, 15, 4);

	Character* playerCharacters[]{
	&horseRider, &jester, &executioner, &king, &queen, &wizard, &dragon, &soldier
	};

	Character* aiCharacters[]{
	&centaur, &cthulhu, &cyclops, &griffin, &minotaur, &troll, &werewolf, &snake
	};

	Obstacle obstacle1({ GetRandom15(), GetRandom11() }, surface, renderer, "stone.png");
	Obstacle obstacle2({ GetRandom15(), GetRandom11() }, surface, renderer, "stone.png");
	Obstacle obstacle3({ GetRandom15(), GetRandom11() }, surface, renderer, "stone.png");
	Obstacle obstacle4({ GetRandom15(), GetRandom11() }, surface, renderer, "stone.png");


	SDL_FreeSurface(surface);

	float acceleration = 0.5f;

	uint32_t lastTickTime = 0;
	uint32_t tickTime = 0;
	uint32_t deltaTime = 0;

	int texWidth = gridElementPixelWidth;
	int texHeight = gridElementPixelHeight;

	SetArraySides();

	// The main loop
	// Every iteration is a frame
	bool done = false;
	while (!done)
	{
		// Polling the messages from the OS.
		// That could be key downs, mouse movement, ALT+F4 or many others
		while (SDL_PollEvent(&sdlEvent))
		{
			if (sdlEvent.type == SDL_QUIT) // The user wants to quit
			{
				done = true;
			}
			else if (sdlEvent.type == SDL_KEYDOWN) // A key was pressed
			{
				switch (sdlEvent.key.keysym.sym) // Which key?
				{
				case SDLK_ESCAPE: // Posting a quit message to the OS queue so it gets processed on the next step and closes the game
					SDL_Event event;
					event.type = SDL_QUIT;
					event.quit.type = SDL_QUIT;
					event.quit.timestamp = SDL_GetTicks();
					SDL_PushEvent(&event);
					break;
					// Other keys here
				default:
					break;
				}
			}
			else if (sdlEvent.type == SDL_MOUSEBUTTONDOWN)
			{
				if (sdlEvent.button.button == SDL_BUTTON_LEFT)
				{
					playerIsMoving = true;
					SetAllGridElementsToZero();

					SDL_GetMouseState(&mousePos.x, &mousePos.y);
					aiTarget = GetRandomCharacter(playerCharacters);
					random = GetRandom11();

					Vector2i grid = MouseToGridPos(mousePos);

					for (int i = 0; i < 8; i++)
					{
						if (aiCharacters[i]->currentGrid.x == grid.x + 1 && aiCharacters[i]->currentGrid.y == grid.y + 1)
						{
							playerMarkedEnemy = true;
							enemyIndex = i;
						}
					}
				}
			}
		}

		// Clearing the screen
		SDL_RenderClear(renderer);

		switch (tour)
		{
		case 0:
			PlayTour(&horseRider, &centaur, &playerIsMoving, &playerFinishMove, &aiIsMoving, &tour, 1, mousePos, playerCharacters[aiTarget], &playerMarkedEnemy, aiCharacters[enemyIndex], random);
			break;
		case 1:
			PlayTour(&jester, &cthulhu, &playerIsMoving, &playerFinishMove, &aiIsMoving, &tour, 2, mousePos, playerCharacters[aiTarget], &playerMarkedEnemy, aiCharacters[enemyIndex], random);
			break;
		case 2:
			PlayTour(&executioner, &cyclops, &playerIsMoving, &playerFinishMove, &aiIsMoving, &tour, 3, mousePos, playerCharacters[aiTarget], &playerMarkedEnemy, aiCharacters[enemyIndex], random);
			break;
		case 3:
			PlayTour(&king, &griffin, &playerIsMoving, &playerFinishMove, &aiIsMoving, &tour, 4, mousePos, playerCharacters[aiTarget], &playerMarkedEnemy, aiCharacters[enemyIndex], random);
			break;
		case 4:
			PlayTour(&queen, &minotaur, &playerIsMoving, &playerFinishMove, &aiIsMoving, &tour, 5, mousePos, playerCharacters[aiTarget], &playerMarkedEnemy, aiCharacters[enemyIndex], random);
			break;
		case 5:
			PlayTour(&wizard, &troll, &playerIsMoving, &playerFinishMove, &aiIsMoving, &tour, 6, mousePos, playerCharacters[aiTarget], &playerMarkedEnemy, aiCharacters[enemyIndex], random);
			break;
		case 6:
			PlayTour(&dragon, &werewolf, &playerIsMoving, &playerFinishMove, &aiIsMoving, &tour, 7, mousePos, playerCharacters[aiTarget], &playerMarkedEnemy, aiCharacters[enemyIndex], random);
			break;
		case 7:
			PlayTour(&soldier, &snake, &playerIsMoving, &playerFinishMove, &aiIsMoving, &tour, 0, mousePos, playerCharacters[aiTarget], &playerMarkedEnemy, aiCharacters[enemyIndex], random);
			break;
		default:
			break;
		}

		for (int i = 0; i < 8; i++)
		{
			SetRect(&playerCharacters[i]->rect, playerCharacters[i]->position);
			SetRect(&aiCharacters[i]->rect, aiCharacters[i]->position);
			DrawImage(renderer, playerCharacters[i]->texture, playerCharacters[i]->rect);
			DrawImage(renderer, aiCharacters[i]->texture, aiCharacters[i]->rect);

			SetTextRect(&playerCharacters[i]->textRect, playerCharacters[i]->position);
			SetTextRect(&aiCharacters[i]->textRect, aiCharacters[i]->position);
			DrawImage(renderer, playerCharacters[i]->textTexture, playerCharacters[i]->textRect);
			DrawImage(renderer, aiCharacters[i]->textTexture, aiCharacters[i]->textRect);
		}
		for (int i = 0; i < 8; i++)
		{
			if (aiCharacters[i]->updateHealth == true && aiCharacters[i]->isAlive == true)
			{
				aiCharacters[i]->UpdateHealth(renderer, textSurface, font);
				aiCharacters[i]->updateHealth = false;
			}
			if (playerCharacters[i]->updateHealth == true && playerCharacters[i]->isAlive == true)
			{
				playerCharacters[i]->UpdateHealth(renderer, textSurface, font);
				playerCharacters[i]->updateHealth = false;
			}
		}

		for (int i = 0; i < 8; i++)
		{
			if (playerCharacters[i]->isAlive == false && playerCharacters[i]->imageDisabled == true)
			{
				playerCharacters[i]->texture = SetTexture(surface, renderer, "skull.png");
				SDL_DestroyTexture(playerCharacters[i]->textTexture);
				playerCharacters[i]->textTexture = SetTexture(surface, renderer, "close.png");
				playerCharacters[i]->imageDisabled = false;
			}
			if (aiCharacters[i]->isAlive == false && aiCharacters[i]->imageDisabled == true)
			{
				aiCharacters[i]->texture = SetTexture(surface, renderer, "skull.png");;
				SDL_DestroyTexture(aiCharacters[i]->textTexture);
				aiCharacters[i]->textTexture = SetTexture(surface, renderer, "close.png");;
				aiCharacters[i]->imageDisabled = false;
			}
		}

		SetRect(&obstacle1.rectObstacle, obstacle1.position);
		SetRect(&obstacle2.rectObstacle, obstacle2.position);
		SetRect(&obstacle3.rectObstacle, obstacle3.position);
		SetRect(&obstacle4.rectObstacle, obstacle4.position);
	   
		DrawImage(renderer, obstacle1.texture, obstacle1.rectObstacle);
		DrawImage(renderer, obstacle2.texture, obstacle2.rectObstacle);
		DrawImage(renderer, obstacle3.texture, obstacle3.rectObstacle);
		DrawImage(renderer, obstacle4.texture, obstacle4.rectObstacle);

		// Showing the screen to the player
		SDL_RenderPresent(renderer);

		// next frame...
	}
	// If we reached here then the main loop stoped
	// That means the game wants to quit

	SDL_FreeSurface(textSurface);
	// Shutting down the renderer
	SDL_DestroyRenderer(renderer);

	// Shutting down the window
	SDL_DestroyWindow(window);

	// Quitting the Image SDL library
	IMG_Quit();
	// Quitting the main SDL library
	SDL_Quit();

	// Done.
	return 0;
}