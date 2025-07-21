cmake_minimum_required(VERSION 3.10)

project(tincan VERSION 1.0 DESCRIPTION "TinCan")

message("Building with CMake version: ${CMAKE_VERSION}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

set(SOURCES ${SOURCES}
	src/main.cpp
	src/game_layer.cpp
	src/system_input.cpp
	src/system_movement.cpp
	src/system_drawable.cpp
	src/system_weapon.cpp
	src/system_lifetime.cpp
	src/system_enemy_spawner.cpp
	src/system_projectile.cpp
	src/system_shield.cpp
	src/system_player_visuals.cpp
	src/collision_utils.cpp
	src/gamedata_projectile.cpp
	src/gamedata_sprite.cpp
	src/gamedata_enemy.cpp
	src/gamedata_weapon.cpp
	src/system_behaviour.cpp
	src/gamedata_spawner.cpp
	src/system_group.cpp
	src/gamedata_level.cpp
	src/system_level.cpp
	src/system_pickup.cpp
	src/gamedata_pickup.cpp
	src/game_world.cpp
	src/system_world_bounds.cpp
	src/system_targeting.cpp
	src/gamedata.cpp
	src/system_power_weapon.cpp
)

find_package(canyon REQUIRED)
find_package(EnTT REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} canyon::canyon)
target_link_libraries(${PROJECT_NAME} EnTT::EnTT)

if (MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/ignore:4099")
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W3 /WX")
	target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-Wall -Werror")
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

