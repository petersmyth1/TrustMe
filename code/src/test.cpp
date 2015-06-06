#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <SDL2_ttf/SDL_ttf.h>
#include <SDL2_mixer/SDL_mixer.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <sstream>
#include "test.h"
#include "LTexture.h"
#include "Leap.h"
using namespace Leap;

//Screen dimension constants
const int SCREEN_WIDTH = 1280; //900;
const int SCREEN_HEIGHT = 800; //600;

int mouseX, mouseY;
int mouseDeltaX, mouseDeltaY;
int butt;
//Leap Motion stuff
Controller lmController;
Frame lmFrame;
Leap::HandList lmHands;
float lmDeltaX, lmDeltaY;
float lmX, lmY; 
Uint64 lastFrameID;
SDL_Rect handRect;
LTexture debugTextTexture;
string debugText;
bool handsLastFrame = false;;

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

SDL_Renderer* gRenderer = NULL;
SDL_Surface* gScreenSurface;

LTexture gCardTexture;
LTexture gEmptyTexture;
LTexture gBackgroundTexture;
LTexture gComputerOverlayTexture;
LTexture gWhiteGradientTexture;

Mix_Chunk* gCreditCardSwipeStart = NULL;
Mix_Chunk* gCreditCardSwipeMiddle = NULL;
Mix_Chunk* gCreditCardSwipeEnd = NULL;

int tCreditCardSwipeStart = 0;
int tEnteredSlot = 0;
int tCreditCardSwipeMiddle = 0;
bool canPlaySwipeMiddleSound = false;
bool creditCardSwipeInProgress = false;
int tCreditCardSwipeDuration = 0;

LTexture card;
bool canCreateCard = true;

verticalPixelExplosion vpe;

struct Card
{
	float x, y;
	LTexture texture;
	int tCardCreated;
	bool loaded;
} gCard;

//Text and font related things
TTF_Font *gFontSmall;
TTF_Font *gFont;
TTF_Font *gFontBig;
TTF_Font *gFontTitle;
TTF_Font *gDebugFont;
LTexture gPromptTexture;
LTexture gCreditCardNumberTexture;
LTexture gPressEnterTexture;
LTexture gTitleTexture;
string creditCardNumber;
int cursorPosition;
int tCursor;

int tFade;
int tFontWait;

bool title = true;

enum TITLE_STATES
{
	TITLE_STATE_NEW,
	TITLE_STATE_FADING,
	TITLE_STATE_FADED
};
int TITLE_STATE = TITLE_STATE_NEW;

int tBackspace = 0;

/////// >> WARNINGS AND NOTES << \\\\\\\

//Primary pixel format: SDL_PIXELFORMAT_ARGB8888

//Accessing PixelFormat from a surface like this - surface->format - CAUSES SEGMENTATION FAULT
		 
/////// >> END << \\\\\\\


