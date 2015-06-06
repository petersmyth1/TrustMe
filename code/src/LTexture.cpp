//LTexture .cpp file

//I don't know if I should have this file?????

#include "LTexture.h"

SDL_Surface* loadSurface( std::string path )
{
	SDL_Surface* loadedSurface = SDL_LoadBMP(path.c_str());
	if(loadedSurface == NULL)
	{
		printf("Unable to load image %s. SDL Error: %s\n",
			   path.c_str(), SDL_GetError());
	}

	return loadedSurface;
}

LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
	mPixels = NULL;
	mPitch = 0;
	mPixelFormat = 0;
	mSurface = NULL;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile( std::string path, SDL_Renderer* renderer, SDL_Window* window )
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	mSurface = loadSurface( path );
	if( mSurface == NULL )
	{
		printf( "Unable to load image %s! SDL_image Error: %s\n",
				path.c_str(), SDL_GetError() );
	}
	else
	{
		mPixelFormat = mSurface->format;
		
	   	//Create blank streamable texture
   		newTexture = SDL_CreateTexture( renderer,
		   								SDL_GetWindowPixelFormat( window ),
	   									SDL_TEXTUREACCESS_STREAMING,
   										mSurface->w,
	   								    mSurface->h );
		
		if( newTexture == NULL )
	   	{
   			printf( "Unable to create blank texture! SDL Error: %s\n",
		   			SDL_GetError() );
	   	}
   		else
		{
	   		//Lock texture for manipulation
   			SDL_LockTexture( newTexture, NULL, &mPixels, &mPitch );

		   	//Copy loaded/formatted surface pixels
	   		memcpy( mPixels, mSurface->pixels,
						mSurface->pitch * mSurface->h);

    		//Unlock texture to update
		   	SDL_UnlockTexture( newTexture );
	   		mPixels = NULL;

		   	//Get image dimensions
	   		mWidth = mSurface->w;
   			mHeight = mSurface->h;
		}
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

#ifdef _SDL_TTF_H
bool LTexture::loadFromRenderedText( std::string textureText, SDL_Color textColor )
{
	//Get rid of preexisting texture
	free();

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid( gFont, textureText.c_str(), textColor );
	if( textSurface != NULL )
	{
		//Create texture from surface pixels
        mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
		if( mTexture == NULL )
		{
			printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}

		//Get rid of old surface
		SDL_FreeSurface( textSurface );
	}
	else
	{
		printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
	}

	
	//Return success
	return mTexture != NULL;
}
#endif

void LTexture::free()
{
	//Free texture if it exists
	if( mTexture != NULL )
	{
		SDL_DestroyTexture( mTexture );
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
		mPixels = NULL;
		mPitch = 0;
	}
}

void LTexture::setColor( Uint8 red, Uint8 green, Uint8 blue )
{
	//Modulate texture rgb
	SDL_SetTextureColorMod( mTexture, red, green, blue );
}

void LTexture::setBlendMode( SDL_BlendMode blending )
{
	//Set blending function
	SDL_SetTextureBlendMode( mTexture, blending );
}
		
void LTexture::setAlpha( Uint8 alpha )
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod( mTexture, alpha );
}

void LTexture::render( SDL_Renderer* renderer, int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip )
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if( clip != NULL )
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx( renderer, mTexture, clip, &renderQuad, angle, center, flip );
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

bool LTexture::lockTexture()
{
	bool success = true;

	//Texture is already locked
	if( mPixels != NULL )
	{
		printf( "Texture is already locked!\n" );
		success = false;
	}
	//Lock texture
	else
	{
		if( SDL_LockTexture( mTexture, NULL, &mPixels, &mPitch ) != 0 )
		{
			printf( "Unable to lock texture! %s\n", SDL_GetError() );
			success = false;
		}
	}

	return success;
}

bool LTexture::unlockTexture()
{
	bool success = true;

	//Texture is not locked
	if( mPixels == NULL )
	{
		printf( "Texture is not locked!\n" );
		success = false;
	}
	//Unlock texture
	else
	{
		SDL_UnlockTexture( mTexture );
		mPixels = NULL;
		mPitch = 0;
	}

	return success;
}

void* LTexture::getPixels()
{
	return mPixels;
}

int LTexture::getPitch()
{
	return mPitch;
}

SDL_PixelFormat* LTexture::getPixelFormat()
{
	return mPixelFormat;
}
