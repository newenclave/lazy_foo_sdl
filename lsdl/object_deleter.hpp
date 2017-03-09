#ifndef LSDL_OBJECT_DELETER_HPP
#define LSDL_OBJECT_DELETER_HPP

namespace lsdl {

template <typename T>
struct object_deleter;

#define LSDL_DECLARE_DELETER( Type, DeleteCall )            \
    template <>                                             \
    struct object_deleter<Type> {                           \
                                                            \
        using value_type    = Type;                         \
        using deleter_type  = object_deleter<value_type>;   \
                                                            \
        void operator ( )( value_type *obj ) const          \
        {                                                   \
            DeleteCall( obj );                              \
        }                                                   \
    }

LSDL_DECLARE_DELETER( SDL_Surface,   SDL_FreeSurface );
LSDL_DECLARE_DELETER( SDL_Window,    SDL_DestroyWindow );
LSDL_DECLARE_DELETER( SDL_Renderer,  SDL_DestroyRenderer );
LSDL_DECLARE_DELETER( SDL_Texture,   SDL_DestroyTexture );

}

#endif // OBJECT_DELETER_HPP