//Starts up SDL and creates window
bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
		{
			printf( "Warning: Linear texture filtering not enabled!" );
		}

		//Create window
		gWindow = SDL_CreateWindow( "xxxx-xxxx-xxxx-xxxx",
									SDL_WINDOWPOS_UNDEFINED,
									SDL_WINDOWPOS_UNDEFINED,
									SCREEN_WIDTH,
									SCREEN_HEIGHT,
									SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n",
					SDL_GetError() );
			success = false;
		}
		else
		{
		    IMG_Init( IMG_INIT_PNG );
			TTF_Init();
			Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
			
			SDL_SetRelativeMouseMode(SDL_TRUE);
			
			SDL_SetWindowFullscreen(gWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
			
			//Create renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1,
											SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
			gScreenSurface = SDL_GetWindowSurface(gWindow);
			if( gRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n",
						SDL_GetError() );
				success = false;
			}
		}
	}

	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//Textures
	gCardTexture.loadFromFileIMG("../assets/lowres_credit_cards/chemical_card.png",
								 gRenderer, gWindow);
	vpe.texture.createTexture(gRenderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	gComputerOverlayTexture.loadFromFileIMG("../assets/computerFront_wide-r.png", gRenderer, gWindow);
	gBackgroundTexture.loadFromFileIMG("../assets/computerBack_wide-r.gif", gRenderer, gWindow);
	gWhiteGradientTexture.loadFromFileIMG("../assets/white-gradient-bothSides-whiter.png", gRenderer, gWindow);
	
	//Music & Sounds
	gCreditCardSwipeStart = Mix_LoadWAV("../assets/sounds/creditCardSwipe_start.wav");
	Mix_VolumeChunk(gCreditCardSwipeStart, MIX_MAX_VOLUME/10);
	gCreditCardSwipeMiddle = Mix_LoadWAV("../assets/sounds/creditCardSwipe_middle.wav");
	Mix_VolumeChunk(gCreditCardSwipeMiddle, MIX_MAX_VOLUME/2);
	
    //Fonts
	gFontSmall = TTF_OpenFont( "../assets/fonts/NEOTERIC.ttf", 20);
	gFont = TTF_OpenFont( "../assets/fonts/NEOTERIC.ttf", 28);
	gFontBig = TTF_OpenFont( "../assets/fonts/NEOTERIC.ttf", 50);
	gFontTitle = TTF_OpenFont( "../assets/fonts/NEOTERIC.ttf", 120);
	gDebugFont = TTF_OpenFont( "../assets/fonts/LiberationMono.ttf", 50);
	
	SDL_Color textColor = {255, 255, 255};
	gPromptTexture.loadFromRenderedText("ENTER YOUR CREDIT CARD NUMBER:",
										gRenderer,
										gFont,
										textColor);
    gCreditCardNumberTexture.loadFromRenderedText("XXXX-XXXX-XXXX-XXXX",
												  gRenderer,
												  gFontBig,
												  textColor);
	gPressEnterTexture.loadFromRenderedText("PRESS ENTER",
											gRenderer,
											gFontSmall,
											textColor);
	gTitleTexture.loadFromRenderedText("TRUST ME",
									   gRenderer,
									   gFontTitle,
									   textColor);
	cursorPosition = 0;
	tCursor = SDL_GetTicks();
	tFade = SDL_GetTicks();
	tFontWait = SDL_GetTicks();
	debugText = "";
	lmX = SCREEN_WIDTH/2;
	lmY = SCREEN_HEIGHT/2;
	
	return success;
}

//Frees media and shuts down SDL
void close()
{
	//Delete whatever shit
	delete(vpe.pixels);
	
	//Destroy window	
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	SDL_Quit();
	IMG_Quit();
	TTF_Quit();
}

Uint32 makeColor(Uint8 blue, Uint8 green, Uint8 red, Uint8 alpha)
{
	Uint32 finalColor = blue | (green << 8) | (red << 16) | (alpha << 24);
	return finalColor;
}

int map(int s, int a1, int a2, int b1, int b2)
{
    return b1 + (s-a1)*(b2-b1)/(a2-a1);
}

float map(float s, float a1, float a2, float b1, float b2)
{
    return b1 + (s-a1)*(b2-b1)/(a2-a1);
}

