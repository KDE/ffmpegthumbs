
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${AVUTIL_INCLUDES} ${AVFORMAT_INCLUDES} ${AVCODEC_INCLUDES} ${SWSCALE_INCLUDES} )


CHECK_INCLUDE_FILE(ffmpeg/avcodec.h HAVE_FFMPEG_AVCODEC)
CHECK_INCLUDE_FILE(ffmpeg/avformat.h HAVE_FFMPEG_AVFORMAT)
CHECK_INCLUDE_FILE(ffmpeg/avutil.h HAVE_FFMPEG_AVUTIL)
CHECK_INCLUDE_FILE(ffmpeg/swscale.h HAVE_FFMPEG_SWSCALE)


if (HAVE_FFMPEG_AVFORMAT)
	SET( CMAKE_CXX_FLAGS "-D FFMPEG_AVFORMAT_API=1" )
endif(HAVE_FFMPEG_AVFORMAT)

if (HAVE_FFMPEG_AVCODEC)
	SET( CMAKE_CXX_FLAGS "-D FFMPEG_AVCODEC_API=1" )
endif(HAVE_FFMPEG_AVCODEC)

if (HAVE_FFMPEG_SWSCALE)
	SET( CMAKE_CXX_FLAGS "-D FFMPEG_SWSCALE_API=1" )
endif(HAVE_FFMPEG_SWSCALE)


set( ffmpegthumbs_PART_SRCS
  ffmpegthumbnailer.cpp
  ffmpegthumbnailer/filmstripfilter.cpp
  ffmpegthumbnailer/moviedecoder.cpp
  ffmpegthumbnailer/imagewriter.cpp
  ffmpegthumbnailer/videothumbnailer.cpp
)

kde4_add_plugin(ffmpegthumbs ${ffmpegthumbs_PART_SRCS})


target_link_libraries(ffmpegthumbs  ${KDE4_KIO_LIBS} ${AVUTIL_LIBRARIES} ${AVFORMAT_LIBRARIES} ${AVCODEC_LIBRARIES} ${SWSCALE_LIBRARIES} )

install(TARGETS ffmpegthumbs DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES  ffmpegthumbs.desktop DESTINATION ${SERVICES_INSTALL_DIR})

if (KDE4_BUILD_TESTS)
	add_subdirectory(tests)
endif (KDE4_BUILD_TESTS)
