if( NOT TARGET BitTao )
	get_filename_component( BITTAO_SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../src/" ABSOLUTE )
	get_filename_component( BITTAO_INCLUDE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../include/" ABSOLUTE )
	get_filename_component( BITTAO_LIBRARY_PATH "${CMAKE_CURRENT_LIST_DIR}/../../lib/" ABSOLUTE )
	get_filename_component( CINDER_PATH "${CMAKE_CURRENT_LIST_DIR}/../../../../" ABSOLUTE )

	file(GLOB_RECURSE BIT_TAO_SRC "${BITTAO_SOURCE_PATH}/*.cpp")
	file(GLOB_RECURSE BIT_TAO_HEADERS "${BITTAO_INCLUDE_PATH}/*.h*")
	file(GLOB_RECURSE BIT_TAO_LIBRARIES "${BITTAO_LIBRARY_PATH}/${CMAKE_BUILD_TYPE}/*.lib")

	function(assign_source_group)
	    foreach(_source IN ITEMS ${ARGN})
	        if (IS_ABSOLUTE "${_source}")
	            file(RELATIVE_PATH _source_rel "${BITTAO_INCLUDE_PATH}" "${_source}")
	        else()
	            set(_source_rel "${_source}")
	        endif()
	        get_filename_component(_source_path "${_source_rel}" PATH)
	        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
	        source_group("${_source_path_msvc}" FILES "${_source}")
	    endforeach()
	endfunction(assign_source_group)

	assign_source_group(${BIT_TAO_HEADERS})

	set(BOOST_ROOT "D:/" CACHE STRING "Paths to boost")
	set(Boost_USE_STATIC_LIBS        ON) # only find static libs
	set(Boost_USE_MULTITHREADED      ON)
	set(Boost_USE_STATIC_RUNTIME      ON)
	find_package(Boost 1.60.0 COMPONENTS chrono date_time filesystem system regex thread)
	if(Boost_FOUND)
		message("Boost FOUND! no need to copy libs.")
		message("${Boost_INCLUDE_DIRS}")
		message("${Boost_LIBRARIES}")
	else()
		message("Boost not found, use local libs.")
		message("${BIT_TAO_LIBRARIES}")
	endif()

	if( NOT TARGET cinder )
	    include( "${CINDER_PATH}/proj/cmake/configure.cmake" )
	    find_package( cinder REQUIRED PATHS
	        "${CINDER_PATH}/${CINDER_LIB_DIRECTORY}"
	        "$ENV{CINDER_PATH}/${CINDER_LIB_DIRECTORY}" )
	endif()

	if( NOT TARGET OpenCV3)
		get_filename_component( OpenCV3_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../../OpenCV3/proj/cmake" ABSOLUTE )
		find_package( OpenCV3 REQUIRED PATHS "${OpenCV3_MODULE_PATH}" )
	endif()

	if( NOT TARGET CinderGstreamer)
		get_filename_component( CinderGstreamer_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../../CinderGstreamer/proj/cmake" ABSOLUTE )
		find_package( CinderGstreamer REQUIRED PATHS "${CinderGstreamer_MODULE_PATH}" )
	endif()
	message("${CinderGstreamer_INCLUDES}")

	add_library( BitTao ${BIT_TAO_SRC} ${BIT_TAO_HEADERS})

	target_include_directories( BitTao PUBLIC 
		${BITTAO_SOURCE_PATH}
		${BITTAO_INCLUDE_PATH}
		"${Boost_INCLUDE_DIRS}"
		"${CINDER_PATH}/include"
		"${OpenCV3_INCLUDES}"
		"${CinderGstreamer_INCLUDES}"
		)

	target_link_libraries( BitTao PUBLIC ${OpenCV3_LIBRARIES} ${BIT_TAO_LIBRARIES} ${Boost_LIBRARIES} PRIVATE cinder)

	
endif()



