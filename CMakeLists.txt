cmake_minimum_required( VERSION 3.10 )


#--------------------------------------------------
project( imgui-app )
message( STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}" )
message( STATUS "Compiler ID: ${CMAKE_CXX_COMPILER_ID} (${CMAKE_CXX_COMPILER_VERSION})" )
#--------------------------------------------------


#--------------------------------------------------
set( CMAKE_DISABLE_SOURCE_CHANGES  ON  )
set( CMAKE_DISABLE_IN_SOURCE_BUILD ON  )
set( CMAKE_CXX_STANDARD            17  )
set( CMAKE_CXX_STANDARD_REQUIRED   ON  )
set( CMAKE_CXX_EXTENSIONS          OFF )

set( THIRDPARTY_DIR ${CMAKE_CURRENT_LIST_DIR}/thirdparty )

set( IMGUI_COMPILE_GLFW ON  )
set( IMGUI_TARGET_WEB   OFF )
#--------------------------------------------------


#--------------------------------------------------
if( ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten" )
    set( IMGUI_COMPILE_GLFW OFF )
    set( IMGUI_TARGET_WEB   ON  )
    message( STATUS ">> Building web-GUI" )
endif()
#--------------------------------------------------


#--------------------------------------------------
#----- Set a default install directory in case none is passed
if( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT OR NOT CMAKE_INSTALL_PREFIX )
    set( CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "default install path" FORCE )
    message( STATUS ">> No installation directory given. Using: '${CMAKE_INSTALL_PREFIX}'" )
else()
    message( STATUS ">> Installation directory given: '${CMAKE_INSTALL_PREFIX}'" )
endif()

#----- RPATH setup for the installation
if( APPLE AND PLANNER_BUNDLE_APP )
    # Use the full RPATH of the build tree
    set( CMAKE_SKIP_BUILD_RPATH FALSE )

    # When building, don't use the install RPATH, it will still be used
    # later on in the install phase
    set( CMAKE_BUILD_WITH_INSTALL_RPATH FALSE )

    # Add the automaticaly determined parts of the RPATH which point to
    # directories outside of the build tree to the install RPATH
    set( CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE )
endif()
#--------------------------------------------------


#--------------------------------------------------
#----- GLFW subproject
if( IMGUI_COMPILE_GLFW )
    set( GLFW_BUILD_EXAMPLES OFF CACHE BOOL "the folder is not in this repo"           FORCE )
    set( GLFW_BUILD_TESTS    OFF CACHE BOOL "the folder is not in this repo"           FORCE )
    set( GLFW_BUILD_DOCS     OFF CACHE BOOL "the folder is not in this repo"           FORCE )
    set( GLFW_INSTALL        OFF CACHE BOOL "we don't want this in the install folder" FORCE )

    add_subdirectory( ${THIRDPARTY_DIR}/glfw )
endif()
#--------------------------------------------------


#--------------------------------------------------
#----- Dear ImGui library
set( IMGUI_SRC_DIR ${THIRDPARTY_DIR}/imgui )

set( IMGUI_PUBLIC_HEADERS ${IMGUI_SRC_DIR}/imgui.h )

set( IMGUI_SOURCES
    ${IMGUI_SRC_DIR}/imgui.cpp
    ${IMGUI_SRC_DIR}/imgui_demo.cpp
    ${IMGUI_SRC_DIR}/imgui_draw.cpp
    ${IMGUI_SRC_DIR}/imgui_tables.cpp
    ${IMGUI_SRC_DIR}/imgui_widgets.cpp
    ${IMGUI_SRC_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_SRC_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_SRC_DIR}/misc/cpp/imgui_stdlib.cpp
    ${THIRDPARTY_DIR}/glad/glad.c )

add_library( imgui STATIC ${IMGUI_SOURCES} )

target_link_libraries( imgui glfw )

target_include_directories( imgui PRIVATE ${THIRDPARTY_DIR} ${IMGUI_SRC_DIR} )
#--------------------------------------------------


#--------------------------------------------------
#----- fmtlib
add_subdirectory( ${CMAKE_CURRENT_LIST_DIR}/thirdparty/fmt )
#--------------------------------------------------


#--------------------------------------------------
#----- The application
set( EXECUTABLE_NAME app )

add_executable( ${EXECUTABLE_NAME} ${OS_BUNDLE_NAME}
    ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/fonts.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/styles.cpp )

target_include_directories( ${EXECUTABLE_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${THIRDPARTY_DIR} )

target_link_libraries( ${EXECUTABLE_NAME} imgui fmt::fmt )

if( IMGUI_COMPILE_GLFW )
    target_link_libraries( ${EXECUTABLE_NAME} glfw )
elseif( IMGUI_TARGET_WEB )
    target_include_directories( ${EXECUTABLE_NAME} PRIVATE ${EMSCRIPTEN_ROOT_PATH}/system/include )
    ### Set specific flags for emcc and to place the result under webapp/js/
    set_target_properties( ${EXECUTABLE_NAME} PROPERTIES
        COMPILE_FLAGS "-O3 -g -s WASM=1 -s USE_WEBGL2=1 -s USE_GLFW=3 -s FULL_ES3=1 -s DEMANGLE_SUPPORT=1 -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1 -s ASSERTIONS=1 -s DISABLE_EXCEPTION_CATCHING=0 -s FORCE_FILESYSTEM=1"
        LINK_FLAGS    "-O3 -g -s WASM=1 -s USE_WEBGL2=1 -s USE_GLFW=3 -s FULL_ES3=1 -s DEMANGLE_SUPPORT=1 -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1 -s ASSERTIONS=1 -s DISABLE_EXCEPTION_CATCHING=0 -s FORCE_FILESYSTEM=1 -s ALLOW_MEMORY_GROWTH=1 -s EXTRA_EXPORTED_RUNTIME_METHODS=['cwrap, FS']"
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/webapp/js )
    target_compile_definitions( ${EXECUTABLE_NAME} PRIVATE -DIMGUIAPP_TARGET_WEB )
endif()
#--------------------------------------------------


#--------------------------------------------------
#----- Installing executable
# The bundle is at the root of the install tree,
# otherwiese it'll go into the bin directory.
if( APPLE )
    install( TARGETS ${EXECUTABLE_NAME}
        BUNDLE  DESTINATION .   COMPONENT bundle
        RUNTIME DESTINATION bin COMPONENT runtime )
else()
    # install( FILES ${CMAKE_CURRENT_LIST_DIR}/resources/AppIcon.ico DESTINATION . )
    install( TARGETS ${EXECUTABLE_NAME} DESTINATION . COMPONENT bundle )
endif()

#----- Installing documentation
install( FILES ${CMAKE_CURRENT_LIST_DIR}/README.md DESTINATION . RENAME Readme.md COMPONENT docs )

#----- Installing licenses
install( FILES ${CMAKE_CURRENT_LIST_DIR}/LICENSE DESTINATION . RENAME License.txt COMPONENT licenses )

install( FILES ${CMAKE_CURRENT_LIST_DIR}/thirdparty/fmt/LICENSE.rst   DESTINATION thirdparty-licenses RENAME fmt.rst   COMPONENT licenses )
install( FILES ${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui/LICENSE.txt DESTINATION thirdparty-licenses RENAME imgui.txt COMPONENT licenses )
install( FILES ${CMAKE_CURRENT_LIST_DIR}/thirdparty/glfw/LICENSE.md   DESTINATION thirdparty-licenses RENAME glfw.txt  COMPONENT licenses )
#--------------------------------------------------


#--------------------------------------------------
#----- Bundling paths
set( APPS "\${CMAKE_INSTALL_PREFIX}/bin/${EXECUTABLE_NAME}" )

if( WIN32 )
    set( APPS "${CMAKE_INSTALL_PREFIX}/${EXECUTABLE_NAME}.exe" )
endif( WIN32 )

#----- Bundling properties on macOS
if( BUNDLE_APP AND APPLE )
    set( APPS "\${CMAKE_INSTALL_PREFIX}/${EXECUTABLE_NAME}.app" )

    ### Apple bundle resource files
    # For Apple set the icns file containing icons
    # set where in the bundle to put the icns file
    # set_source_files_properties(
    #     ${CMAKE_CURRENT_LIST_DIR}/resources/AppIcon.icns
    #     ${CMAKE_CURRENT_LIST_DIR}/resources/Icon.png
    #     PROPERTIES MACOSX_PACKAGE_LOCATION Resources )

    ### Apple bundle Info.plist file
    set_target_properties( ${EXECUTABLE_NAME} PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME    "ImGui App"
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.gpmueller.ImGuiApp"
        MACOSX_BUNDLE_ICON_FILE      "AppIcon.icns"
        MACOSX_BUNDLE_INFO_STRING    "ImGui App"
        MACOSX_BUNDLE_COPYRIGHT      "© Gideon Müller")
endif()

#----- Bundle install
if( BUNDLE_APP AND APPLE )
    # Now the work of copying dependencies into the bundle/package
    # The quotes are escaped and variables to use at install time have their $ escaped
    # An alternative is the do a configure_file() on a script and use install(SCRIPT  ...).

    ### Need to make the app and binary executable
    install( CODE "
        include(BundleUtilities)
        fixup_bundle(\"${APPS}\" \"\" \"${DIRS}\")
        " COMPONENT bundle )

    install( CODE "
        execute_process( COMMAND chmod +x \"\${CMAKE_INSTALL_PREFIX}/${EXECUTABLE_NAME}.app\" )
        execute_process( COMMAND chmod +x \"\${CMAKE_INSTALL_PREFIX}/${EXECUTABLE_NAME}.app/Contents/MacOS/${EXECUTABLE_NAME}\" )
        " COMPONENT bundle )
endif()
#--------------------------------------------------


#--------------------------------------------------
#----- Packaging
# To Create a package, one can run "cpack -G DragNDrop CPackConfig.cmake"
# on Mac OS X where CPackConfig.cmake is created by including CPack.
# And then there's ways to customize this as well
set( CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION . )
include( InstallRequiredSystemLibraries )

if( APPLE  )
    set( CPACK_GENERATOR "DragNDrop" )
    set( CPACK_BINARY_DRAGNDROP ON )
    set( CPACK_COMPONENTS_ALL bundle )
elseif( WIN32 )
    set( CPACK_GENERATOR "ZIP" )
    set( CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE )
    set( CPACK_COMPONENTS_ALL bundle icons licenses docs )
else()
    set( CPACK_GENERATOR "ZIP" )
    set( CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE )
    set( CPACK_COMPONENTS_ALL bundle icons licenses docs )
endif()

set( CPACK_ARCHIVE_COMPONENT_INSTALL ON )
set( CPACK_PACKAGE_VERSION_MAJOR ${META_VERSION_MAJOR} )
set( CPACK_PACKAGE_VERSION_MINOR ${META_VERSION_MINOR} )
set( CPACK_PACKAGE_VERSION_PATCH ${META_VERSION_PATCH} )

include( CPack )
#--------------------------------------------------