
function(copy_asset TARGET_NAME)
    target_sources(${TARGET_NAME} PRIVATE ${ARGN})

    foreach(ASSET IN ITEMS ${ARGN})
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${ASSET}" "${CMAKE_CURRENT_BINARY_DIR}/${ASSET}" COPYONLY)

        if(APPLE)
            cmake_path(GET ASSET PARENT_PATH ASSET_DIR_PATH)
            set_source_files_properties(
                ${ASSET}
                PROPERTIES
                MACOSX_PACKAGE_LOCATION Resources/${ASSET_DIR_PATH}
            )
        endif()
    endforeach()
endfunction()
