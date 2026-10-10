# Keep the upstream notices with binary release packages.
set( motor_license_root "${MOTOR_SOURCE_DIR}/externals" )

install( FILES "${motor_license_root}/ocornut/imgui/LICENSE.txt"
    DESTINATION licenses/imgui )
install( FILES "${motor_license_root}/epezent/implot/LICENSE"
    DESTINATION licenses/implot )
install( FILES "${motor_license_root}/nelarius/imnodes/LICENSE.md"
    DESTINATION licenses/imnodes )
install( FILES "${motor_license_root}/kcat/openal-soft/COPYING"
    "${motor_license_root}/kcat/openal-soft/LICENSE-pffft"
    DESTINATION licenses/openal-soft )
install( FILES "${motor_license_root}/nothings/stb/LICENSE"
    DESTINATION licenses/stb )
install( FILES "${motor_license_root}/aconstlink/rapidxml/LICENSE"
    "${motor_license_root}/aconstlink/rapidxml/license.txt"
    DESTINATION licenses/rapidxml )
install( FILES "${motor_license_root}/nlohmann/json/LICENSE.MIT"
    DESTINATION licenses/nlohmann_json )
install( FILES "${motor_license_root}/jkuhlmann/cgltf/LICENSE"
    DESTINATION licenses/cgltf )

# Lua keeps its license in lua.h; Khronos notices remain in the installed headers.
install( FILES "${motor_license_root}/lua/lua/lua.h"
    DESTINATION licenses/lua )

unset( motor_license_root )
