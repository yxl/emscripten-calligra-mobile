if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUC)
    add_definitions(-Wno-unused-parameter)
endif (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUC)

if(SHOULD_BUILD_FILTER_XLS_TO_SHEETS OR SHOULD_BUILD_FILTER_SHEETS_TO_XLS)
    add_subdirectory( sidewinder )
endif(SHOULD_BUILD_FILTER_XLS_TO_SHEETS OR SHOULD_BUILD_FILTER_SHEETS_TO_XLS)

if(SHOULD_BUILD_FILTER_XLS_TO_SHEETS)
    add_subdirectory( import )
endif(SHOULD_BUILD_FILTER_XLS_TO_SHEETS)

if(SHOULD_BUILD_FILTER_SHEETS_TO_XLS)
    add_subdirectory( export )
endif(SHOULD_BUILD_FILTER_SHEETS_TO_XLS)

