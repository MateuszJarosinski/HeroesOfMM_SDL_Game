#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include <Windows.h>

SDL_Texture* SetTexture(SDL_Surface* surface, SDL_Renderer* renderer, const char* imagePath);

typedef unsigned char uchar;

const int screenWidth = 1920;
const int screenHeight = 1080;

const int gridHeight = 11;
const int gridWidth = 15;

const int gridElementPixelHeight = 98;
const int gridElementPixelWidth = 128;

//the board is 13/17 because I need space for the sides of the map (I need them later)
//in practice the player plays on 11/15 map
uchar battlefield[13][17];

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

struct Character
{
	SDL_Texture* texture;
	Vector2i position;
	Vector2i currentGrid;

	Character(Vector2i pos, SDL_Surface* sur, SDL_Renderer* rend, const char* imagePath);
	void PlaceCharacter(Vector2i pos);
	void Move(Vector2i destinationGrid);
};

void Character::PlaceCharacter(Vector2i pos)
{
	position.x = (pos.x - 1) * gridElementPixelWidth + gridElementPixelWidth / 2;
	position.y = (pos.y - 1) * gridElementPixelHeight + gridElementPixelHeight / 2;
}

Character::Character(Vector2i pos, SDL_Surface* sur, SDL_Renderer* rend, const char* imagePath)
{
	PlaceCharacter(pos);
	texture = SetTexture(sur, rend, imagePath);
}

void Character::Move(Vector2i destinationGrid)
{
	currentGrid = { position.x / gridElementPixelWidth ,position.y / gridElementPixelHeight };
	destinationGrid.x += 1;
	destinationGrid.y += 1;
	currentGrid.x += 1;
	currentGrid.y += 1;

	if (battlefield[destinationGrid.y][destinationGrid.x] != 255)
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
	SetArraySides();

	if (destination != 255)
	{
		
		if (playerPos > up)
		{
			//printf("check\n");
			if (position.y - gridElementPixelHeight >= gridElementPixelHeight / 2)
			{
				position.y -= gridElementPixelHeight;
				currentGrid.y -= 1;
			}
			Sleep(150);
		}
		else if (playerPos > down)
		{
			//printf("check\n");
			if (position.y + gridElementPixelHeight <= (screenHeight - gridElementPixelHeight / 2))
			{
				position.y += gridElementPixelHeight;
				currentGrid.y += 1;
			}
			Sleep(150);
		}

		if (playerPos > left)
		{
			//printf("check\n");
			if (position.x - gridElementPixelWidth >= gridElementPixelWidth / 2)
			{
				position.x -= gridElementPixelWidth;
				currentGrid.x -= 1;
			}
			Sleep(150);
		}
		else if (playerPos > right)
		{
			//printf("check\n");
			if (position.x + gridElementPixelWidth <= (screenWidth - gridElementPixelWidth / 2))
			{
				position.x += gridElementPixelWidth;
				currentGrid.x += 1;

			}
			Sleep(150);
		}
	}
}

struct Obstacle
{
	SDL_Texture* texture;

	Vector2i position;

	Obstacle(Vector2i pos, SDL_Surface* surface, SDL_Renderer* renderer, const char* imagePath);
	void PlaceObstacle();
};

Obstacle::Obstacle(Vector2i pos, SDL_Surface* surface, SDL_Renderer* renderer, const char* imagePath)
{
	position = pos;
	PlaceObstacle();
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

void SetAllGridElementsToZero()
{
	for (int i = 0; i < 13; i++)
	{
		for(int j = 0; j < 17; j++)
		{
			battlefield[i][j] = 0;
		}
	}
}

void SetObstaclePlacement()
{
	//position on grid is: position += 1
	battlefield[10][3] = 255;
	battlefield[9][9] = 255;
	battlefield[4][3] = 255;
	battlefield[7][5] = 255;
}

void PrintArray()
{
	for (int rows = 0; rows < 13; rows++)
	{
		for (int columns = 0; columns < 17; columns++)
		{
			printf("%d  ", battlefield[rows][columns]);
		}
		printf("\n");
	}
	printf("\n");
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
	SDL_SetRenderDrawColor(*renderer, 11, 22, 33, 255);

	return true;
}

int main()
{
	Vector2i mousePos = { 960,  0 + gridElementPixelHeight / 2 + gridElementPixelHeight };

	SDL_Event sdlEvent;
	SDL_Renderer* renderer = nullptr;
	SDL_Window* window = nullptr;
	SDL_Surface* surface = nullptr;
	bool initSDLResult = InitSDL(&renderer, &window);
	if (!initSDLResult)
	{
		return -1;
	}

	Character player({ 15,11 }, surface, renderer, "spaceship.png");
	Character enemy({ 3,3 }, surface, renderer, "star-wars.png");

	Obstacle obstacle1({3,10}, surface, renderer, "star-wars.png");
	Obstacle obstacle2({9,9}, surface, renderer, "star-wars.png");
	Obstacle obstacle3({3,4}, surface, renderer, "star-wars.png");
	Obstacle obstacle4({5,7}, surface, renderer, "star-wars.png");

	SDL_FreeSurface(surface);

	float acceleration = 0.5f;

	uint32_t lastTickTime = 0;
	uint32_t tickTime = 0;
	uint32_t deltaTime = 0;

	int texWidth = gridElementPixelWidth;
	int texHeight = gridElementPixelHeight;

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
				if (sdlEvent.button.button == SDL_BUTTON_RIGHT)
				{
					SetAllGridElementsToZero();
					SetObstaclePlacement();

					SDL_GetMouseState(&mousePos.x, &mousePos.y);
				}
			}
		}

		// Clearing the screen
		SDL_RenderClear(renderer);
		// All drawing goes here

		// Let's draw a sample image

		//calculating deltaTime
		deltaTime = DeltaTime(&lastTickTime, &tickTime);

		player.Move(MouseToGridPos(mousePos));

		// Here is the rectangle where the image will be on the screen
		SDL_Rect rect;
		SDL_Rect rect1;
		SDL_Rect rect2;
		SDL_Rect rect3;
		SDL_Rect rect4;
		SDL_Rect enemyRect;

		SetRect(&rect, player.position);
		SetRect(&enemyRect, enemy.position);
		SetRect(&rect1, obstacle1.position);
		SetRect(&rect2, obstacle2.position);
		SetRect(&rect3, obstacle3.position);
		SetRect(&rect4, obstacle4.position);

		DrawImage(renderer, player.texture, rect);
		DrawImage(renderer, enemy.texture, enemyRect);
		DrawImage(renderer, obstacle1.texture, rect1);
		DrawImage(renderer, obstacle2.texture, rect2);
		DrawImage(renderer, obstacle3.texture, rect3);
		DrawImage(renderer, obstacle4.texture, rect4);

		// Showing the screen to the player
		SDL_RenderPresent(renderer);

		// next frame...
	}
	// If we reached here then the main loop stoped
	// That means the game wants to quit

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

//Left
	//if (currentX < x && (x - (texWidth/2) > 0))
	//{
	//	x -= acceleration * deltaTime;
	//}
	////Right
	//if (currentX > x && (x + (texWidth/2) < screenWidth))
	//{
	//	x += acceleration * deltaTime;
	//}
	////Down
	//if (currentY < y && (y - (texHeight/2) > 0))
	//{
	//	y -= acceleration * deltaTime;
	//}
	////Up
	//if (currentY > y && (y + (texHeight / 2) < screenHeight))
	//{
	//	y += acceleration * deltaTime;
	//}