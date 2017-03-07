#include <iostream>
#include <memory>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

template <typename T>
struct sdl_deleter;

#define SDL_DECLARE_DELETER( Type, DeleteCall )         \
    template <>                                         \
    struct sdl_deleter<Type> {                          \
                                                        \
        using value_type    = Type;                     \
        using deleter_type  = sdl_deleter<value_type>;  \
                                                        \
        void operator ( )( value_type *obj ) const      \
        {                                               \
            DeleteCall( obj );                          \
        }                                               \
    }

SDL_DECLARE_DELETER( SDL_Surface,   SDL_FreeSurface );
SDL_DECLARE_DELETER( SDL_Window,    SDL_DestroyWindow );
SDL_DECLARE_DELETER( SDL_Renderer,  SDL_DestroyRenderer );
SDL_DECLARE_DELETER( SDL_Texture,   SDL_DestroyTexture );

template <typename T>
class sdl_object {

public:

    using value_type    = T;
    using deleter_type  = typename sdl_deleter<T>::deleter_type;
    using object_ptr    = std::unique_ptr<T, deleter_type>;

    sdl_object( )
    { }

    sdl_object( value_type *obj )
        :ptr_(obj, sdl_deleter<value_type>( ))
    { }

    sdl_object( sdl_object &&other )
        :ptr_(std::move(other.ptr_))
    { }

    sdl_object( const sdl_object & )              = delete;
    sdl_object& operator = ( const sdl_object & ) = delete;

    sdl_object& operator = ( sdl_object &&other )
    {
        ptr_.swap( other.ptr_ );
        return *this;
    }

    sdl_object& operator = ( value_type *obj )
    {
        sdl_object tmp(obj);
        swap( tmp );
        return *this;
    }

    void swap( sdl_object &other )
    {
        ptr_.swap( other.ptr_ );
    }

    value_type *get( )
    {
        return ptr_.get( );
    }

    const value_type *get( ) const
    {
        return ptr_.get( );
    }

    operator bool ( ) const
    {
        return ptr_.get( ) != nullptr;
    }

private:

    object_ptr ptr_;
};

sdl_object<SDL_Texture> load_text( SDL_Renderer *rndr, const char *path )
{
    sdl_object<SDL_Texture> txt;
    sdl_object<SDL_Surface> res = SDL_LoadBMP( path );

    if( res.get( ) ) {
        txt = SDL_CreateTextureFromSurface( rndr, res.get( ) );
    }
    return std::move(txt);
}

int main( )
{
    int res = SDL_Init( SDL_INIT_VIDEO );

    sdl_object<SDL_Window> main_window =
            SDL_CreateWindow( "test", SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              640, 480, SDL_WINDOW_SHOWN );

    sdl_object<SDL_Renderer> renderer =
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

    while( !quit ) {
        SDL_Event e;
        while( SDL_PollEvent( &e ) ) {
            if( e.type == SDL_QUIT ) {
                quit = true;
                break;
            } else if( e.type == SDL_KEYDOWN ) {
                switch (e.key.keysym.sym) {
                case SDLK_UP:    last = up.get( ); break;
                case SDLK_DOWN:  last = down.get( ); break;
                case SDLK_RIGHT: last = left.get( ); break;
                case SDLK_LEFT:  last = right.get( ); break;
                default:
                    break;
                }
            }

            SDL_RenderClear( renderer.get( ) );
            SDL_RenderCopy( renderer.get( ), last, NULL, &rect );
            SDL_RenderPresent( renderer.get( ) );
        };
    }

    return 0;
}
