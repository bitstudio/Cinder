if( NOT TARGET OpenCV3 )
	
	set(OpenCV_DIR "D:/opencv/3.4.0/build_x64/install/" CACHE STRING "Paths to opencv install")
	if( NOT OpenCV_FOUND)
		find_package( OpenCV REQUIRED opencv_world opencv_img_hash)
		message("OpenCV Dir >>> ${OpenCV_DIR}")
		message("OpenCV lib path >>> ${OpenCV_LIB_PATH}")
	endif()

	get_filename_component( INCLUDE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../include" ABSOLUTE )
	set(OpenCV3_INCLUDES ${INCLUDE_PATH} ${OpenCV_INCLUDE_DIRS})

	if( OpenCV_FOUND )
		message("OpenCV libs >>> ${OpenCV_LIBS}")
		if(MSVC)
			set(OpenCV3_LIBRARIES ${OpenCV_LIBS} vfw32.lib)
		else(MSVC)
			set(OpenCV3_LIBRARIES ${OpenCV_LIBS} "libgstriff-1.0.so")
		endif(MSVC)
	endif()

	if( NOT TARGET cinder )
	    include( "${CINDER_PATH}/proj/cmake/configure.cmake" )
	    find_package( cinder REQUIRED PATHS
	        "${CINDER_PATH}/${CINDER_LIB_DIRECTORY}"
	        "$ENV{CINDER_PATH}/${CINDER_LIB_DIRECTORY}" )
	endif()
endif()
