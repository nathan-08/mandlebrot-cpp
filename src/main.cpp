#include <SDL.h>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/math/special_functions/gamma.hpp>

#include <iostream>
#include <string>
#include <thread>

using std::cout, std::endl, std::runtime_error, std::exception;
using namespace boost::multiprecision;

SDL_Window *window;
SDL_Renderer *renderer;
const int WIDTH = 1200;
const int HEIGHT = 1200;

//typedef cpp_bin_float_50 ld;
typedef long double ld;

ld min_x = -2;
ld max_x = 2;
ld min_y = -2;
ld max_y = 2;
int max_iterations = 40;
ld factor = 1;

template <typename T>
T map ( T value, T in_min, T in_max, T out_min, T out_max ) {
  return ( value - in_min ) * ( out_max - out_min ) / ( in_max - in_min ) + out_min;
}

void init () {
  if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    throw runtime_error( "failed to init SDL" );

  window = SDL_CreateWindow( "mandlebrot",
      SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN );
  if ( !window )
    throw runtime_error( "failed to create window" );

  renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
  if ( !renderer )
    throw runtime_error( "failed to create renderer" );
}

void quit () {
  SDL_DestroyRenderer( renderer );
  renderer = nullptr;
  SDL_DestroyWindow( window );
  window = nullptr;
  SDL_Quit();
  cout << "goodbye\n";
}

struct Color {
  int r;
  int g;
  int b;
};

Color pixels[WIDTH][HEIGHT];

void updateWindowSlice ( int height, int offset_y ) {
  for ( int x = 0; x < WIDTH; x++ ) { // x = a
    for ( int y = offset_y; y < offset_y + height; y++ ) { // y = b
      ld a = map<ld>( x, 0, WIDTH, min_x, max_x );
      ld b = map<ld>( y, 0, HEIGHT, min_y, max_y );

      ld ai = a;
      ld bi = b;
      int n = 0;

      for ( int i = 0; i < max_iterations; i++ ) {
        ld a1 = a * a - b * b;
        ld b1 = 2 * a * b;

        a = a1 + ai;
        b = b1 + bi;

        if ( ( a + b ) > 2 ) {
          break;
        }
        n++;
      }
      int bright = map<int>( n, 0, max_iterations, 0, 255 );

      //if ( n == max_iterations ) {
        //bright = 0;
      //}

      if ( bright <= 20 ) {
        bright = 0;
      }

      int blue = map<int>( std::sqrt( bright ), 0, std::sqrt( 0xff ), 0, 0xff );
      int green = bright;
      int red = map<int>( bright * bright, 0, 0xff * 0xff, 0, 0xff );
      //int blue = bright;

      pixels[x][y] = Color{ red, green, blue };
    }
  }
}

void render () {
  cout << "rendering (" << max_iterations << ") - (" << min_x << ", " << min_y << ") (" << max_x << ", " << max_y << ")" << endl;

  std::vector<std::thread> threads;
  for ( int i = 0; i < HEIGHT; i += 100 ) {
    threads.push_back( std::thread( updateWindowSlice, 100, i ) );
  }
  for ( auto &t : threads ) {
    t.join();
  }

  for ( int x = 0; x < WIDTH; x++ ) {
    for ( int y = 0; y < HEIGHT; y++ ) {
      Color p = pixels[x][y];
      SDL_SetRenderDrawColor( renderer, p.r, p.g, p.b, 0xff );
      SDL_RenderDrawPoint( renderer, x, y );
    }
  }


  SDL_RenderPresent( renderer );
  cout << "done!\n";
}

void mainloop () {
  SDL_Event e;
  bool quit = false;
  ld x1, y1, x2, y2;

  render();

  while ( !quit ) {
    while ( SDL_PollEvent( &e ) != 0 ) {
      if ( e.type == SDL_QUIT ) {
        quit = true;
      }
      else if ( e.type == SDL_MOUSEBUTTONDOWN ) {
        int x, y;
        SDL_GetMouseState( &x, &y );
        x1 = map<ld>( x, 0, WIDTH, min_x, max_x );
        y1 = map<ld>( y, 0, HEIGHT, min_y, max_y );
      }
      else if ( e.type == SDL_MOUSEBUTTONUP ) {
        int x, y;
        SDL_GetMouseState( &x, &y );
        x2 = map<ld>( x, 0, WIDTH, min_x, max_x );
        y2 = map<ld>( y, 0, HEIGHT, min_y, max_y );
        ld deltax = x2 - x1;
        ld deltay = y2 - y1;
        ld max_delta = std::max( deltax, deltay );
        x2 = x1 + max_delta;
        y2 = y1 + max_delta;
        min_x = x1;
        max_x = x2;
        min_y = y1;
        max_y = y2;
        render();
      }
      else if ( e.type == SDL_KEYDOWN && e.key.repeat == 0 ) {
        if ( e.key.keysym.sym == SDLK_UP ) {
          if ( e.key.keysym.mod == KMOD_LSHIFT ) {
            max_iterations += 1000;
            cout << max_iterations << endl;
          }
          else {
            max_iterations += 100;
            cout << max_iterations << endl;
          }
        }
        else if ( e.key.keysym.sym == SDLK_DOWN ) {
          if ( e.key.keysym.mod == KMOD_LSHIFT ) {
            max_iterations -= 1000;
            max_iterations = max_iterations > 0 ? max_iterations : 0;
            cout << max_iterations << endl;
          }
          else {
            max_iterations -= 100;
            max_iterations = max_iterations > 0 ? max_iterations : 0;
            cout << max_iterations << endl;
          }
        }
        else if ( e.key.keysym.sym == SDLK_f ) {
          render();
        }
      }
    }
  }
}

int main ( int argc, char **argv ) {
  cout << "initializing..." << endl;
  try {
    init();
    mainloop();
  }
  catch ( exception &e ) {
    cout << e.what() << endl;
  }
  quit();
}

