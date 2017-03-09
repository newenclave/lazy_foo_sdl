#ifndef LSDL_OBJECT_PTR_HPP
#define LSDL_OBJECT_PTR_HPP

#include <memory>

#include "object_deleter.hpp"

namespace lsdl {

    template <typename T>
    class object_ptr {

    public:

        using value_type    = T;
        using deleter_type  = typename object_deleter<T>::deleter_type;
        using pointer_type  = std::unique_ptr<T, deleter_type>;

        object_ptr( )
        { }

    //    virtual ~sdl_object( )
    //    { }

        object_ptr( value_type *obj )
            :ptr_(obj, object_deleter<value_type>( ))
        { }

        object_ptr( object_ptr &&other )
            :ptr_(std::move(other.ptr_))
        { }

        object_ptr( const object_ptr & )              = delete;
        object_ptr& operator = ( const object_ptr & ) = delete;

        object_ptr& operator = ( object_ptr &&other )
        {
            ptr_.swap( other.ptr_ );
            return *this;
        }

        object_ptr& operator = ( value_type *obj )
        {
            object_ptr tmp(obj);
            swap( tmp );
            return *this;
        }

        void swap( object_ptr &other )
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

        pointer_type ptr_;
    };

}

#endif // OBJECT_PTR_HPP
