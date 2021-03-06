
find_package(OpenLibrary REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

include(GenerateExportHeader)

configure_file(config.hpp.in ${CMAKE_BINARY_DIR}/config.hpp)

if(BUILD_UPDATER)
    add_definitions(-DUPDATER_ENABLED)
    add_subdirectory(updater)
endif()

add_subdirectory(core)
add_subdirectory(crash_catcher)
add_subdirectory(photos_crawler)
add_subdirectory(database)
add_subdirectory(face_recognition)
add_subdirectory(gui)
add_subdirectory(plugins)
add_subdirectory(project_utils)
add_subdirectory(unit_tests_utils)
add_subdirectory(system)

set(BROOM_SOURCES main.cpp
    $<TARGET_OBJECTS:plugins>
)

if(WIN32)

    set(EXE_FLAGS WIN32)
    configure_file(env/PhotoBroom.hpp.cmake PhotoBroom.hpp @ONLY)

    include_directories(${CMAKE_BINARY_DIR}/tools)                                 #for PhotoBroom.rc include files
    set(BROOM_SOURCES ${BROOM_SOURCES} env/PhotoBroom.rc)

else()
    set(EXE_FLAGS)
endif()

add_executable(photo_broom ${EXE_FLAGS} ${BROOM_SOURCES})

target_link_libraries(photo_broom
    core
    crash_catcher
    database
    face_recognition
    gui
    project_utils
    system
    Qt::Core
    Qt::Widgets
)

target_include_directories(photo_broom
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)

install(TARGETS photo_broom RUNTIME DESTINATION ${PATH_BIN})

if(WIN32)
    #photo_broom on Windows requires Images (for exe file ico)
    add_dependencies(photo_broom Images)
endif()
