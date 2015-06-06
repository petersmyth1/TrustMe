#ifndef LTEXTURE_H
#define LTEXTURE_H

class LTexture
{
	public:
		//Initializes variables
		LTexture();

		//Deallocates memory
		~LTexture();

		//Create an empty texture
		bool createTexture(SDL_Renderer* renderer, int width, int height);
		
		//Loads image at specified path
		bool loadFromFileBMP( std::string path, SDL_Renderer* renderer, SDL_Window* window );
		bool loadFromFileIMG( std::string path, SDL_Renderer* renderer, SDL_Window* window );
		
		#ifdef _SDL_TTF_H
		//Creates image from font string
		bool loadFromRenderedText( std::string textureText, SDL_Renderer* renderer, TTF_Font* font, SDL_Color textColor );
		#endif

		//Deallocates texture
		void free();

		//Set color modulation
		void setColor( Uint8 red, Uint8 green, Uint8 blue );

		//Set blending
		void setBlendMode( SDL_BlendMode blending );

		//Set alpha modulation
		void setAlpha( Uint8 alpha );

		//General render
		void render( SDL_Renderer* renderer, SDL_Rect* sourceRect = NULL, SDL_Rect* destRect = NULL);
		
		//Renders texture at given point
		void renderEx( SDL_Renderer* renderer, int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );

		//Gets image dimensions
		int getWidth();
		int getHeight();

		//Pixel manipulators
		bool lockTexture();
		bool unlockTexture();

		//Pixel accessors
		void* getPixels();
		int getPitch();
		SDL_PixelFormat* getPixelFormat();
		Uint8 getPixelRed( Uint32 pixel );
		Uint8 getPixelGreen( Uint32 pixel );
		Uint8 getPixelBlue( Uint32 pixel );
		Uint8 getPixelAlpha( Uint32 pixel );
		void getPixelValues( Uint8* pixelArrayPointer, Uint32 pixel );

		void clearTextureToColor(Uint32 color);
		
	private:
		//The actual hardware texture
		SDL_Texture* mTexture;
		void* mPixels;
		int mPitch;
		SDL_PixelFormat* mPixelFormat;

		//Keep surface for pixelFormat I guess
		SDL_Surface* mSurface;

		//Image dimensions
		int mWidth;
		int mHeight;
};


SDL_Surface* loadSurfaceIMG( std::string path )
{
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if(loadedSurface == NULL)
	{
		printf("Unable to load image %s. SDL Error: %s\n",
			   path.c_str(), IMG_GetError());
	}

	return loadedSurface;
}

SDL_Surface* loadSurfaceBMP( std::string path )
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

bool LTexture::createTexture(SDL_Renderer* renderer, int width, int height)
{
	//Get rid of preexisting texture
	free();

	//Final texture
	SDL_Texture* newTexture = NULL;
	
	//Create blank streamable texture
	newTexture = SDL_CreateTexture( renderer,
								    SDL_PIXELFORMAT_ARGB8888,
									SDL_TEXTUREACCESS_STREAMING,
								    width,
									height);
		
	if( newTexture == NULL )
	{
		printf( "Unable to create blank texture! SDL Error: %s\n",
				SDL_GetError() );
	}
	else
	{	
		//Get image dimensions
		mWidth = width;
		mHeight = height;
	}
	
	
	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

bool LTexture::loadFromFileBMP( std::string path, SDL_Renderer* renderer, SDL_Window* window )
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	mSurface = loadSurfaceBMP( path );
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

bool LTexture::loadFromFileIMG( std::string path, SDL_Renderer* renderer, SDL_Window* window )
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	mSurface = loadSurfaceIMG( path );
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

	setBlendMode(SDL_BLENDMODE_BLEND);
	
	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

#ifdef _SDL_TTF_H
bool LTexture::loadFromRenderedText( std::string textureText,  SDL_Renderer* renderer, TTF_Font* font, SDL_Color textColor )
{
	//Get rid of preexisting texture
	free();

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Blended( font, textureText.c_str(), textColor );
	if( textSurface != NULL )
	{
		//Create texture from surface pixels
        mTexture = SDL_CreateTextureFromSurface( renderer, textSurface );
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

void LTexture::setBlendMode( SDL_BlendMode blendMode )
{
	//Set blending function
	SDL_SetTextureBlendMode( mTexture, blendMode );
}
		
void LTexture::setAlpha( Uint8 alpha )
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod( mTexture, alpha );
}

void LTexture::render( SDL_Renderer* renderer, SDL_Rect* sourceRect, SDL_Rect* destRect )
{
	//Turn on alpha blending
    setBlendMode(SDL_BLENDMODE_BLEND);

	SDL_RenderCopy( renderer, mTexture, sourceRect, destRect );
}

void LTexture::renderEx( SDL_Renderer* renderer, int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip )
{
	//Turn on alpha blending
	setBlendMode(SDL_BLENDMODE_BLEND);
	
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

Uint8 LTexture::getPixelRed( Uint32 pixel )
{
	SDL_PixelFormat* fmt = getPixelFormat();
	Uint32 temp;
	temp = pixel & fmt->Rmask;
	temp = temp >> fmt->Rshift;
	temp = temp << fmt->Rloss;
	return (Uint8)temp;
}

Uint8 LTexture::getPixelGreen( Uint32 pixel )
{
	SDL_PixelFormat* fmt = getPixelFormat();
	Uint32 temp;
	temp = pixel & fmt->Gmask;
	temp = temp >> fmt->Gshift;
	temp = temp << fmt->Gloss;
	return (Uint8)temp;
}

Uint8 LTexture::getPixelBlue( Uint32 pixel )
{
	SDL_PixelFormat* fmt = getPixelFormat();
	Uint32 temp;
	temp = pixel & fmt->Bmask;
	temp = temp >> fmt->Bshift;
	temp = temp << fmt->Bloss;
	return (Uint8)temp;
}

Uint8 LTexture::getPixelAlpha( Uint32 pixel )
{
	SDL_PixelFormat* fmt = getPixelFormat();
	Uint32 temp;
	temp = pixel & fmt->Amask;
	temp = temp >> fmt->Ashift;
	temp = temp << fmt->Aloss;
	return (Uint8)temp;
}

void LTexture::getPixelValues( Uint8* pixelArrayPointer, Uint32 pixel )
{
	pixelArrayPointer[0] = getPixelRed(pixel);
	pixelArrayPointer[1] = getPixelGreen(pixel);
	pixelArrayPointer[2] = getPixelBlue(pixel);
    pixelArrayPointer[3] = getPixelAlpha(pixel);
}

void LTexture::clearTextureToColor(Uint32 color)
{
	Uint32* pixels = (Uint32* )getPixels();
	
	for(int i = 0;
		i < getWidth() * getHeight();
		++i)
	{
		pixels[i] = color;
	}
}

#endif
