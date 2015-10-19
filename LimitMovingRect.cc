#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <string>

using namespace std;

//Motion changer function
void loadKeyStat(void);
void changeDirection(int* vectorDirect);
void moveUpward(void);
void moveDownward(void);
void moveLeftward(void);
void moveRightward(void);
bool LimitInitialiseAll(void);
void LimitUpdateAll(void);
void lineJumpRect(int* directVector);
void inclinedJumpRect(int* directVector, char inclinePhase);
int multiKeyPressHandler(void);
void duckPositionRect(void);
void rollDuckedRect(char rollSide);
void mapKeyStatToKeyTable(void);
void loadImageToTexture(std::string pathImage);

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int incrementer = 1;

//The window we'll be rendering to
SDL_Window* window = NULL;

//The surface contained by the window
SDL_Surface* screen = NULL;

//The screen Renderer
SDL_Renderer* screenRenderer = NULL;

//Texture wrapper class
class LTexture
{
	public:
		//Initializes variables
		LTexture();

		//Deallocates memory
		~LTexture();

		//Loads image at specified path
		bool loadFromFile( std::string path );

		//Deallocates texture
		void free();

		//Renders texture at given point
		void render( int x, int y, SDL_Rect* clip = NULL );

		//Gets image dimensions
		int getWidth();
		int getHeight();

	private:
		//The actual hardware texture
		SDL_Texture* mTexture;

		//Image dimensions
		int mWidth;
		int mHeight;
};

LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile( std::string path )
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	screen = IMG_Load( path.c_str() );
	if( loadedSurface == NULL )
	{
		printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
	}
	else
	{
		//Color key image
		SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0xFF, 0xFF ) );

		//Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( screenRenderer, loadedSurface );
		if( newTexture == NULL )
		{
			printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		//SDL_FreeSurface( loadedSurface );
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

void LTexture::free()
{
	//Free texture if it exists
	if( mTexture != NULL )
	{
		SDL_DestroyTexture( mTexture );
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::render( int x, int y, SDL_Rect* clip )
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
	SDL_RenderCopy( screenRenderer, mTexture, clip, &renderQuad );
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}



//The texture for caveman
//LTexture* playerTexture = NULL;

//The texture for background
LTexture* screenTexture;

//The image Rectangle for png file
SDL_Rect* imageRect = NULL;

//The player Rectangle on screen
SDL_Rect* playerRect = NULL;

//The image rectangle for scree
SDL_Rect* screenImageRect = NULL;

//keyboard statistics table
const Uint8* keyStat;

//Color variables
Uint32 color;
Uint32 color2;

//Duck down variable
bool isDuck = false;

//Roll key status
bool rollKeyDown = false;

//Duck key status
bool duckKeyDown = false;

//motion diimageRection variable
char motionFace = '\0';

//Type of key press
enum keyTypePressed
{
	KEY_PRESS_SURFACE_DEFAULT,
	KEY_PRESS_SURFACE_UP,
	KEY_PRESS_SURFACE_DOWN,
	KEY_PRESS_SURFACE_LEFT,
	KEY_PRESS_SURFACE_RIGHT,
	KEY_PRESS_SURFACE_SPACE,
	KEY_PRESS_SURFACE_RCTRL,
	KEY_PRESS_SURFACE_R,
	KEY_PRESS_SURFACE_TOTAL
};

//Multi key press array
bool keyPressTable[KEY_PRESS_SURFACE_TOTAL];

//Key press surfaces constants
enum KeyPressSurfaces
{
	DIRECT_VECTOR_SURFACE_DEFAULT,
	DIRECT_VECTOR_SURFACE_UP,
	DIRECT_VECTOR_SURFACE_DOWN,
	DIRECT_VECTOR_SURFACE_LEFT,
	DIRECT_VECTOR_SURFACE_RIGHT,
	DIRECT_VECTOR_SURFACE_JUMP,
	DIRECT_VECTOR_SURFACE_JUMPL,
	DIRECT_VECTOR_SURFACE_JUMPR,
	DIRECT_VECTOR_SURFACE_DUCK,
	DIRECT_VECTOR_SURFACE_LROLL,
	DIRECT_VECTOR_SURFACE_RROLL	
};

int main( int argc, char* args[] )
{
	int keyDirectValue = 0;
	bool running = true;
	const int FPS = 30;
        Uint32 start;
	keyStat = SDL_GetKeyboardState(NULL);

	if(LimitInitialiseAll() == 0)
	{
		cout << "Terminate due to above Errors\n";
		return 0;
	}
	else
	{
        	color = SDL_MapRGB(screen->format, 0xff,0xff,0xff);
        	color2 = SDL_MapRGB(screen->format, 0,0,0);
	   
        	while(running) 
		{
        		start = SDL_GetTicks();	
                	if(SDL_QuitRequested())
			{
                	       	running = false;
                	        continue;                                		
                	}

			loadKeyStat();
			keyDirectValue = multiKeyPressHandler();
			changeDirection(&keyDirectValue);		
			LimitUpdateAll();
			//SDL_Delay(1);
	
                	if(1000/FPS > SDL_GetTicks()-start) 
			{
                		SDL_Delay((1000/FPS)-(SDL_GetTicks()-start));
                	}
		}	
		//Destroy window
		SDL_DestroyWindow( window );
	
		//Quit SDL subsystems
		SDL_Quit();

		return 0;
	}
}

void loadKeyStat(void)
{
	SDL_PumpEvents();	
	mapKeyStatToKeyTable();
}

void changeDirection(int* vectorDirect)
{
	switch(*vectorDirect)
	{
		case DIRECT_VECTOR_SURFACE_UP:
		{
			moveUpward();
			break;
		}

		case DIRECT_VECTOR_SURFACE_DOWN:
		{
			moveDownward();
			break;
		}
		
		case DIRECT_VECTOR_SURFACE_LEFT:
		{
			moveLeftward();
			break;
		}

		case DIRECT_VECTOR_SURFACE_RIGHT:
		{
			moveRightward();
			break;
		}
		
		case DIRECT_VECTOR_SURFACE_JUMP:
		{
			lineJumpRect(vectorDirect);
			break;
		}

		case DIRECT_VECTOR_SURFACE_JUMPL:
		{
			inclinedJumpRect(vectorDirect, 'l');
			break;
		}

		case DIRECT_VECTOR_SURFACE_JUMPR:
		{
			inclinedJumpRect(vectorDirect, 'r');
			break;
		}

		case DIRECT_VECTOR_SURFACE_DUCK:
		{
			duckPositionRect();
			break;
		}

		case DIRECT_VECTOR_SURFACE_LROLL:
		{
			rollDuckedRect('l');
			break;
		}

		case DIRECT_VECTOR_SURFACE_RROLL:
		{
			rollDuckedRect('r');
		}
	
		case DIRECT_VECTOR_SURFACE_DEFAULT:
		{
			break;
		}
	}	
}

void moveUpward(void)
{
	if((imageRect->y/* + imageRect->h*/) > 0)
	{
		imageRect->y -= incrementer;
	}
	else
	{
		if((imageRect->x/* + imageRect->w*/) > 0)
		{
			imageRect->x -= imageRect->w;
			imageRect->y = SCREEN_HEIGHT;
		}
		else
		{
			imageRect->y = SCREEN_HEIGHT - imageRect->h;
			imageRect->x = SCREEN_WIDTH - imageRect->w;
		}
	}
}

void moveDownward(void)
{
	if((imageRect->y + imageRect->h) < SCREEN_HEIGHT)
	{
		imageRect->y += incrementer;
	}
	else
	{
		if((imageRect->x + imageRect->w) < SCREEN_WIDTH)
		{
			imageRect->x += imageRect->w;
			imageRect->y = 0;
		}
		else
		{
			imageRect->y = 0;
			imageRect->x = 0;
		}
	}
}

void moveLeftward(void)
{
	if((imageRect->x/* + imageRect->w*/) > 0)
	{
		imageRect->x -= incrementer;
	}
	else
	{
		if((imageRect->y/* + imageRect->h*/) > imageRect->h)
		{
			imageRect->x = SCREEN_WIDTH - imageRect->w;
			imageRect->y -= imageRect->h;
		}
		else
		{
			imageRect->y = SCREEN_HEIGHT - imageRect->h;
			imageRect->x = SCREEN_WIDTH - imageRect->w;
		}
	}
}

void moveRightward(void)
{
	if((imageRect->x + imageRect->w) < SCREEN_WIDTH)
	{
		imageRect->x += incrementer;
	}
	else
	{
		if((imageRect->y + imageRect->h) < SCREEN_HEIGHT)
		{
			imageRect->x = 0;
			imageRect->y += imageRect->h;
		}
		else
		{
			imageRect->y = 0;
			imageRect->x = 0;
		}
	}
}

bool LimitInitialiseAll(void)
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
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
		window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( window == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//Create renderer for window
			screenRenderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
			if( screenRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor( screenRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if( !( IMG_Init( imgFlags ) & imgFlags ) )
				{
					printf( "SDL_image could not initialize! SDL_mage Error: %s\n", IMG_GetError() );
					success = false;
				}
				else
				{
					//SDL_FillRect( screen, NULL, SDL_MapRGB( screen->format, 0xFF, 0xFF, 0xFF ) );
					imageRect = new SDL_Rect;
				
					imageRect->x = 0;
					imageRect->y = 0;
					imageRect->w = 20;
					imageRect->h = 20;
				}
			}
		}
	}

	return success;
}
/*
int LimitInitialiseAll(void)
{
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
		return 0;
	}
	else
	{
		//Create window
		window = SDL_CreateWindow( "Limit Moving Rect", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( window == NULL )
		{
			cout << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
			return 0;
		}
		else
		{
			//Get window surface
			screen = SDL_GetWindowSurface( window );
			if(screen == NULL)
			{
				cout << "Screen could not be generated ! SDL Error : " << SDL_GetError() << endl;
				return 0;
			}
			else
			{
				SDL_FillRect( screen, NULL, SDL_MapRGB( screen->format, 0xFF, 0xFF, 0xFF ) );
				imageRect = new SDL_Rect;
				
				imageRect->x = 0;
				imageRect->y = 0;
				imageRect->w = 20;
				imageRect->h = 20;

        			return 1;
			}
		}
	}
}
*/
void LimitUpdateAll(void)
{
	/*SDL_FillRect(screen, &screen->clip_imageRect, color);
	SDL_FillRect(screen, imageRect, color2);
	SDL_UpdateWindowSurface( window );*/
	screenTexture->loadFromFile(RectOperations/prehistoric.png);//SDL_RenderCopy(screenRenderer, screenTexture, surfaceRect->clip_imageRect, surfaceRect->clip_imageRect);
	screenTexture->render(0, 0, screen->clip_rect);
	SDL_RenderCopy(screenRenderer, screenTexture, imageRect, screenImageRect);
	SDL_RenderPresent(screenRenderer);
}

/*void loadImageToTexture(std::string pathImage)
{

}*/

void lineJumpRect(int* directVector)
{
	bool running = true;
	int yPositionRect = imageRect->y;
	
	if(yPositionRect > imageRect->h)
	{
		(*directVector) = DIRECT_VECTOR_SURFACE_UP;
		while(running)
		{
			changeDiimageRection(directVector);
			if(imageRect->y == (yPositionRect - imageRect->h))
			{
				(*directVector) = DIRECT_VECTOR_SURFACE_DOWN;
			}
			else if(imageRect->y == yPositionRect)
			{
				running = false;
			}
			LimitUpdateAll();
			SDL_Delay(5);
		}
	}
}

void inclinedJumpRect(int* directVector, char inclinePhase)
{
	bool running = true;
	int yPositionRect = imageRect->y;
	int xPositionRect = imageRect->x;
	bool UP = true;

	if(yPositionRect > imageRect->h)
	{
		switch(inclinePhase)
		{
			case 'l':
			{
				while(running)
				{
					if(xPositionRect > (2*(imageRect->w)))
					{
						(*directVector) = DIRECT_VECTOR_SURFACE_LEFT;
						changeDiimageRection(directVector);
						LimitUpdateAll();
						if((imageRect->x == (xPositionRect - (2*(imageRect->w)))))
						{
							running = false;
						}
						else if(!UP)
						{
							(*directVector) = DIRECT_VECTOR_SURFACE_DOWN;
							changeDiimageRection(directVector);
							LimitUpdateAll();
						}
						else
						{
							(*directVector) = DIRECT_VECTOR_SURFACE_UP;
							changeDiimageRection(directVector);
							LimitUpdateAll();
							if(imageRect->y == (yPositionRect - imageRect->h))
							{
								UP = false;
							}
						}
						SDL_Delay(5);
					}
					else
					{
						running = false;
					}
				}
			}
			break;

			case 'r':
			{
				while(running)
				{
					if(xPositionRect < (SCREEN_WIDTH - (2*(imageRect->w))))
					{
						(*directVector) = DIRECT_VECTOR_SURFACE_RIGHT;
						changeDiimageRection(directVector);
						LimitUpdateAll();
						if((imageRect->x == (xPositionRect + (2*(imageRect->w)))))
						{
							running = false;
						}
						else if(!UP)
						{
							(*directVector) = DIRECT_VECTOR_SURFACE_DOWN;
							changeDiimageRection(directVector);
							LimitUpdateAll();
						}
						else
						{
							(*directVector) = DIRECT_VECTOR_SURFACE_UP;
							changeDiimageRection(directVector);
							LimitUpdateAll();
							if(imageRect->y == (yPositionRect - imageRect->h))
							{
								UP = false;
							}
						}
						SDL_Delay(5);
					}
					else
					{
						running = false;
					}
				}
			}
			break;
		}
	}
}

int multiKeyPressHandler(void)
{
	if((keyPressTable[KEY_PRESS_SURFACE_SPACE]) & (keyPressTable[KEY_PRESS_SURFACE_LEFT]) & !isDuck)
	{
		keyPressTable[KEY_PRESS_SURFACE_SPACE] = false;
		keyPressTable[KEY_PRESS_SURFACE_LEFT] = false;
		return DIRECT_VECTOR_SURFACE_JUMPL;
	}
	else if((keyPressTable[KEY_PRESS_SURFACE_SPACE]) & (keyPressTable[KEY_PRESS_SURFACE_RIGHT]) & !isDuck)
	{
		keyPressTable[KEY_PRESS_SURFACE_SPACE] = false;
		keyPressTable[KEY_PRESS_SURFACE_RIGHT] = false;
		return DIRECT_VECTOR_SURFACE_JUMPR;
	}
	else if((keyPressTable[KEY_PRESS_SURFACE_R]) & (motionFace == 'l') & isDuck)
	{
		keyPressTable[KEY_PRESS_SURFACE_R] = false;
		return DIRECT_VECTOR_SURFACE_LROLL;
	}
	else if((keyPressTable[KEY_PRESS_SURFACE_R]) & (motionFace == 'r') & isDuck)
	{
		keyPressTable[KEY_PRESS_SURFACE_R] = false;
		return DIRECT_VECTOR_SURFACE_RROLL;
	}
	else if(keyPressTable[KEY_PRESS_SURFACE_SPACE] & !isDuck)
	{
		keyPressTable[KEY_PRESS_SURFACE_SPACE] = false;
		return DIRECT_VECTOR_SURFACE_JUMP;
	}
	else if(keyPressTable[KEY_PRESS_SURFACE_RCTRL])
	{
		keyPressTable[KEY_PRESS_SURFACE_RCTRL] = false;
		return DIRECT_VECTOR_SURFACE_DUCK;
	}
	else if(keyPressTable[KEY_PRESS_SURFACE_LEFT])
	{
		keyPressTable[KEY_PRESS_SURFACE_LEFT] = false;
		return DIRECT_VECTOR_SURFACE_LEFT;
	}
	else if(keyPressTable[KEY_PRESS_SURFACE_RIGHT])
	{
		keyPressTable[KEY_PRESS_SURFACE_RIGHT] = false;
		return DIRECT_VECTOR_SURFACE_RIGHT;
	}
	else if(keyPressTable[KEY_PRESS_SURFACE_UP])
	{
		keyPressTable[KEY_PRESS_SURFACE_UP] = false;
		return DIRECT_VECTOR_SURFACE_UP;
	}
	else if(keyPressTable[KEY_PRESS_SURFACE_DOWN])
	{
		//keyPressTable[KEY_PRESS_SURFACE_DOWN] = false;
		return DIRECT_VECTOR_SURFACE_DOWN;
	}
	else
	{
		//keyPressTable[KEY_PRESS_SURFACE_DEFAULT] = false;
		return DIRECT_VECTOR_SURFACE_DEFAULT;
	}
}

void duckPositionRect(void)
{
	if(!isDuck)
	{
		imageRect->y += ((imageRect->h)/2);
		imageRect->h = ((imageRect->h)/2);
		isDuck = true;
	}
	else
	{
		imageRect->y -= imageRect->h;
		imageRect->h += imageRect->h;
		isDuck = false;
	}
}

void rollDuckedRect(char rollSide)
{
	if(((imageRect->x + imageRect->w) <= (SCREEN_WIDTH - (2*(imageRect->w)))) && isDuck)
	{	
		switch(rollSide)
		{
			case 'r':
			{
				
				imageRect->x = (imageRect->x + imageRect->w - imageRect->h);
				imageRect->y = (imageRect->y + imageRect->h - imageRect->w);
				
				imageRect->w += imageRect->h;
				imageRect->h = imageRect->w - imageRect->h;
				imageRect->w = imageRect->w - imageRect->h;
				LimitUpdateAll();
				SDL_Delay(10);
				imageRect->x = (imageRect->x + imageRect->w);
				imageRect->y = (imageRect->y + imageRect->h - imageRect->w);

				imageRect->w += imageRect->h;
				imageRect->h = imageRect->w - imageRect->h;
				imageRect->w = imageRect->w - imageRect->h;
				LimitUpdateAll();
				SDL_Delay(10);
				imageRect->x = (imageRect->x + imageRect->w - imageRect->h);
				imageRect->y = (imageRect->y + imageRect->h - imageRect->w);
				
				imageRect->w += imageRect->h;
				imageRect->h = imageRect->w - imageRect->h;
				imageRect->w = imageRect->w - imageRect->h;
				LimitUpdateAll();
				SDL_Delay(10);
				imageRect->x = (imageRect->x + imageRect->w);
				imageRect->y = (imageRect->y + imageRect->h - imageRect->w);

				imageRect->w += imageRect->h;
				imageRect->h = imageRect->w - imageRect->h;
				imageRect->w = imageRect->w - imageRect->h;
				LimitUpdateAll();
				SDL_Delay(10);
			}
			break;
		
			case 'l':
			{
				imageRect->x = (imageRect->x - imageRect->w + imageRect->h);
				imageRect->y = (imageRect->y + imageRect->h - imageRect->w);
				
				imageRect->w += imageRect->h;
				imageRect->h = imageRect->w - imageRect->h;
				imageRect->w = imageRect->w - imageRect->h;
				LimitUpdateAll();
				SDL_Delay(10);
				imageRect->x = (imageRect->x - imageRect->w);
				imageRect->y = (imageRect->y + imageRect->h - imageRect->w);

				imageRect->w += imageRect->h;
				imageRect->h = imageRect->w - imageRect->h;
				imageRect->w = imageRect->w - imageRect->h;
				LimitUpdateAll();
				SDL_Delay(10);
				imageRect->x = (imageRect->x - imageRect->w + imageRect->h);
				imageRect->y = (imageRect->y + imageRect->h - imageRect->w);
				
				imageRect->w += imageRect->h;
				imageRect->h = imageRect->w - imageRect->h;
				imageRect->w = imageRect->w - imageRect->h;
				LimitUpdateAll();
				SDL_Delay(10);
				imageRect->x = (imageRect->x - imageRect->w);
				imageRect->y = (imageRect->y + imageRect->h - imageRect->w);

				imageRect->w += imageRect->h;
				imageRect->h = imageRect->w - imageRect->h;
				imageRect->w = imageRect->w - imageRect->h;
			}
			break;
		}
	}
}

void mapKeyStatToKeyTable(void)
{
	if(keyStat[SDL_SCANCODE_UP] == 1)
	keyPressTable[KEY_PRESS_SURFACE_UP] = true;
	else if(keyStat[SDL_SCANCODE_UP] == 0)
	keyPressTable[KEY_PRESS_SURFACE_UP] = false;
	
	if(keyStat[SDL_SCANCODE_DOWN] == 1)
	keyPressTable[KEY_PRESS_SURFACE_DOWN] = true;
	else if(keyStat[SDL_SCANCODE_DOWN] == 0)
	keyPressTable[KEY_PRESS_SURFACE_DOWN] = false;
		
	if(keyStat[SDL_SCANCODE_LEFT] == 1)
	{
		keyPressTable[KEY_PRESS_SURFACE_LEFT] = true;
		motionFace = 'l';
	}	
	else if(keyStat[SDL_SCANCODE_LEFT] == 0)
	keyPressTable[KEY_PRESS_SURFACE_LEFT] = false;
	
	if(keyStat[SDL_SCANCODE_RIGHT] == 1)
	{
		keyPressTable[KEY_PRESS_SURFACE_RIGHT] = true;
		motionFace = 'r';
	}	
	else if(keyStat[SDL_SCANCODE_RIGHT] == 0)
	keyPressTable[KEY_PRESS_SURFACE_RIGHT] = false;
				
	if((keyStat[SDL_SCANCODE_R] == 1) & rollKeyDown)
	{
		keyPressTable[KEY_PRESS_SURFACE_R] = false;
		rollKeyDown = false;
	}
	else if((keyStat[SDL_SCANCODE_R] == 0) & !rollKeyDown)
	{
		keyPressTable[KEY_PRESS_SURFACE_R] = true;
		rollKeyDown = true;
	}

	if(keyStat[SDL_SCANCODE_SPACE] == 1)
	keyPressTable[KEY_PRESS_SURFACE_SPACE] = true;
	else if(keyStat[SDL_SCANCODE_SPACE] == 0)
	keyPressTable[KEY_PRESS_SURFACE_SPACE] = false;
						
	if((keyStat[SDL_SCANCODE_RCTRL] == 1))
	{		
		keyPressTable[KEY_PRESS_SURFACE_RCTRL] = false;
		duckKeyDown = true;
	}	
	else if((keyStat[SDL_SCANCODE_RCTRL] == 0) & duckKeyDown)
	{
		keyPressTable[KEY_PRESS_SURFACE_RCTRL] = true;
		duckKeyDown = false;
	}
}
