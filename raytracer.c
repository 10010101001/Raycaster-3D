#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include "map.h"

#define RESX 960
#define RESY 540
#define HALFRESX 480
#define HALFRESY 270
#define ROTATESPEED 0.03
#define MOVESPEED 0.08
#define STRAFEMOVESPEED 0.0565685424948
#define BGCOL 17
#define clipDistance 26
#define playerSize 1

struct Point3D {
  float x, y, z;
};

struct pos {
  int x, y, z;
};


void main() {

  char placeDist = 3;

  SDL_Window* window = NULL; //init SDL
  SDL_Renderer* renderer = NULL;

  bool running = true;

  if( SDL_Init( SDL_INIT_VIDEO ) < 0 )  {
    printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    running = false;
  }

  window = SDL_CreateWindow("SDL Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, RESX, RESY, SDL_WINDOW_SHOWN );

  if( window==NULL) {
    printf( "SDL_Error: %s\n", SDL_GetError() );
    running = false;
  }

  renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED);

  if(renderer==NULL) {
    printf("Renderer error: %s\n", SDL_GetError() );
    running = false;
  }

  SDL_Texture * texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, RESX,RESY);
  Uint32 pixels[RESX*RESY];


  const Uint8* keystate = SDL_GetKeyboardState( NULL );
  int t1, t2;
  struct Point3D pos;
  pos.x = 4; 
  pos.y = 4;
  pos.z = 29;
  struct Point3D dir;
  dir.x = 1;
  dir.y = 0; //direction vector
  dir.z = 0;
  struct Point3D plane;
  plane.x = 0;
  plane.y = 0.8;
  plane.z = 0.45;
  struct Point3D vel;

  float time = 0;
  float oldtime = 0; //for FPS calculations

  void rotatez(float rotspeed) {
    float oldDirX = dir.x;
    dir.x = dir.x * cos(rotspeed) - dir.y * sin(rotspeed);
    dir.y = oldDirX * sin(rotspeed) + dir.y * cos(rotspeed);
    float oldPlaneX = plane.x;
    plane.x = plane.x * cos(rotspeed) - plane.y * sin(rotspeed);
    plane.y = oldPlaneX * sin(rotspeed) + plane.y * cos(rotspeed);
  }

  void rotatex(float rotspeed) {
    float dirW = 1;
    dir.z = dir.z * cos(rotspeed) - dirW * sin(rotspeed);
    dirW = dir.z * sin(rotspeed) + dirW * cos(rotspeed);
    dir.x *= dirW;
    dir.y *= dirW;
    float planeW = 1;
    plane.z = plane.z * cos(rotspeed) - planeW * sin(rotspeed);
    planeW = plane.z * sin(rotspeed) + planeW * cos(rotspeed);
    plane.x *= planeW;
    plane.y *= planeW;
    printf("%f\n", dirW);
  }


  void move() {
    float velocity = ((keystate[SDL_SCANCODE_D] ^ keystate[SDL_SCANCODE_A]) &&
                      (keystate[SDL_SCANCODE_W] ^ keystate[SDL_SCANCODE_S])) ? STRAFEMOVESPEED : MOVESPEED;

    struct Point3D oldpos = pos;

    if(keystate[SDL_SCANCODE_D]) {
      pos.x -= dir.y * velocity;
      pos.y += dir.x * velocity;
    }
    if(keystate[SDL_SCANCODE_A]) {
      pos.x += dir.y * velocity;
      pos.y -= dir.x * velocity;
    }
    if(keystate[SDL_SCANCODE_W]) {
      pos.x += dir.x * velocity;
      pos.y += dir.y * velocity;
    } 
    if(keystate[SDL_SCANCODE_S]) {
      pos.x -= dir.x * velocity;
      pos.y -= dir.y * velocity;
    }
    if(keystate[SDL_SCANCODE_E]) {
      pos.z -= STRAFEMOVESPEED;
    } 
    if(keystate[SDL_SCANCODE_Q]) {
      pos.z += STRAFEMOVESPEED;
    }

    if(MAP[(char)pos.x][(char)pos.y][(char)pos.z]) pos = oldpos;
  } //remove oldpos and velocity when not needed

  while( running ) {

    t1 = SDL_GetTicks();

    SDL_Event e;
    running  = !(SDL_PollEvent( &e ) && e.type == SDL_QUIT);

    move();

    if(keystate[SDL_SCANCODE_LEFT]) {
      rotatez(-ROTATESPEED);
    }
    if(keystate[SDL_SCANCODE_RIGHT]){
      rotatez(ROTATESPEED);
    }
    /*if(keystate[SDL_SCANCODE_UP]) {
      rotatex(ROTATESPEED);
    }
    if(keystate[SDL_SCANCODE_DOWN]){
      rotatex(-ROTATESPEED); // under construction
    }*/

    if(keystate[SDL_SCANCODE_SPACE] && MAP[(char)(pos.x+dir.x*placeDist)][(char)(pos.y+dir.y*placeDist)][(char)pos.z] == 0) {
      MAP[(char)(pos.x+dir.x*placeDist)][(char)(pos.y+dir.y*placeDist)][(char)pos.z] =  1;
    }
    else if(keystate[SDL_SCANCODE_LCTRL] && MAP[(char)(pos.x+dir.x*placeDist)][(char)(pos.y+dir.y*placeDist)][(char)pos.z] == 1) {
      MAP[(char)(pos.x+dir.x*placeDist)][(char)(pos.y+dir.y*placeDist)][(char)pos.z] =  0;
    }
    if(keystate[SDL_SCANCODE_L]) running = 0;

    for(int x = 0; x < RESX; x += 1 ) { //raycasting code
      for(int y = 0; y < RESY; y += 1) {

        struct Point3D camera;
        camera.x = (float)x/HALFRESX -1; // ray direction from camera, -1 to 1
        camera.y = (float)y/HALFRESY -1; // ray dir
        struct Point3D rayd;
        rayd.z = dir.z + plane.z * camera.y;
        rayd.x = dir.x + plane.x * camera.x; // frustum thingy
        rayd.y = dir.y + plane.y * camera.x;
        
        struct pos map;
        map.x = (char)pos.x;
        map.y = (char)pos.y;
        map.z = (char)pos.z;
        
        struct Point3D sdist;
        struct Point3D delta;
        delta.x = fabsf(1/rayd.x);
        delta.y = fabsf(1/rayd.y);
        delta.z = fabsf(1/rayd.z);

        struct pos step;
        
        if(rayd.x < 0) {
          step.x = -1;
          sdist.x = (pos.x - map.x) * delta.x; // one side
        }
        else {
          step.x = 1;
          sdist.x = (map.x + 1.0 - pos.x) * delta.x; // have to round the other way for this side.
        }
        if(rayd.y < 0) {
          step.y = -1;
          sdist.y = (pos.y - map.y) * delta.y; // one side
        }
        else {
          step.y = 1;
          sdist.y = (map.y + 1.0 - pos.y) * delta.y; // have to round the other way for this side.
        }
        if(rayd.z < 0) {
          step.z = -1;
          sdist.z = (pos.z - map.z) * delta.z; // one side
        }
        else {
          step.z = 1;
          sdist.z = (map.z + 1.0 - pos.z) * delta.z; // have to round the other way for this side.
        }
        
        char side; //either 0 (NS), or 1 (EW), or 2(UD)

        while( MAP[map.x][map.y][map.z] < 1) {
          if(sdist.y < sdist.x ) {
            if(sdist.y < sdist.z) {
              sdist.y += delta.y;
              map.y += step.y;
              side = 1; 
            }
            else {
              sdist.z += delta.z;
              map.z += step.z; 
              side = 2;
            }
          }
          else {
            if(sdist.x < sdist.z) {
              sdist.x += delta.x;
              map.x += step.x;
              side = 0;
            }
            else {
              sdist.z += delta.z;
              map.z += step.z; 
              side = 2;
            }
          }
        }

        if(side == 0) pixels[y*RESX+x] = 0x0000FF;
        else if(side == 1) pixels[y*RESX+x] = 0x00FF00;
        else pixels[y*RESX+x] = 0xFF0000;
      }
    }
    SDL_UpdateTexture(texture, NULL, pixels, RESX * sizeof(Uint32));
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent( renderer );
    t2 = SDL_GetTicks();
    int ms = t2-t1;
    printf("pos: %f, %f, %f | dir: %f, %f, %f  |  render time: %dms\n", pos.x, pos.y, pos.z, dir.x, dir.y, dir.z, ms );

  }
  SDL_DestroyWindow( window );
  SDL_DestroyRenderer( renderer );
  SDL_DestroyTexture( texture );
  SDL_Quit();
}
