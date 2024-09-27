
#include "api.h"
#include "double_dispatch.h"

namespace motor
{
    namespace scene
    {
        class MOTOR_SCENE_API ivisitable
        {
            motor_core_dd_id_vfn() ;

        public:
            virtual ~ivisitable( void_t ) noexcept {}
        };
    }
}