#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "GameWindow.h"
//#include "SimpleJSON/json.hpp"

int main(int argc, char **argv) {
  GameWindow* gameWindow = GameWindow::Create();

  if(!gameWindow)
  {
    SDL_Quit();
    std::cout << "SDL init failed." << std::endl;
    return -1;
  }
  bool game_open = true;
  SDL_Event event;
  while(game_open)
  {
    SDL_PollEvent(&event);
    if(event.type == SDL_QUIT){
      break;
    }
    gameWindow->drawFrame();
    SDL_Delay(16);
  }
  return 0;
}