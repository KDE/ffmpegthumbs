set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
########### next target ###############


include_directories( .. )


########### next target ###############
set(ffmpegthumbtest_SRCS ffmpegthumbtest.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../ffmpegthumbnailer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../ffmpegthumbnailer/filmstripfilter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../ffmpegthumbnailer/moviedecoder.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../ffmpegthumbnailer/imagewriter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../ffmpegthumbnailer/videothumbnailer.cpp
)


add_executable(ffmpegthumbtest ${ffmpegthumbtest_SRCS} )

target_link_libraries(ffmpegthumbtest Qt5::Gui KF5::KIOWidgets ${AVUTIL_LIBRARIES} ${AVFILTER_LIBRARIES} ${AVFORMAT_LIBRARIES} ${AVCODEC_LIBRARIES} ${SWSCALE_LIBRARIES})




