if( NOT TARGET QuickTime )
	
	set(Bit_package_path "D:/bit/packages")
	get_filename_component( INCLUDE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../include" ABSOLUTE )
	set(QuickTime_INCLUDES ${INCLUDE_PATH})

	file(GLOB_RECURSE QuickTime_LIBS "${CMAKE_CURRENT_LIST_DIR}/../../lib/msw/x86/*.lib")
	set(QuickTime_LIBRARIES ${QuickTime_LIBS})

endif()
