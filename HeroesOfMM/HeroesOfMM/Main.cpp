#include <stdio.h>
#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include <Windows.h>

typedef unsigned char uchar;

const int screenWidth = 1920;
const int screenHeight = 1080;

const int gridHeight = 11;
const int gridWidth = 15;

const int gridElementPixelHeight = 98;
const int gridElementPixelWidth = 128;

//the board is 13/17 because I need space for the sides of the map (I need them later)
//in practice the player plays on 11/15 map
uchar grid[13][17];

struct Vector2i
{
	int x;
	int y;
};

struct Character
{
	Vector2i position;
};

struct Obstacle
{
	Vector2i position;
	void SetPosition(Vector2i position);
};

void Obstacle::SetPosition(Vector2i pos)
{
	position.x = (pos.x - 1) * gridElementPixelWidth + gridElementPixelWidth / 2;
	position.y = (pos.y - 1) * gridElementPixelHeight + gridElementPixelHeight / 2;
}

//struct Image
//{
//	SDL_Texture* texture;
//	Vector2i texSize;
//	void Init(const char* fileName);
//	void Destroy();
//	void Render(SDL_Renderer* renderer, Vector2i position);
//};
//
//void Image::Init(const char* fileName)
//{
//	/*texture = tex;
//	texSize = size;*/
//}
//
//void Image::Destroy()
//{
//	SDL_DestroyTexture(texture);
//}
//
//void Image::Render(SDL_Renderer* renderer, Vector2i position)
//{
//
//}

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
			grid[i][j] = 0;
		}
	}
}

void SetObstaclePlacement()
{
	//position on grid is: position += 1
	grid[10][3] = 255;
	grid[9][9] = 255;
	grid[4][3] = 255;
	grid[7][5] = 255;
}

void SetArraySides()
{
	for (int i = 0; i < 15; i++)
	{
		grid[0][i] = 255;
	}
	for (int i = 0; i < 11; i++)
	{
		grid[i][0] = 255;
	}

	for (int i = 0; i < 15; i++)
	{
		grid[12][i] = 255;
	}
	for (int i = 0; i < 11; i++)
	{
		grid[i][16] = 255;
	}
}

void PrintArray()
{
	for (int rows = 0; rows < 13; rows++)
	{
		for (int columns = 0; columns < 17; columns++)
		{
			printf("%d  ", grid[rows][columns]);
		}
		printf("\n");
	}
	printf("\n");
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
				uchar A = grid[i][j];

				if (A != 0 && A != 255)
				{
					uchar B = A + 1;
					uchar gridDown = grid[i + 1][j];
					uchar gridUp = grid[i - 1][j];
					uchar gridRight = grid[i][j + 1];
					uchar gridLeft = grid[i][j - 1];

					if (i < 11 && (gridDown != 255 && gridDown < B))
					{
						if (grid[i + 1][j] == 0)
						{
							grid[i + 1][j] = B;
							S = true;
						};
					}
					if (i > 0 && (gridUp != 255 && gridUp < B))
					{
						if (grid[i - 1][j] == 0)
						{
							grid[i - 1][j] = B;
							S = true;
						};
					}
					if (j < 15 && (gridRight != 255 && gridRight < B))
					{
						if (grid[i][j + 1] == 0)
						{
							grid[i][j + 1] = B;
							S = true;
						};
					}
					if (j > 0 && (gridLeft != 255 && gridLeft < B))
					{
						if (grid[i][j - 1] == 0)
						{
							grid[i][j - 1] = B;
							S = true;
						};
					}
				}
			}
		}
	}
	//PrintArray();
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

