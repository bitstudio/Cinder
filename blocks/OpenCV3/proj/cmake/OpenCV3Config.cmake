if( NOT TARGET OpenCV3 )
	
	if(MSVC)
		set(OpenCV3_LOCATION "D:/opencv/3.4.0/build_x64/")
		file(GLOB_RECURSE CV_LIBS "${OpenCV3_LOCATION}/lib/${CMAKE_BUILD_TYPE}/*.lib")
		file(GLOB_RECURSE CV_3rdparty_LIBS "${OpenCV3_LOCATION}/3rdparty/lib/${CMAKE_BUILD_TYPE}/*.lib")
		file(GLOB_RECURSE CV_ippicv_LIBS "${OpenCV3_LOCATION}/3rdparty/ippicv/ippicv_win/lib/intel64/*.lib")
	else(MSVC)
		set(OpenCV3_LOCATION "/media/tao/DATA/opencv/3.4.0/build_ubuntu/")
		file(GLOB_RECURSE CV_LIBS "${OpenCV3_LOCATION}/lib/*.a")
		file(GLOB_RECURSE CV_3rdparty_LIBS "${OpenCV3_LOCATION}/3rdparty/lib/*.a")
		file(GLOB_RECURSE CV_ippicv_LIBS "${OpenCV3_LOCATION}/3rdparty/ippicv/ippicv_lnx/lib/intel64/*.a")
	endif(MSVC)

	get_filename_component( INCLUDE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../include" ABSOLUTE )
	get_filename_component( CV_INCLUDE_PATH "${OpenCV3_LOCATION}/install/include" ABSOLUTE )
	set(OpenCV3_INCLUDES ${INCLUDE_PATH} ${CV_INCLUDE_PATH})

	if(MSVC)
		set(OpenCV3_LIBRARIES ${CV_LIBS} ${CV_3rdparty_LIBS} ${CV_ippicv_LIBS} vfw32.lib)
	else(MSVC)
		set(OpenCV3_LIBRARIES ${CV_LIBS} ${CV_3rdparty_LIBS} ${CV_ippicv_LIBS} "libgstriff-1.0.so")
	endif(MSVC)

	message("${OpenCV3_LIBRARIES}")

	if( NOT TARGET cinder )
	    include( "${CINDER_PATH}/proj/cmake/configure.cmake" )
	    find_package( cinder REQUIRED PATHS
	        "${CINDER_PATH}/${CINDER_LIB_DIRECTORY}"
	        "$ENV{CINDER_PATH}/${CINDER_LIB_DIRECTORY}" )
	endif()
endif()
