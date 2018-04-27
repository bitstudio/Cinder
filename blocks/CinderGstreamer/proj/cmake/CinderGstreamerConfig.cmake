if( NOT TARGET CinderGstreamer )

	get_filename_component( BIT_GSTREAMER_SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../src/" ABSOLUTE )
	get_filename_component( BIT_GSTREAMER_INCLUDE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../include/" ABSOLUTE )
	get_filename_component( CINDER_PATH "${CMAKE_CURRENT_LIST_DIR}/../../../../" ABSOLUTE )

	file(GLOB_RECURSE BIT_GSTREAMER_SRC "${BIT_GSTREAMER_SOURCE_PATH}/*.cpp")
	file(GLOB_RECURSE BIT_GSTREAMER_HEADERS "${BIT_GSTREAMER_INCLUDE_PATH}/*.h*")

	if(MSVC)

		get_filename_component( GSTREAMER_LIBRARY_PATH "D:/gstreamer/1.0/x86_64/lib/" ABSOLUTE )
		file(GLOB_RECURSE GST_LIBRARIES "${GSTREAMER_LIBRARY_PATH}/*.lib")

		set(CinderGstreamer_INCLUDES 
			${BIT_GSTREAMER_INCLUDE_PATH} 
			"D:/gstreamer/1.0/x86_64/include"
			"D:/gstreamer/1.0/x86_64/include/gstreamer-1.0"
			"D:/gstreamer/1.0/x86_64/include/glib-2.0"
			"D:/gstreamer/1.0/x86_64/lib/glib-2.0/include"
			"D:/gstreamer/1.0/x86_64/include/libxml2" )

	else(MSVC)
		find_package(PkgConfig)

		pkg_check_modules(GST REQUIRED gstreamer-1.0>=1.4
				               gstreamer-sdp-1.0>=1.4
				               gstreamer-video-1.0>=1.4
				               gstreamer-app-1.0>=1.4)

		set(CinderGstreamer_INCLUDES 
			${BIT_GSTREAMER_INCLUDE_PATH} 
			${GST_INCLUDE_DIRS})
		
	endif(MSVC)


	if( NOT TARGET cinder )
	    include( "${CINDER_PATH}/proj/cmake/configure.cmake" )
	    find_package( cinder REQUIRED PATHS
	        "${CINDER_PATH}/${CINDER_LIB_DIRECTORY}"
	        "$ENV{CINDER_PATH}/${CINDER_LIB_DIRECTORY}" )
	endif()

	# add_definitions(-D__STDC_LIMIT_MACROS)
	add_library( CinderGstreamer ${BIT_GSTREAMER_SRC} ${BIT_GSTREAMER_HEADERS})

	target_include_directories( CinderGstreamer PUBLIC 
		${BIT_GSTREAMER_SOURCE_PATH}
		${CinderGstreamer_INCLUDES}
		"${CINDER_PATH}/include"
		)

	target_link_libraries( CinderGstreamer PUBLIC ${GST_LIBRARIES} PRIVATE cinder)

	
endif()



