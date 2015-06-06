#ifndef TEST_H
#define TEST_H

#include <SDL2/SDL.h>
#include "LTexture.h"

typedef std::string string;

struct v2
{
	int x, y;
};

struct pixel
{
public:
    int x, y;
	int ix, iy; //initial x and y
	Uint32 color;
	float xSpeed;
	float ySpeed;

	bool blank;
private:
	
};

struct verticalPixelExplosion
{
	LTexture texture;
	pixel* pixels;
	int pixelCount;
 	int radiationLine;
	int step;
	int direction;

	int originX, originY;
	int initialSpeedX;
};

bool init();
bool loadMedia();
void close();
SDL_Surface* loadSurface( std::string path );
SDL_Texture* loadTexture( std::string path );
Uint32 makeColor(Uint8 blue, Uint8 green, Uint8 red, Uint8 alpha = 0xff);
void createCard();
void explodePixelsVertically();

#endif
