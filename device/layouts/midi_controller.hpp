#include "../typedefs.h"
#include "../components/button.hpp"
#include "../components/scroll.hpp"
#include "../components/point.hpp"
#include "../device.hpp"


namespace motor
{
    namespace device
    {
        namespace layouts
        {
            class midi_controller
            {
                motor_this_typedefs( midi_controller ) ;
                motor_typedefs( motor::device::device_tpl< this_t >, this_device ) ;

                friend this_device_t ;

            public:

                enum class input_component
                {};

                enum class output_component 
                {};

                static void_t init_components( this_t::this_device_ref_t dev ) 
                {
                }
            };
            motor_typedef( midi_controller ) ;
        }
        motor_typedefs( motor::device::device_tpl< motor::device::layouts::midi_controller>, midi_device ) ;
    }
}