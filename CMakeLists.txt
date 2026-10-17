cmake_minimum_required(VERSION 3.27)

project(tincan VERSION 1.0 DESCRIPTION "TinCan")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES
	src/utils_collision.cpp
	src/gamedata.cpp
	src/game_layer.cpp
	src/game_world.cpp
	src/main.cpp
	src/system_behaviour.cpp
	src/system_contact.cpp
	src/system_damage.cpp
	src/system_drawable.cpp
	src/system_enemy_spawner.cpp
	src/system_group.cpp
	src/system_input.cpp
	src/system_lifetime.cpp
	src/system_map.cpp
	src/system_movement.cpp
	src/system_pickup.cpp
	src/system_power_weapon.cpp
	src/system_projectile.cpp
	src/system_shield.cpp
	src/system_targeting.cpp
	src/system_weapon.cpp
	src/system_world_bounds.cpp
	src/system_animation.cpp
	src/system_behaviour_editor.cpp
	src/system_formation_editor.cpp
	src/system_player_editor.cpp
	src/debug_draw_path.cpp
	src/sound_player.cpp
	src/system_sound.cpp
	src/system_hit_stop.cpp
	src/system_respawn.cpp
	src/system_floating_text.cpp
	src/system_score.cpp
	src/game_over_layer.cpp
)

find_package(moth_anim REQUIRED)
find_package(moth_bridge REQUIRED)
find_package(moth_ecs REQUIRED)
find_package(moth_tilemap REQUIRED)
find_package(moth_profile REQUIRED)
find_package(moth_audio REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} moth::anim)
target_link_libraries(${PROJECT_NAME} moth::bridge)
target_link_libraries(${PROJECT_NAME} moth::ecs)
target_link_libraries(${PROJECT_NAME} moth::tilemap)
target_link_libraries(${PROJECT_NAME} moth::profile moth::profile_imgui)
target_link_libraries(${PROJECT_NAME} moth::audio)

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/ignore:4099")
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W3 /WX")
	target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-Wall -Werror")
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

