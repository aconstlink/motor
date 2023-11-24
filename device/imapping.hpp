

#pragma once

#include <motor/std/string>

namespace motor
{
    namespace device
    {
        class imapping
        {
        private:

            motor::string_t _name ;

        public:

            imapping( motor::string_cref_t name ) : _name( name ) {}
            imapping( imapping&& rhv ) : _name( std::move( rhv._name ) ) {}
            imapping( imapping const& rhv ) : _name( rhv._name ) {}
            virtual ~imapping( void_t ) {}

            motor::string_cref_t name( void_t ) const noexcept { return _name ; }

        protected:

            void_t set_name( motor::string_cref_t name ) noexcept { _name = name ; }
            motor::string_t move_name( void_t ) noexcept { return std::move( _name ) ; }

        public:

            virtual void_t update( void_t ) noexcept = 0;
        };
        motor_typedef( imapping ) ;
    }
}