int main()
{
	//starting position
	int x = 960.0f;
	int y = 0 + gridElementPixelHeight/2 + gridElementPixelHeight;

	int columnNum = 0;
	int rowNum = 0;
	int playerColumnLocation = 0;
	int playerRowLocation = 0;

	int destinationX = x;
	int destinationY = y;

	float acceleration = 0.5f;

	uint32_t lastTickTime = 0;
	uint32_t tickTime = 0;
	uint32_t deltaTime = 0;

	// Init SDL libraries
	SDL_SetMainReady(); // Just leave it be
	int result = 0;
	result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO); // Init of the main SDL library
	if (result) // SDL_Init returns 0 (false) when everything is OK
	{
		printf("Can't initialize SDL. Error: %s", SDL_GetError()); // SDL_GetError() returns a string (as const char*) which explains what went wrong with the last operation
		return -1;
	}

	result = IMG_Init(IMG_INIT_PNG); // Init of the Image SDL library. We only need to support PNG for this project
	if (!(result & IMG_INIT_PNG)) // Checking if the PNG decoder has started successfully
	{
		printf("Can't initialize SDL image. Error: %s", SDL_GetError());
		return -1;
	}

	// Creating the window 1920x1080 (could be any other size)
	SDL_Window* window = SDL_CreateWindow("HeroesOfMM",
		0, 0,
		screenWidth, screenHeight,
		SDL_WINDOW_SHOWN);

	if (!window)
		return -1;

	// Creating a renderer which will draw things on the screen
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer)
		return -1;

	// Setting the color of an empty window (RGBA). You are free to adjust it.
	SDL_SetRenderDrawColor(renderer, 11, 22, 33, 255);

	SDL_Surface* surface = nullptr;

	SDL_Texture* texture = SetTexture(surface, renderer, "spaceship.png");
	SDL_Texture* obstacle_texture = SetTexture(surface, renderer, "star-wars.png");

	int texWidth = gridElementPixelWidth;
	int texHeight = gridElementPixelHeight;

	// Bye-bye the surface
	SDL_FreeSurface(surface);

	//SetAllGridElementsToZero();
	//ArrayHack();
	//SetObstacles();

	bool done = false;
	SDL_Event sdlEvent;

	Obstacle obstacle1;
	obstacle1.SetPosition(Vector2i{ 3,10 });

	Obstacle obstacle2;
	obstacle2.SetPosition(Vector2i{ 9,9 });
   
	Obstacle obstacle3;
	obstacle3.SetPosition(Vector2i{ 3,4 });

	Obstacle obstacle4;
	obstacle4.SetPosition(Vector2i{ 5,7 });

	// The main loop
	// Every iteration is a frame
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

					SDL_GetMouseState(&destinationX, &destinationY);

					columnNum = destinationX / gridElementPixelWidth;
					rowNum = destinationY / gridElementPixelHeight;
					playerColumnLocation = x / gridElementPixelWidth;
					playerRowLocation = y / gridElementPixelHeight;

					columnNum += 1;
					rowNum += 1;

					playerColumnLocation += 1;
					playerRowLocation += 1;

					printf("col %i\n", columnNum);
					printf("row %i\n", rowNum);

					printf("p col %i\n", playerColumnLocation);
					printf("p row %i\n", playerRowLocation);


					if (grid[rowNum][columnNum] != 255)
					{
						grid[rowNum][columnNum] = 1;
					}
					GrassfireAlgorithm();
					SetArraySides();
					PrintArray();
				}
			}
		}

		// Clearing the screen
		SDL_RenderClear(renderer);
		// All drawing goes here

		// Let's draw a sample image

		//calculating deltaTime
		deltaTime = DeltaTime(&lastTickTime, &tickTime);

		uchar destination = grid[rowNum][columnNum];
		uchar player = grid[playerRowLocation][playerColumnLocation];
		uchar down = grid[playerRowLocation + 1][playerColumnLocation];
		uchar up = grid[playerRowLocation - 1][playerColumnLocation];
		uchar right = grid[playerRowLocation][playerColumnLocation + 1];
		uchar left = grid[playerRowLocation][playerColumnLocation - 1];

		if (destination != 255)
		{
			if (player > up)
			{
				if (y - gridElementPixelHeight >= gridElementPixelHeight / 2)
				{
					y -= gridElementPixelHeight;
					playerRowLocation -= 1;
				}
				Sleep(150);
			}
			else if (player > down)
			{
				if (y + gridElementPixelHeight <= (screenHeight - gridElementPixelHeight / 2))
				{;
					y += gridElementPixelHeight;
					playerRowLocation += 1;
				}
				Sleep(150);
			}

			if (player > left)
			{
				if (x - gridElementPixelWidth >= gridElementPixelWidth / 2)
				{
					x -= gridElementPixelWidth;
					playerColumnLocation -= 1;
				}
				Sleep(150);
			}
			else if (player > right)
			{
				if (x + gridElementPixelWidth <= (screenWidth - gridElementPixelWidth / 2))
				{
					x += gridElementPixelWidth;
					playerColumnLocation += 1;

				}
				Sleep(150);
			}
		}

		// Here is the rectangle where the image will be on the screen
		SDL_Rect rect;
		SDL_Rect rect1;
		SDL_Rect rect2;
		SDL_Rect rect3;
		SDL_Rect rect4;

		SetRect(&rect, Vector2i{x,y});
		SetRect(&rect1, obstacle1.position);
		SetRect(&rect2, obstacle2.position);
		SetRect(&rect3, obstacle3.position);
		SetRect(&rect4, obstacle4.position);

		DrawImage(renderer, texture, rect);
		DrawImage(renderer, obstacle_texture, rect1);
		DrawImage(renderer, obstacle_texture, rect2);
		DrawImage(renderer, obstacle_texture, rect3);
		DrawImage(renderer, obstacle_texture, rect4);

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