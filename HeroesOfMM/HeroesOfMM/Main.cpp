#include <stdio.h>
#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include <Windows.h>

#define screenWidth 1920
#define screenHeight 1080

#define gridHeight 11
#define gridWidth 15

#define gridElementPixelHeight 98
#define gridElementPixelWidth 128

//the board is 13/17 because I need space for the sides of the map (I need them later)
//in practice the player plays on 11/15 map
unsigned char grid[13][17];

struct Obstacle
{
	int positionX;
	int positionY;
};

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
	grid[9][2] = 255;
	grid[8][8] = 255;
	grid[3][2] = 255;
	grid[6][4] = 255;
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
				unsigned char A = grid[i][j];

				if (A != 0 && A != 255)
				{
					unsigned char B = A + 1;
					unsigned char gridDown = grid[i + 1][j];
					unsigned char gridUp = grid[i - 1][j];
					unsigned char gridRight = grid[i][j + 1];
					unsigned char gridLeft = grid[i][j - 1];

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

SDL_Texture* SetTexture(SDL_Surface* surface, SDL_Renderer* renderer, const char* image_path)
{
	surface = IMG_Load(image_path);
	if (!surface)
	{
		printf("Unable to load an image %s. Error: %s", image_path, IMG_GetError());
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

void SetRect(SDL_Rect* rect, int x, int y, int tex_width, int tex_height)
{
	rect->x = (int)round(x - tex_width / 2); // Counting from the image's center but that's up to you
	rect->y = (int)round(y - tex_height / 2); // Counting from the image's center but that's up to you
	rect->w = (int)tex_width;
	rect->h = (int)tex_height;
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

	int tex_width = gridElementPixelWidth;
	int tex_height = gridElementPixelHeight;

	// Bye-bye the surface
	SDL_FreeSurface(surface);

	//SetAllGridElementsToZero();
	//ArrayHack();
	//SetObstacles();

	bool done = false;
	SDL_Event sdl_event;

	Obstacle obstacle1;
	obstacle1.positionX = 2 * gridElementPixelWidth + gridElementPixelWidth / 2;
	obstacle1.positionY = 9 * gridElementPixelHeight + gridElementPixelHeight / 2;

	Obstacle obstacle2;
	obstacle2.positionX = 8 * gridElementPixelWidth + gridElementPixelWidth / 2;
	obstacle2.positionY = 8 * gridElementPixelHeight + gridElementPixelHeight / 2;

	Obstacle obstacle3;
	obstacle3.positionX = 2 * gridElementPixelWidth + gridElementPixelWidth / 2;
	obstacle3.positionY = 3 * gridElementPixelHeight + gridElementPixelHeight / 2;

	Obstacle obstacle4;
	obstacle4.positionX = 4 * gridElementPixelWidth + gridElementPixelWidth / 2;
	obstacle4.positionY = 6 * gridElementPixelHeight + gridElementPixelHeight / 2;

	// The main loop
	// Every iteration is a frame
	while (!done)
	{
		// Polling the messages from the OS.
		// That could be key downs, mouse movement, ALT+F4 or many others
		while (SDL_PollEvent(&sdl_event))
		{
			if (sdl_event.type == SDL_QUIT) // The user wants to quit
			{
				done = true;
			}
			else if (sdl_event.type == SDL_KEYDOWN) // A key was pressed
			{
				switch (sdl_event.key.keysym.sym) // Which key?
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
			else if (sdl_event.type == SDL_MOUSEBUTTONDOWN)
			{
				if (sdl_event.button.button == SDL_BUTTON_RIGHT)
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

		unsigned char destination = grid[rowNum][columnNum];
		unsigned char player = grid[playerRowLocation][playerColumnLocation];
		unsigned char down = grid[playerRowLocation + 1][playerColumnLocation];
		unsigned char up = grid[playerRowLocation - 1][playerColumnLocation];
		unsigned char right = grid[playerRowLocation][playerColumnLocation + 1];
		unsigned char left = grid[playerRowLocation][playerColumnLocation - 1];

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

		SetRect(&rect, x, y, tex_width, tex_height);
		SetRect(&rect1, obstacle1.positionX , obstacle1.positionY, tex_width, tex_height);
		SetRect(&rect2, obstacle2.positionX , obstacle2.positionY, tex_width, tex_height);
		SetRect(&rect3, obstacle3.positionX , obstacle3.positionY, tex_width, tex_height);
		SetRect(&rect4, obstacle4.positionX , obstacle4.positionY, tex_width, tex_height);

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
	//if (currentX < x && (x - (tex_width/2) > 0))
	//{
	//	x -= acceleration * deltaTime;
	//}
	////Right
	//if (currentX > x && (x + (tex_width/2) < screenWidth))
	//{
	//	x += acceleration * deltaTime;
	//}
	////Down
	//if (currentY < y && (y - (tex_height/2) > 0))
	//{
	//	y -= acceleration * deltaTime;
	//}
	////Up
	//if (currentY > y && (y + (tex_height / 2) < screenHeight))
	//{
	//	y += acceleration * deltaTime;
	//}