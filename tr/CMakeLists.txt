
find_program(LUPDATE lupdate)
find_program(LRELEASE lrelease)

if(LUPDATE AND LRELEASE)

    set(translations pl en)
    set(qm_files)
    set(tr_update_targets)

    # collect all source files for translations
    file(GLOB_RECURSE source_files
         LIST_DIRECTORIES FALSE
         ${CMAKE_SOURCE_DIR}/src/*.cpp
         ${CMAKE_SOURCE_DIR}/src/*.hpp
         ${CMAKE_SOURCE_DIR}/src/*.ui
         ${CMAKE_SOURCE_DIR}/src/*.qml
    )

    # write down all paths into a file
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/sources.lst "")
    foreach(file ${source_files})
        file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/sources.lst ${file})
        file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/sources.lst "\n")
    endforeach()

    foreach(tr IN LISTS translations)

        # generate TS files from source files lists
        # this command generates two files:
        # one is a ts file in source dir which should be under version control system.
        # second one is a flag in binary dir indicating ts file was generated.
        # It would be cool to have one file but if ts file is marked as this command's
        # output then it will be deleted every time 'clean' target is triggered.
        add_custom_command(OUTPUT photo_broom_${tr}.ts.flag
                           COMMAND ${LUPDATE}
                           ARGS    @sources.lst
                                   -I ${PROJECT_SOURCE_DIR}/src/core
                                   -I ${PROJECT_SOURCE_DIR}/src/database
                                   -I ${PROJECT_SOURCE_DIR}/src/photos_crawler
                                   -I ${PROJECT_SOURCE_DIR}/src/plugins
                                   -noobsolete
                                   -ts ${PROJECT_SOURCE_DIR}/tr/photo_broom_${tr}.ts
                           COMMAND ${CMAKE_COMMAND}
                           ARGS    -E touch photo_broom_${tr}.ts.flag
                           DEPENDS ${source_files}
                                   ${CMAKE_CURRENT_BINARY_DIR}/sources.lst
        )

        # qm generation rules
        add_custom_command(OUTPUT photo_broom_${tr}.qm
                           COMMAND ${LRELEASE} ${PROJECT_SOURCE_DIR}/tr/photo_broom_${tr}.ts -qm photo_broom_${tr}.qm
                           DEPENDS photo_broom_${tr}.ts.flag
        )

        # installation rules
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/photo_broom_${tr}.qm
                DESTINATION ${PATH_TR}
        )

        # remember qm files
        list(APPEND qm_files
                    ${CMAKE_CURRENT_BINARY_DIR}/photo_broom_${tr}.qm
        )

    endforeach()

    # translations target to be attached to photo broom target
    add_custom_target(translations
                      DEPENDS ${tr_update_targets}
                      DEPENDS ${qm_files}
    )

    add_dependencies(photo_broom translations)

endif()
