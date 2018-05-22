cmake_minimum_required( VERSION 3.0 FATAL_ERROR )
set( CMAKE_VERBOSE_MAKEFILE ON )

set( BIT_PROJECT_NAME "NewCinderProject" CACHE STRING "Project name")

set( BIT_BLOCKS "BitTao;CinderGstreamer;OpenCV3;QuickTime;" CACHE STRING "Paths to blocks")


get_filename_component( CINDER_PATH "D:/Cinder/" ABSOLUTE )
get_filename_component( APP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/" ABSOLUTE )

project( ${BIT_PROJECT_NAME} )

include( "${CINDER_PATH}/proj/cmake/modules/cinderMakeApp.cmake" )

file(GLOB_RECURSE BIT_HEADERS "${APP_PATH}/include/*.*")
file(GLOB_RECURSE BIT_SRC "${APP_PATH}/src/*.*")
file(GLOB_RECURSE BIT_CONFIG "${APP_PATH}/config/*.json")

ci_make_app (
    APP_NAME    ${BIT_PROJECT_NAME}
    CINDER_PATH ${CINDER_PATH}
    SOURCES     ${BIT_HEADERS} ${BIT_SRC} ${BIT_CONFIG}
    INCLUDES    ${APP_PATH}/include ${APP_PATH}/config
    BLOCKS 		${BIT_BLOCKS}
)

source_group("config" FILES ${BIT_CONFIG})
foreach(config_file ${BIT_CONFIG} )
	get_filename_component(config_filename ${config_file} NAME)
	message(${config_filename})
	configure_file("${APP_PATH}/config/${config_filename}" "${CMAKE_CURRENT_BINARY_DIR}/${BIT_PROJECT_NAME}/${CMAKE_BUILD_TYPE}/${config_filename}" COPYONLY)
endforeach(config_file)