void createCard()
{
	gCard.texture.loadFromFileIMG("../assets/lowres_credit_cards/blank-card.png",
										gRenderer, gWindow);
	string nums [4];
	for(int i = 0;
		i < 4;
		++i)
	{
		for(int ii = 0;
		ii < 4;
		++ii)
		{
			nums[i] += creditCardNumber.at(ii + (i*4));
		}
	}
	
	int red;
	std::stringstream(nums[0]) >> red;
	int green;
	std::stringstream(nums[1]) >> green;
	int blue;
	std::stringstream(nums[2]) >> blue;
	
	red = (float)red / (9999/255);
	blue = (float)blue / (9999/255);
	green = (float)green / (9999/255);

	gCard.texture.lockTexture();

	Uint32* pixels = (Uint32* )gCard.texture.getPixels();
	int pixelCount = gCard.texture.getWidth() * gCard.texture.getHeight();
	for(int i = 0;
		i < pixelCount;
		++i)
	{
		Uint32 pixel = pixels[i];
		if((pixel & 0x000000ff) == 0)
		{
			pixels[i] = makeColor(0,0,0,0);
		}
		else
		{
			pixels[i] = makeColor((Uint8)((red * 1.5) + gCard.texture.getPixelRed(pixel)) / 2.5,
								  (Uint8)((green * 1.5) + gCard.texture.getPixelGreen(pixel)) / 2.5,
								  (Uint8)((blue * 1.5) + gCard.texture.getPixelBlue(pixel)) / 2.5,
								  255);
		}
	}

	//Use last set of 4 numbers to do weird shifty shit
	string s1, s2;
	s1 = nums[3].at(0);
	s1 += nums[3].at(1);
	s2 = nums[3].at(2);
	s2 += nums[3].at(3);
	int i1, i2;
	std::stringstream(s1) >> i1;
	std::stringstream(s2) >> i2;
	
	gCard.texture.unlockTexture();

	gCard.x = SCREEN_WIDTH/2 - gCard.texture.getWidth()/2;
	gCard.y = 250;
	gCard.tCardCreated = SDL_GetTicks();
	gCard.loaded = true;
}

//pixels: array of pixels, radiationLine: line from which pixels move away
void explodePixelsVertically()
{
	vpe.texture.lockTexture();

	//Clear pixels
	Uint32* pixels = (Uint32* )vpe.texture.getPixels();
	int pixelCount = vpe.texture.getWidth() * vpe.texture.getHeight();
	for(int i = 0;
		i < pixelCount;
		++i)
	{
	    pixels[i] = makeColor(0, 0, 0, 0);
	}

	//Update pixel location
	pixelCount = gCard.texture.getWidth() * gCard.texture.getHeight();
	for(int i = 0;
		i < pixelCount;
		++i)
	{
		pixel* pixel = &vpe.pixels[i];

		//Moving right
		if(vpe.direction > 0)
		{
			if(vpe.pixels[i].x < SCREEN_WIDTH - 450)
			{
				if(pixel->xSpeed > 10)
				{
					pixel->xSpeed -= (pixel->xSpeed - 1) * .4;
				}
				else if(pixel->xSpeed > 5)
				{
					pixel->xSpeed -= (pixel->xSpeed - 1) * .2;
				}
				else
				{
					pixel->xSpeed -= (pixel->xSpeed - 1) * .1;
				}

				if(pixel->x > (SCREEN_WIDTH - 300) - 50 && pixel->xSpeed > 5)
				{
					pixel->xSpeed = 5;
				}
				else if(pixel->x > (SCREEN_WIDTH - 300) && pixel->x < (SCREEN_WIDTH - 300) + 100 &&  pixel->xSpeed > 1)
				{
					pixel->xSpeed = 1;
				}
			}
			else
			{
				vpe.pixels[i].xSpeed += .1 + ((float)(rand() % 4 + 1 ))/10;
				float yIncrement;
				yIncrement = (vpe.pixels[i].iy - (float)vpe.radiationLine) / 10;
				//(vpe.pixels[i].iy - (float)vpe.radiationLine) / ((float)vpe.radiationLine/2);		    
				vpe.pixels[i].ySpeed += yIncrement + ((float)(rand() % 5 )-2)/10;;
			}

			if(vpe.pixels[i].x + vpe.pixels[i].xSpeed >= 1279)
			{
				vpe.pixels[i].x = 1279;
			}
			else
			{
				vpe.pixels[i].x += vpe.pixels[i].xSpeed;
			}
		}
		else
		{
			if(vpe.pixels[i].x > 450)
			{
				if(pixel->xSpeed < -10)
				{
					pixel->xSpeed += (-1-pixel->xSpeed) * .4;
				}
				else if(pixel->xSpeed < -5)
				{
					pixel->xSpeed += (-1-pixel->xSpeed) * .2;
				}
				else
				{
					pixel->xSpeed += (-1-pixel->xSpeed) * .1;
				}
				
				
			    if(pixel->x >= 450  && pixel->xSpeed < -5)
				{
					pixel->xSpeed = -5;
				}
				else if(pixel->x >= 450 &&
						pixel->x < 550 &&  pixel->xSpeed < -1)
				{
					pixel->xSpeed = -2;
				}
			}
			else
			{
				vpe.pixels[i].xSpeed -= .1 + ((float)(rand() % 4 + 1 ))/10;
				float yIncrement;
				yIncrement = (vpe.pixels[i].iy - (float)vpe.radiationLine) / 10;
				//(vpe.pixels[i].iy - (float)vpe.radiationLine) / ((float)vpe.radiationLine/2);		    
				vpe.pixels[i].ySpeed += yIncrement + ((float)(rand() % 5 )-2)/10;
			}

			if(vpe.pixels[i].x + vpe.pixels[i].xSpeed <=1 )
			{
				vpe.pixels[i].x = 1;
			}
			else
			{
				vpe.pixels[i].x += vpe.pixels[i].xSpeed;
			}
		}
		
		vpe.pixels[i].y += vpe.pixels[i].ySpeed;
		
		//Check if pixel is outside screen
		if(vpe.pixels[i].x > SCREEN_WIDTH || vpe.pixels[i].x < 0 ||
		   vpe.pixels[i].y > SCREEN_HEIGHT || vpe.pixels[i].y < 0)
		{
		    continue;
		}

		//Set colors of pixels to their new locations
		int destPixel = vpe.pixels[i].x + (vpe.pixels[i].y * vpe.texture.getWidth());
		if(vpe.pixels[i].x >= SCREEN_WIDTH)
		{
			vpe.pixels[destPixel].color = makeColor(0,0,0,0);
		}
		else
		{
			pixels[destPixel] = vpe.pixels[i].color;
		}
	}

	if(vpe.direction > 0)
	{
		if(vpe.pixels[0].x > SCREEN_WIDTH-2 && vpe.pixels[0].y < 2)
		{
			vpe.pixels = NULL;
			vpe.texture.clearTextureToColor(makeColor(0,0,0,0));
		}
	}
	else
	{
		if(vpe.pixels[vpe.texture.getWidth()-1].x < 2 && vpe.pixels[0].y < 2)
		{
			vpe.pixels = NULL;
			vpe.texture.clearTextureToColor(makeColor(0,0,0,0));
		}
	}
	vpe.texture.unlockTexture();
}

