#include <iostream>
#include <memory>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

template <typename T>
struct sdl_deleter;

template <>
struct sdl_deleter<SDL_Surface> {

    using value_type    = SDL_Surface;
    using deleter_type  = sdl_deleter<value_type>;

    void operator ( )(value_type *obj) const
    {
        SDL_FreeSurface( obj );
    }
};

template <>
struct sdl_deleter<SDL_Window> {

    using value_type    = SDL_Window;
    using deleter_type  = sdl_deleter<value_type>;

    void operator ( )( value_type *obj ) const
    {
        SDL_DestroyWindow( obj );
    }
};

template <typename T>
class sdl_object {

public:

    using value_type    = T;
    using deleter_type  = typename sdl_deleter<T>::deleter_type;
    using object_ptr    = std::unique_ptr<T, deleter_type>;

    sdl_object( )
    { }

    explicit sdl_object( value_type *obj )
        :ptr_(obj, sdl_deleter<value_type>( ))
    { }

    sdl_object( const sdl_object & ) = delete;
    sdl_object& operator = ( const sdl_object & ) = delete;

    sdl_object( sdl_object &&other )
        :ptr_(std::move(other.ptr_))
    { }

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

private:

    object_ptr ptr_;
};

int main( )
{
    int res = SDL_Init( SDL_INIT_VIDEO );

    sdl_object<SDL_Window> main_window (
            SDL_CreateWindow( "test", SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              640, 480, SDL_WINDOW_SHOWN ) );



    return 0;
}
