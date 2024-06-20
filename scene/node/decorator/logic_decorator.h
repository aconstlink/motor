
#include "../decorator.h"

namespace motor
{
    namespace scene
    {
        class MOTOR_SCENE_API logic_decorator : public decorator
        {
            typedef decorator base_t ;
            motor_this_typedefs( logic_decorator ) ;

        public:

            logic_decorator( void_t ) noexcept ;
            logic_decorator( this_rref_t ) noexcept ;
            logic_decorator( this_cref_t ) = delete ;
            logic_decorator( node_ptr_t ) noexcept;
            virtual ~logic_decorator( void_t ) noexcept ;
                
        public:

            void_t set_decorated( motor::scene::node_mtr_safe_t ) noexcept ;

        public:

            virtual motor::scene::result apply( motor::scene::ivisitor_ptr_t ) noexcept ;
        };
        motor_typedef( logic_decorator ) ;
    }
}