int main( int argc, char* args[] )
{
	//Start up SDL and create window
	if( !init() )
	{
		printf( "Failed to initialize!\n" );
	}
	else
	{
		if( !loadMedia() )
		{
			printf( "Failed to load media!\n" );
		}
		else
		{   
			bool quit = false;
			SDL_Event e;

			int computerX = 580;
			int computerY = 540;
			int computerW = 120;
			SDL_Rect fillRect = {computerX, computerY + 30, computerW, 120};
			SDL_Rect slotRect = {computerX, computerY, computerW, 40};
			SDL_Rect cursorRect;
			bool colliding = false;
			bool swipedLeft = false;
			bool swipedRight = false;
			
			SDL_StartTextInput();
			while( !quit )
			{
				SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0xFF, 0xFF );
				//Handle events on queue
				while( SDL_PollEvent( &e ) != 0 )
				{
					if( e.type == SDL_QUIT || e.key.keysym.sym == SDLK_ESCAPE )
					{
						quit = true;
					}

					if(title)
					{
						if(e.key.keysym.sym == SDLK_RETURN ||
						   e.key.keysym.sym == SDLK_KP_ENTER ||
						   e.key.keysym.sym == SDLK_0 ||
						   e.key.keysym.sym == SDLK_1 ||
						   e.key.keysym.sym == SDLK_2 ||
						   e.key.keysym.sym == SDLK_3 ||
						   e.key.keysym.sym == SDLK_4 ||
						   e.key.keysym.sym == SDLK_5 ||
						   e.key.keysym.sym == SDLK_6 ||
						   e.key.keysym.sym == SDLK_7 ||
						   e.key.keysym.sym == SDLK_8 ||
						   e.key.keysym.sym == SDLK_9)
						{
							if(tFade < SDL_GetTicks() && TITLE_STATE == TITLE_STATE_NEW)
							{
								tFade = SDL_GetTicks() + 1000;
								TITLE_STATE = TITLE_STATE_FADING;
							}
						}
					}
					else
					{
						if(e.key.keysym.sym == SDLK_KP_ENTER ||
						   e.key.keysym.sym == SDLK_RETURN)
						{
							if(!gCard.loaded && cursorPosition == 16)
							{
								createCard();
								creditCardNumber = "";
								cursorPosition = 0;
								tFade = SDL_GetTicks() + 500;
							}
						}

						if(e.key.keysym.sym == SDLK_BACKSPACE)
						{
							if(cursorPosition > 0 &&
							   SDL_GetTicks() > tBackspace)
							{
								cursorPosition -= 1;

								string newString = "";
								for(int i = 0;
									i < cursorPosition;
									++i)
								{
									newString += creditCardNumber[i];
								}
								creditCardNumber = newString;
								tBackspace = SDL_GetTicks() + 100;
								tCursor = SDL_GetTicks();
								//std::cout << creditCardNumber << std::endl;
							}
						}
					
						if(e.type == SDL_TEXTINPUT)
						{
							std::locale loc;
							if(isdigit(e.text.text[0], loc))
							{
								if(cursorPosition != 16)
								{
									creditCardNumber += e.text.text;
									//std::cout << creditCardNumber << std::endl;
									cursorPosition++;
									tCursor = SDL_GetTicks();
								}
							}
						}
					}
				}

				//Get leap motion input
				if(lmController.isConnected())
				{
					lmFrame = lmController.frame();
					lmHands = lmFrame.hands();
					Leap::Vector position = lmHands[0].palmPosition();

					if(lmFrame.hands().count() > 0)
					{   
						if(handsLastFrame)
						{
							int positionX = position.x;
							positionX = map(positionX, -200, 200, 0, SCREEN_WIDTH);
							lmDeltaX = positionX - lmX;
							lmDeltaX *= 2;
							lmX = positionX;

							int positionY = position.y;
							positionY = map(positionY, 200, 500, SCREEN_HEIGHT*2, 0);
							lmDeltaY = positionY - lmY;
							lmDeltaY *= 2;
							lmY = positionY;
						}
						else
						{
							int positionX = position.x;
							positionX = map(positionX, -200, 200, 0, SCREEN_WIDTH);
							lmDeltaX = positionX - lmX;
							lmX = positionX;

							int positionY = position.y;
							positionY = map(positionY, 0, 500, SCREEN_HEIGHT, 0);
							lmDeltaY = positionY - lmY;
							lmY = positionY;
						}
						handsLastFrame = true;
					
						std::stringstream ssX;
						std::stringstream ssY;
						std::stringstream ssZ;

						//ssX << lmX;
						//string str = ssX.str();
					
						ssY << lmDeltaX;
						string str =  ssY.str();

						//ssZ << position.z;
						//str += " ," + ssZ.str();

						debugText = "Y position: ";
				    	debugText += str;
					
					}
					else
					{
						lmDeltaX = 0;
						lmDeltaY = 0;
						handsLastFrame = true;
					}
					//debugText = str + " ";
				}

				if(title)
				{
					//UPDATE TITLE
					if(TITLE_STATE == TITLE_STATE_FADING)
					{
						if(tFade < SDL_GetTicks())
						{
							TITLE_STATE = TITLE_STATE_FADED;
							tFade = SDL_GetTicks() + 1500;
						}
					}
					else if(TITLE_STATE == TITLE_STATE_FADED && tFade < SDL_GetTicks())
					{
						title = false;
					}
				}
				else
				{
				//UPDATE GAME
				//int previousMouseX = mouseX;
				//int previousMouseY = mouseY;
				SDL_GetRelativeMouseState(&mouseDeltaX, &mouseDeltaY);

				//Card collisions with computer (fillRect)
				if(gCard.loaded)
				{
					int finalPositionX = gCard.x + lmDeltaX; //mouseDeltaX;
					int finalPositionY = gCard.y + lmDeltaY; //mouseDeltaY;

					colliding = true;
					int distanceBetweenCentersX = abs((fillRect.x + fillRect.w/2) -
													  (gCard.x + gCard.texture.getWidth()/2));
					int gCard_halfWidth =(gCard.texture.getWidth()/2) ;
					int fillRect_halfWidth = (fillRect.w/2);
					int overlapX = (gCard_halfWidth + fillRect_halfWidth) - distanceBetweenCentersX;
					if(overlapX <= 0)
					{
						colliding = false;
					}
					int distanceBetweenCentersY = abs((fillRect.y + fillRect.h/2) -
													  (gCard.y + gCard.texture.getHeight()/2));
					int gCard_halfHeight =(gCard.texture.getHeight()/2) ;
					int fillRect_halfHeight = (fillRect.h/2);
					int overlapY = (gCard_halfHeight + fillRect_halfHeight) - distanceBetweenCentersY;
					if(overlapY <= 0)
					{
						colliding = false;
					}

					if(colliding)
					{
						//Collision sound
						if(tCreditCardSwipeStart < SDL_GetTicks())
						{
							Mix_PlayChannel(-1, gCreditCardSwipeStart, 0);
							tCreditCardSwipeStart = SDL_GetTicks() + 500;
						}
						
						//Project out of collision
						if(overlapX > overlapY)
						{
							//Project across y axis
							if(gCard.y + gCard_halfHeight > fillRect.y + fillRect_halfHeight)
							{
								finalPositionY += overlapY;
							}
							else
							{
								finalPositionY -= overlapY;
							}
						}
						else
						{
							if(gCard.x + gCard_halfWidth > fillRect.x + fillRect_halfWidth)
							{
								finalPositionX += overlapX;
							}
							else
							{
								finalPositionX -= overlapX;
							}
						}
					}

					if(finalPositionX < 0)
					{
						finalPositionX = 0;
					}
					else if(finalPositionX + gCard.texture.getWidth() > SCREEN_WIDTH)
					{
						finalPositionX = SCREEN_WIDTH - gCard.texture.getWidth();
					}
					if(finalPositionY < 0)
					{
						finalPositionY = 0;
					}
					else if(finalPositionY + gCard.texture.getHeight() > SCREEN_HEIGHT)
					{
						finalPositionY = SCREEN_HEIGHT - gCard.texture.getHeight();
					}

					gCard.x = finalPositionX;
					gCard.y = finalPositionY;
				}

				//Card collisions with slotRect
				bool collidingWithSlot = true;
				swipedLeft = false;
				swipedRight = false;
				if(gCard.loaded)
				{
					int distanceBetweenCentersX = abs((slotRect.x + slotRect.w/2) -
													  (gCard.x + gCard.texture.getWidth()/2));
					int gCard_halfWidth =(gCard.texture.getWidth()/2) ;
					int slotRect_halfWidth = (slotRect.w/2);
					int overlapX = (gCard_halfWidth + slotRect_halfWidth) - distanceBetweenCentersX;
					if(overlapX <= 0)
					{
						collidingWithSlot = false;
					}
					int distanceBetweenCentersY = abs((slotRect.y + slotRect.h/2) -
													  (gCard.y + gCard.texture.getHeight()/2));
					int gCard_halfHeight =(gCard.texture.getHeight()/2) ;
					int slotRect_halfHeight = (slotRect.h/2);
					int overlapY = (gCard_halfHeight + slotRect_halfHeight) - distanceBetweenCentersY;
					if(overlapY <= 0)
					{
						collidingWithSlot = false;
					}

					if(collidingWithSlot)
					{
						if(abs(lmDeltaX) > 5 && canPlaySwipeMiddleSound)
						{
							Mix_PlayChannel(-1, gCreditCardSwipeMiddle, 0);
							canPlaySwipeMiddleSound = false;
							creditCardSwipeInProgress = true;
							tCreditCardSwipeDuration = SDL_GetTicks();
						}
					}
					else
					{
						canPlaySwipeMiddleSound = true;
						if(creditCardSwipeInProgress && lmDeltaX >= 20 &&
						   gCard.x > fillRect.x + fillRect.w &&
						   SDL_GetTicks() - tCreditCardSwipeDuration > 50)
						{
						    swipedRight = true;
							creditCardSwipeInProgress = false;
						}
						if(creditCardSwipeInProgress && lmDeltaX <= -20 &&
						   gCard.x < fillRect.x &&
						   SDL_GetTicks() - tCreditCardSwipeDuration > 50)
						{
						    swipedLeft = true;
							creditCardSwipeInProgress = false;
						}
						if(creditCardSwipeInProgress && lmDeltaX < 20)
						{
							creditCardSwipeInProgress = false;
						}
					}
					
				}

				if(swipedRight || swipedLeft)
				{
					if(vpe.pixels == NULL)
					{
						gCard.texture.lockTexture();
					    vpe.texture.lockTexture();
						
						Uint32* cardPixels = ((Uint32* )gCard.texture.getPixels());
						Uint32* emptyPixels = ((Uint32* )vpe.texture.getPixels());
						int pixelCount = gCard.texture.getWidth() * gCard.texture.getHeight();
					    vpe.pixels = new pixel [pixelCount];
						//int offset = (vpe.texture.getWidth()) -
						//	((gCardTexture.getWidth()) % vpe.texture.getWidth());
						v2 destPixel = {0, 0};
						for(int y = 0;
							y < gCard.texture.getHeight();
							++y)
						{
							destPixel.y = gCard.y + y;
							for(int x = 0;
								x < gCard.texture.getWidth();
								++x)
							{
								destPixel.x = gCard.x + x;
								int cardPixelIndex = x + (y *  gCard.texture.getWidth());
								int emptyPixelIndex = destPixel.x + (destPixel.y * vpe.texture.getWidth());
								emptyPixels[emptyPixelIndex] = cardPixels[cardPixelIndex];
								vpe.pixels[cardPixelIndex].x = destPixel.x;
							    vpe.pixels[cardPixelIndex].y = destPixel.y;
								vpe.pixels[cardPixelIndex].ix = destPixel.x;
							    vpe.pixels[cardPixelIndex].iy = destPixel.y;
								vpe.pixels[cardPixelIndex].color = (Uint32)cardPixels[cardPixelIndex];
								vpe.pixels[cardPixelIndex].xSpeed = lmDeltaX;
								vpe.pixels[cardPixelIndex].ySpeed = 0;
							}
						}
						
						vpe.radiationLine = (gCard.texture.getHeight() / 2) + gCard.y;
						vpe.pixelCount = pixelCount;
						vpe.originX = gCard.x + gCard.texture.getWidth();
						vpe.originY = gCard.y + (gCard.texture.getHeight() / 2);
						vpe.initialSpeedX = lmDeltaX;
						vpe.step = 0;
						if(swipedLeft)
						{
							vpe.direction = -1;
						}
						else
						{
							vpe.direction = 1;
						}
						
						gCard.texture.unlockTexture();
					    vpe.texture.unlockTexture();
						gCard.loaded = false;
						tFontWait = SDL_GetTicks() + 1000;
						tFade = SDL_GetTicks() + 1500;
					}
				};
				if(vpe.pixels != NULL)
				{
					explodePixelsVertically();
				}

				//Update credit card number
				string finalCreditCardNumber = "";
				for(int i = 0;
					i < 4;
					++i)
				{
					finalCreditCardNumber += creditCardNumber[i];
					if(i != 3)
					{
						finalCreditCardNumber += "-";
					}
				}

				if(cursorPosition < 16)
				{
					cursorRect.x = ((SCREEN_WIDTH - gCreditCardNumberTexture.getWidth()) / 2) + 2 +
						(cursorPosition * 34);
					cursorRect.y =  50;
					cursorRect.w = 30;
					cursorRect.h = 50;
					cursorRect.x += (int)(cursorPosition / 4) * 28;
				}
				else
				{
					cursorPosition = 16;
					cursorRect.x = ((SCREEN_WIDTH - gCreditCardNumberTexture.getWidth()) / 2) + 2 +
						(cursorPosition * 34);
					cursorRect.y =  50;
					cursorRect.w = 30;
					cursorRect.h = 50;
					cursorRect.x += (2 * 28) + 28;
				}

				//Fade text in and out
				if(gCard.loaded)
				{
					if(tFade > SDL_GetTicks())
					{
						int alpha = (int)((float)(tFade - SDL_GetTicks()) / (1000/255));
						gPromptTexture.setAlpha(alpha);
						gCreditCardNumberTexture.setAlpha(alpha);
					}
				}
				else
				{
					int alpha = 255;
					if(tFontWait > SDL_GetTicks())
					{
						alpha = 0;
					}
					else if(tFade > SDL_GetTicks())
					{
						alpha = 255 - (int)((float)(tFade - SDL_GetTicks()) / (1000/255));
					}
					
					gPromptTexture.setAlpha(alpha);
					gCreditCardNumberTexture.setAlpha(alpha);
				}

				}
				
				//RENDERING
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderClear( gRenderer );

				gBackgroundTexture.renderEx(gRenderer, 0, 0);
				gWhiteGradientTexture.renderEx(gRenderer, 0, 0);

				if(gCard.loaded)
				{
					gCard.texture.renderEx(gRenderer, gCard.x, gCard.y);
				}

				vpe.texture.renderEx(gRenderer, 0, 0);
			    
				gComputerOverlayTexture.renderEx(gRenderer, 0, 0);

				//RENDER TEXT

				//Draw cursor
				if(title && (TITLE_STATE == TITLE_STATE_NEW ||
							 TITLE_STATE == TITLE_STATE_FADING))
				{
					gTitleTexture.renderEx(gRenderer, (SCREEN_WIDTH - gTitleTexture.getWidth()) / 2, 200);
					if(tFade > SDL_GetTicks())
					{
						int alpha = 0;//(int)(tFade - SDL_GetTicks()) / (1500/254);
						gTitleTexture.setAlpha(alpha);
					}
				}
				else if(!title)
				{
					SDL_SetRenderDrawColor(gRenderer, 0xff, 0xff, 0xff, 0xff);
					if((SDL_GetTicks() - tCursor) % 1000 < 500 && !gCard.loaded && tFontWait < SDL_GetTicks())
					{
						SDL_RenderFillRect(gRenderer, &cursorRect);		
					}

					gPromptTexture.renderEx(gRenderer, (SCREEN_WIDTH - gPromptTexture.getWidth()) / 2, 20);
			    
					gCreditCardNumberTexture.renderEx(gRenderer,
													  (SCREEN_WIDTH - gCreditCardNumberTexture.getWidth()) / 2, 50);
				
					if(cursorPosition == 16)
					{
						gPressEnterTexture.renderEx(gRenderer, (SCREEN_WIDTH - gPressEnterTexture.getWidth()) / 2, 120);
					}
				
					//Render collision boxes for debug
					//SDL_SetRenderDrawColor(gRenderer, 0xff, 0x00, 0xff, 0xff);
					//SDL_RenderFillRect(gRenderer, &fillRect);
					//SDL_SetRenderDrawColor(gRenderer, 0xff, 0xff, 0x00, 0xff);
					//SDL_RenderFillRect(gRenderer, &slotRect);
				}

				//Text texture for debugging the leap motion
				//SDL_Color textColor = {255, 255, 255};
				//debugTextTexture.loadFromRenderedText(debugText,
				//									  gRenderer,
				//									  gDebugFont,
				//									  textColor);
				//debugTextTexture.renderEx(gRenderer, 100, 100);
				
				//Update Screen
				SDL_RenderPresent( gRenderer );
			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}

/* Reference 
  Uint32* pixelsx = (Uint32*)gFooTexture.getPixels();
  int pixelCount = ( gFooTexture.getPitch() / 4 ) * gFooTexture.getHeight();
  pixel = *((Uint32 *)gFooTexture.getPixels());
  finalColor = blue | (green << 8) | (red << 16) | (alpha << 24);
*/
