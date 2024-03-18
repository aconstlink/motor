#include "../typedefs.h"
#include "../components/button.hpp"
#include "../components/scroll.hpp"
#include "../components/point.hpp"
#include "../device.hpp"


namespace motor
{
    namespace controls
    {
        namespace layouts
        {
            class midi_controller
            {
                motor_this_typedefs( midi_controller ) ;

            public:

                enum class input_component
                {};

                enum class output_component 
                {};

                static void_t init_components( motor::controls::device_inout_t dev ) 
                {
                }
            };
            motor_typedef( midi_controller ) ;
        }
        motor_typedefs( motor::controls::device_with< motor::controls::layouts::midi_controller>, midi_device ) ;
    }
}