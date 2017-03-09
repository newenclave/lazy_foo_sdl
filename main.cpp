#include <iostream>
#include <memory>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "lsdl/object_ptr.hpp"

using namespace lsdl;

object_ptr<SDL_Texture> load_text( SDL_Renderer *rndr, const char *path )
{
    object_ptr<SDL_Texture> txt;
    object_ptr<SDL_Surface> res = SDL_LoadBMP( path );

    if( res.get( ) ) {
        txt = SDL_CreateTextureFromSurface( rndr, res.get( ) );
    }
    return std::move(txt);
}

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480

int main( )
{
    int res = SDL_Init( SDL_INIT_VIDEO );

    object_ptr<SDL_Window> main_window =
            SDL_CreateWindow( "test", SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              SCREEN_WIDTH, SCREEN_HEIGHT,
                              SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE );

    object_ptr<SDL_Renderer> renderer =
            SDL_CreateRenderer( main_window.get( ), -1,
                                SDL_RENDERER_ACCELERATED );

    SDL_SetRenderDrawColor( renderer.get( ), 0xFF, 0xFF, 0xFF, 0xFF );
    IMG_Init( IMG_INIT_PNG );

    bool quit = false;

    auto up    = load_text( renderer.get( ), "data/up.bmp" );
    auto down  = load_text( renderer.get( ), "data/down.bmp" );
    auto left  = load_text( renderer.get( ), "data/left.bmp" );
    auto right = load_text( renderer.get( ), "data/right.bmp" );

    auto last = up.get( );

    SDL_Rect rect = { 0, 0, 100, 100 };

    bool press_flag = false;

    while( !quit ) {
        SDL_Event e;
        while( SDL_PollEvent( &e ) ) {
            if( e.type == SDL_QUIT ) {
                quit = true;
                break;
            } else if( e.type == SDL_KEYDOWN ) {
                switch (e.key.keysym.sym) {
                case SDLK_UP:    last = up.get( );    break;
                case SDLK_DOWN:  last = down.get( );  break;
                case SDLK_RIGHT: last = left.get( );  break;
                case SDLK_LEFT:  last = right.get( ); break;
                default:
                    break;
                }
            } else if( e.type == SDL_MOUSEMOTION ) {
                if(press_flag) {
                    rect.x = e.motion.x - 50;
                    rect.y = e.motion.y - 50;
                }
            } else if( e.type == SDL_MOUSEBUTTONUP ) {
                press_flag = false;
            } if( e.type == SDL_MOUSEBUTTONDOWN ) {
                press_flag = true;
            } else if( e.type == SDL_MOUSEMOTION ) {
                //std::cout << "Motion " << e.motion.state << "\n";
            }

            SDL_RenderClear( renderer.get( ) );
            SDL_RenderFillRect( renderer.get( ), NULL );
            SDL_SetRenderDrawColor( renderer.get( ), 0xFF, 0x00, 0x00, 0xFF );
            SDL_Rect fillRect = { SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4,
                                  SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
            SDL_RenderFillRect( renderer.get( ), &fillRect );
            SDL_SetRenderDrawColor( renderer.get( ), 0x00, 0x00, 0xFF, 0xFF );
            SDL_RenderDrawLine( renderer.get( ), 0, SCREEN_HEIGHT / 2,
                                SCREEN_WIDTH, SCREEN_HEIGHT / 2 );
            SDL_RenderCopy( renderer.get( ), last, NULL, &rect );
            SDL_RenderPresent( renderer.get( ) );
            SDL_SetRenderDrawColor( renderer.get( ), 0xFF, 0xFF, 0xFF, 0xFF );
        };
    }

    return 0;
}

void ii( )
{
    //SDL_CommonEvent common;         /**< Common event data */
    SDL_WindowEvent             window;   /**<  SDL_WINDOWEVENT */

    SDL_KeyboardEvent           key;      /**<  SDL_KEYDOWN or
                                                SDL_KEYUP*/

    SDL_TextEditingEvent        edit;     /**<  SDL_TEXTEDITING */

    SDL_TextInputEvent          text;     /**<  SDL_TEXTINPUT  */

    SDL_MouseMotionEvent        motion;   /**<  SDL_MOUSEMOTION */

    SDL_MouseButtonEvent        button;   /**<  SDL_MOUSEBUTTONDOWN or
                                                SDL_MOUSEBUTTONUP */

    SDL_MouseWheelEvent         wheel;    /**<  SDL_MOUSEWHEEL */

    SDL_JoyAxisEvent            jaxis;    /**<  SDL_JOYAXISMOTION */

    SDL_JoyBallEvent            jball;    /**<  SDL_JOYBALLMOTION */

    SDL_JoyHatEvent             jhat;     /**<  SDL_JOYHATMOTION */

    SDL_JoyButtonEvent          jbutton;  /**<  SDL_JOYBUTTONDOWN or
                                                SDL_JOYBUTTONUP */

    SDL_JoyDeviceEvent          jdevice;  /**<  SDL_JOYDEVICEADDED or
                                                SDL_JOYDEVICEREMOVED */

    SDL_ControllerAxisEvent     caxis;    /**<  SDL_CONTROLLERAXISMOTION */

    SDL_ControllerButtonEvent   cbutton;  /**<  SDL_CONTROLLERBUTTONDOWN or
                                                SDL_CONTROLLERBUTTONUP */

    SDL_ControllerDeviceEvent   cdevice;  /**<  SDL_CONTROLLERDEVICEADDED,
                                                SDL_CONTROLLERDEVICEREMOVED, or
                                                SDL_CONTROLLERDEVICEREMAPPED */

    SDL_AudioDeviceEvent        adevice;  /**<  SDL_AUDIODEVICEADDED, or
                                                SDL_AUDIODEVICEREMOVED */

    SDL_QuitEvent               quit;     /**<  SDL_QUIT */

    SDL_UserEvent               user;     /**<  SDL_USEREVENT through
                                                SDL_LASTEVENT-1 */

    SDL_SysWMEvent              syswm;    /**<  SDL_SYSWMEVENT */

    SDL_TouchFingerEvent        tfinger;  /**<  SDL_FINGERMOTION or
                                                SDL_FINGERDOWN or
                                                SDL_FINGERUP*/

    SDL_MultiGestureEvent       mgesture; /**<  SDL_MULTIGESTURE */

    SDL_DollarGestureEvent      dgesture; /**<  SDL_DOLLARGESTURE or
                                                SDL_DOLLARRECORD */

    SDL_DropEvent               drop;     /**<  SDL_DROPFILE */
}

