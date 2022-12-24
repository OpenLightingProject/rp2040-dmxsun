cmake_minimum_required(VERSION 3.13)

# Get all files that make up the web ui so it is only rebuilt on change
file(GLOB_RECURSE WWWSRC_FILES CONFIGURE_DEPENDS ${CMAKE_CMAKE_CURRENT_LIST_DIRCURRENT_SOURCE_DIR}/src/*)

# Command and target to install the node modules
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_LIST_DIR}/node_modules.md5
    DEPENDS ${CMAKE_CURRENT_LIST_DIR}/package.json
    COMMAND npm install
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)
add_custom_target(webui_installDeps
    DEPENDS ${CMAKE_CURRENT_LIST_DIR}/node_modules.md5
)

# Command and target to build the web application
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_LIST_DIR}/build.md5
    DEPENDS ${WWWSRC_FILES}
    COMMAND GENERATE_SOURCEMAP=false npm run build
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)
add_custom_target(webui_build
    DEPENDS webui_installDeps ${CMAKE_CURRENT_LIST_DIR}/build.md5
)
