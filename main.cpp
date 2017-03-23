#include <iostream>
#include <memory>
#include <queue>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "lsdl/object_ptr.hpp"

#include <condition_variable>
#include <mutex>
#include <functional>

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

template <typename ValueType>
struct queue_trait {
    typedef ValueType value_type;
    typedef std::queue<value_type> queue_type;

    static
    void push( queue_type &q, const value_type &value )
    {
        q.emplace( value );
    }

    static
    value_type &front( queue_type &q )
    {
        return q.front( );
    }

    static
    const value_type &front( const queue_type &q )
    {
        return q.front( );
    }

    static
    void pop( queue_type &q )
    {
        return q.pop( );
    }

    static
    size_t size( const queue_type &q )
    {
        return q.size( );
    }

    static
    bool empty( const queue_type &q )
    {
        return q.empty( );
    }

    static
    void clear( queue_type &q )
    {
        q.clear( );
    }
};

enum result_enum {
    WAIT_OK       = 0,
    WAIT_CANCELED = 1,
    WAIT_TIMEOUT  = 2,
};

template <typename ValueType, typename MutexType = std::mutex,
          typename ConditionalType = std::condition_variable,
          typename QueueTrait = queue_trait<ValueType> >
class condition_queue {

public:

    typedef QueueTrait                      q_traits;
    typedef ValueType                       value_type;
    typedef MutexType                       mutex_type;
    typedef std::unique_lock<mutex_type>    locker_type;
    typedef std::condition_variable         conditional_type;
    typedef typename q_traits::queue_type   queue_type;

    condition_queue( )
        :cancel_(false)
    { }

    void reset( )
    {
        locker_type l(queue_lock_);
        cancel_ = false;
    }

    void clear( )
    {
        locker_type l(queue_lock_);
        q_traits::clear( queue_ );
    }

    void cancel( )
    {
        locker_type l(queue_lock_);
        cancel_ = true;
        queue_cond_.notify_all( );
    }

    void push( const value_type &new_value )
    {
        locker_type l(queue_lock_);
        q_traits::push( queue_, new_value );
        queue_cond_.notify_one( );
    }

    template <typename Duration>
    result_enum wait_for( value_type &out, const Duration &dur )
    {
        locker_type l(queue_lock_);
        bool wr = queue_cond_.wait_for( l, dur, not_empty(this) );
        if( wr ) {
            if( cancel_ ) {
                return WAIT_CANCELED;
            } else {
                std::swap(out, q_traits::front( queue_ ));
                q_traits::pop( queue_ );
                return WAIT_OK;
            }
        }
        return WAIT_TIMEOUT;
    }

    result_enum wait( value_type &out )
    {
        locker_type l(queue_lock_);
        queue_cond_.wait( l, not_empty(this) );
        if( cancel_ ) {
            return WAIT_CANCELED;
        } else {
            std::swap(out, q_traits::front( queue_ ));
            q_traits::pop( queue_ );
            return WAIT_OK;
        }
    }

    result_enum wait0( value_type &out )
    {
        locker_type l(queue_lock_);
        if( !q_traits::front( queue_ ) ) {
            std::swap(out, q_traits::front( queue_ ));
            q_traits::pop( queue_ );
            return WAIT_OK;
        } else {
            return WAIT_TIMEOUT;
        }
    }

    bool canceled( ) const
    {
        locker_type l(queue_lock_);
        return cancel_;
    }

private:

    struct not_empty {

        not_empty( const condition_queue *parent )
            :parent_(parent)
        { }

        not_empty &operator = (const not_empty& other)
        {
            parent_ = other.parent_;
            return *this;
        }

        bool operator ( )( ) const
        {
            return !q_traits::empty(parent_->queue_) || parent_->cancel_;
        }

        const condition_queue *parent_;
    };

    friend struct not_empty;

    bool                cancel_;
    queue_type          queue_;
    conditional_type    queue_cond_;
    mutex_type          queue_lock_;
};

int main( )
{

    using func_queue = condition_queue<std::function<void( )> >;

    func_queue calls;

    calls.push( [ ]( ) { std::cout << "Hello"; } );
    calls.push( [ ]( ) { std::cout << ", "; } );
    calls.push( [ ]( ) { std::cout << "world!\n"; } );

    std::function<void( )> c;

    while( WAIT_OK == calls.wait_for( c, std::chrono::nanoseconds(0) ) ) {
        c( );
    }

    return 0;

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

