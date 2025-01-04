#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

bool init(SDL_Window *&window, SDL_Surface *&screenSurface)
{
    if(SDL_Init( SDL_INIT_VIDEO ) < 0) return false;
  
    window = SDL_CreateWindow("Sonic Freedom Fighters", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_SHOWN);
  
    if(window == NULL) return false;
  
    if(!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) return false;
  
    screenSurface = SDL_GetWindowSurface( window );
  
    return true;
}

bool loadSurface( SDL_Surface* gScreenSurface, std::string path, SDL_Surface*& outputSurface )
{
    //Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
    if( loadedSurface == NULL )
    {
        std::cout << "Unable to load image " << path.c_str() << "! SDL_image Error: " << IMG_GetError() << std::endl;
	    return false;
    }
    else
    {
        //Convert surface to screen format
        outputSurface = SDL_ConvertSurface( loadedSurface, gScreenSurface->format, 0 );

        //Get rid of old loaded surface
        SDL_FreeSurface( loadedSurface );
        if( outputSurface == NULL )
        {
            std::cout << "Unable to optimize image " << path.c_str() << "! SDL Error: " << SDL_GetError() << std::endl;
	    return false;
        }
    }

    return true;
}

int __cdecl main(int argc, char **argv) {
  SDL_Window * window = nullptr;
  SDL_Surface* screenSurface = nullptr;
  SDL_Surface* spriteSheet = nullptr;
  if(!init(window, screenSurface) || !loadSurface(screenSurface, "./sprites/sonic3.png", spriteSheet))
  {
    SDL_Quit();
    std::cout << "SDL init failed." << std::endl;
    return -1;
  }
  SDL_Surface* sprite;

  //Fill the surface white
  SDL_FillRect( screenSurface, NULL, SDL_MapRGB( screenSurface->format, 12, 24, 48 ) );
  
  SDL_BlitSurface(spriteSheet, NULL, screenSurface, NULL);
  
  //Update the surface
  SDL_UpdateWindowSurface( window );

  //Wait two seconds
  SDL_Delay( 2000 );
  
  //Destroy window
  SDL_DestroyWindow( window );

  //Quit SDL subsystems
  SDL_Quit();

  return 0;
}

int __cdecl main()
{
    return main(0, NULL);
}
