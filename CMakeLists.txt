# This file configures the "build" of this SKSE plugin
# - how to compile the C++ plugin (in addition to settings found in CMakePresets.json)
# - where to output the resulting .dll file

# It's recommended to set a minimum CMake version.
# If you use CMake features from higher versions, update this to match.
cmake_minimum_required(VERSION 3.28)
# 3.28 for c++23 and CMAKE_CXX_SCAN_FOR_MODULES

# [BEGIN set_project_name]
# Set your project name. This will be the name of your SKSE .dll file.
project(GHotReload VERSION 1.0.0 LANGUAGES CXX)
# [END set_project_name]

# build all .cpp and .ixx files in src/
file(GLOB_RECURSE SOURCE_MAIN src/*.ixx)
file(GLOB_RECURSE SOURCE_SHARED src_shared/*.ixx)
file(GLOB_RECURSE SOURCE_LOADER src_loader/*.ixx)
# you might want to add more folders to search for source files

# #
# YOU DO NOT NEED TO EDIT ANYTHING BELOW HERE
# #

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable experimental dynamic dependency support for C++ modules
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

# Setup your SKSE plugin as an SKSE plugin!
find_package(CommonLibSSE CONFIG REQUIRED)

# see build\release\vcpkg_installed\x64-windows-static-md\share\CommonLibSSE\CommonLibSSE.cmake
add_commonlibsse_plugin(${PROJECT_NAME}_loader  SOURCES ${SOURCE_SHARED} ${SOURCE_LOADER})
add_commonlibsse_plugin(${PROJECT_NAME}         SOURCES ${SOURCE_SHARED} ${SOURCE_MAIN})

# Define a FILE_SET for the module interface files
target_sources("${PROJECT_NAME}_loader" PRIVATE FILE_SET cxx_modules_loader TYPE CXX_MODULES FILES ${SOURCE_LOADER})
target_sources("${PROJECT_NAME}_loader" PRIVATE FILE_SET cxx_modules_shared TYPE CXX_MODULES FILES ${SOURCE_SHARED})
target_sources("${PROJECT_NAME}"        PRIVATE FILE_SET cxx_modules_shared TYPE CXX_MODULES FILES ${SOURCE_SHARED})
target_sources("${PROJECT_NAME}"        PRIVATE FILE_SET cxx_modules        TYPE CXX_MODULES FILES ${SOURCE_MAIN})

add_library(my_compiler_flags INTERFACE)
target_compile_features(my_compiler_flags INTERFACE cxx_std_23) # <--- use C++23 standard
target_precompile_headers(my_compiler_flags INTERFACE PCH.h) # precompiled headers
# Include the header files from the include/ directory
target_include_directories(my_compiler_flags INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Disable warning C4100 unreferenced formal parameter
if (MSVC)
    target_compile_options(my_compiler_flags INTERFACE /wd4100)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_definitions(my_compiler_flags INTERFACE _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING) # silence spam warning STL4043 for debug compile in logger
    endif()
endif()

target_link_libraries(${PROJECT_NAME}_loader  PUBLIC my_compiler_flags)
target_link_libraries(${PROJECT_NAME}         PUBLIC my_compiler_flags)

# NOTE: if you sometimes get raceconditions when running cmake parallel:
# like plugin_main.ixx(23): error C3474: could not open output file 'plugin_main.ifc'
# ctrl+, settings: "Cmake: Parallel Jobs" to 1  -> .vscode\settings.json : "cmake.parallelJobs": 1,

# +++++ output folders 

# If you're not using a mod manager, you probably want the SKSE plugin to go
# inside of your Skyrim "Data" folder.
#
# To do this automatically, set the `SKYRIM_FOLDER` environment variable
# to the path of your Skyrim Special Edition folder
if(DEFINED ENV{SKYRIM_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_FOLDER}/Data")
    set(OUTPUT_FOLDER "$ENV{SKYRIM_FOLDER}\\Data")
endif()

# If you're using Mod Organizer 2 or Vortex, you might want this to go inside
# of your "mods" folder, inside of a subfolder named "<your mod>".
#
# To do this automatically, set the `SKYRIM_MODS_FOLDER` environment variable
# to the path of your "mods" folder. Reboot your PC after changing environment variables
if(DEFINED ENV{SKYRIM_MODS_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_MODS_FOLDER}")
    set(OUTPUT_FOLDER "$ENV{SKYRIM_MODS_FOLDER}\\${PROJECT_NAME}")
    message(STATUS "ENV{SKYRIM_MODS_FOLDER} -> OUTPUT_FOLDER=${OUTPUT_FOLDER}")
endif()

# Otherwise, you can set OUTPUT_FOLDER to any place you'd like :)
# set(OUTPUT_FOLDER "C:/path/to/any/folder")

# When your SKSE .dll is compiled, this will automatically copy the .dll into your mods folder.
# Only works if you configure DEPLOY_ROOT above (or set the SKYRIM_MODS_FOLDER environment variable)
if(DEFINED OUTPUT_FOLDER)
    # If you specify an <OUTPUT_FOLDER> (including via environment variables)
    # then we'll copy your mod files into Skyrim or a mod manager for you!

    # Copy the SKSE plugin .dll files into the SKSE/Plugins/ folder
    set(DLL_FOLDER "${OUTPUT_FOLDER}\\SKSE\\Plugins")
    set(SUB_FOLDER "${DLL_FOLDER}\\sub")

    message(STATUS "SKSE plugin output folder: ${DLL_FOLDER}")
    message(STATUS "SKSE plugin output folder-sub: ${SUB_FOLDER}")

    # set(LOADER_DEST "${DLL_FOLDER}\\$<TARGET_FILE_NAME:${PROJECT_NAME}_loader>")
    set(LOADER_DEST "${DLL_FOLDER}\\${PROJECT_NAME}_loader.dll")

    # try to copy ${PROJECT_NAME}_loader.dll even if no code is changed, but ignore error if the game is running
    add_custom_target(copy_dll ALL
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${DLL_FOLDER}"
        COMMAND "${CMAKE_SOURCE_DIR}/copy_dll.bat" "$<TARGET_FILE:${PROJECT_NAME}_loader>" "${LOADER_DEST}"
        COMMENT "Copying DLL to destination folder"
        VERBATIM
    )
    
    # copy ${PROJECT_NAME}.dll
    add_custom_command(
        TARGET "${PROJECT_NAME}"
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${DLL_FOLDER}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${SUB_FOLDER}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${SUB_FOLDER}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
        VERBATIM
    )

    # If you perform a "Debug" build, also copy .pdb file (for debug symbols)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_custom_command(
            TARGET "${PROJECT_NAME}_loader"
            POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_PDB_FILE:${PROJECT_NAME}_loader>" "${DLL_FOLDER}/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}_loader>"
            VERBATIM
        )
        add_custom_command(
            TARGET "${PROJECT_NAME}"
            POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_PDB_FILE:${PROJECT_NAME}>" "${SUB_FOLDER}/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}>"
            VERBATIM
        )
    endif()
endif()
