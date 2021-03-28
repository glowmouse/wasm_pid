///
/// PID Simulator Web Assembly
/// (C) Andrew Brownbill 2021
///  

#include <iostream>
#include <utility>
#include <unordered_map>
#include <vector>
#include <string>

#include <SDL/SDL.h>
#include <emscripten.h>

// X and Y screen resolution
constexpr int X_SCREEN=1024;
constexpr int Y_SCREEN=768;

// Make a Color palette
class Palette
{
  public:

  Palette( const SDL_Surface* screen ) 
  {
    // Number of color palette entries
    constexpr unsigned NUM_COLORS=256;

    // Interpolate the palette using 3 base colors.
    constexpr unsigned BASE_COLORS=3;
    constexpr unsigned COLOR_RANGES = BASE_COLORS-1;
    constexpr unsigned ENTRIES_PER_RANGE =    // Round up 
        ( NUM_COLORS + COLOR_RANGES-1) / ( COLOR_RANGES );
    constexpr unsigned RI = 0;    // Red Index
    constexpr unsigned GI = 1;    // Green Index
    constexpr unsigned BI = 2;    // Blue Index

    const unsigned int col[ BASE_COLORS ][3] = { 
      { 128, 220, 255 },    // Light Blue  
      { 255, 255, 0  },     // Yellow
      { 255, 0,  0   }};    // Red

    for ( int i = 0; i < NUM_COLORS ; ++i )
    {
      const unsigned int cn = i / ENTRIES_PER_RANGE;
      const unsigned int co = cn + 1;
      const unsigned int s = i % ENTRIES_PER_RANGE;
      const unsigned int oms = ENTRIES_PER_RANGE - s;

      const unsigned r = col[co][RI] * s   / ENTRIES_PER_RANGE + 
                         col[cn][RI] * oms / ENTRIES_PER_RANGE;
      const unsigned g = col[co][GI] * s   / ENTRIES_PER_RANGE +
                         col[cn][GI] * oms / ENTRIES_PER_RANGE;
      const unsigned b = col[co][BI] * s   / ENTRIES_PER_RANGE +
                         col[cn][BI] * oms / ENTRIES_PER_RANGE;

      values.push_back( SDL_MapRGBA( screen->format, r, g, b, 255 ));
    }
  }
  Palette() = delete;
  Palette( const Palette& ) = delete;
  Palette& operator=( const Palette& ) = delete;

  std::vector<Uint32> values;
};

// Draw the game of life buffer on the screen.
void drawScreen( SDL_Surface *screen )
{
  // for testing
  static Uint32 black = SDL_MapRGBA( screen->format, 0, 0, 0, 255 );
  const Palette palette(screen);
  black += 0x01010101;

  // Clear
  Uint32 *start = (Uint32*)screen->pixels;
  Uint32 *end= start + X_SCREEN * Y_SCREEN;
  for ( Uint32 *cur = start; cur < end; ++cur ) *cur = black;
}

// Creates the screen 
class PIDSimSingleton
{
  public:

  PIDSimSingleton( const PIDSimSingleton& other ) = delete;
  PIDSimSingleton& operator=( const PIDSimSingleton& other ) = delete;

  PIDSimSingleton() 
  {
    SDL_Init(SDL_INIT_VIDEO );
    screen = SDL_SetVideoMode(X_SCREEN, Y_SCREEN, 32, SDL_SWSURFACE);
  }
  ~PIDSimSingleton()
  {
    screen = nullptr;
    SDL_Quit();
  }
  
  void update( void )
  {
    if (SDL_MUSTLOCK(screen)) SDL_LockSurface(screen);
    drawScreen( screen );
    if (SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
    SDL_UpdateRect(screen, 0, 0, 0, 0); 
  }

  private:
  SDL_Surface *screen;
};

std::unique_ptr< PIDSimSingleton > singleton; 

// Advance forward one.  Callback from emscripten
void tick() {
  singleton->update(); 
}

int main(int argc, char** argv) 
{
  srand(time( nullptr ));
  
  singleton = std::unique_ptr< PIDSimSingleton >( new PIDSimSingleton());
  emscripten_set_main_loop(tick, 15000, 0);

  return 0;
}

