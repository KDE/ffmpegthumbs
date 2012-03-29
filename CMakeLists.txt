find_package(Qt4 REQUIRED)
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(FFmpeg COMPONENTS AVCODEC AVFORMAT SWSCALE)

include_directories(
    ${QT_INCLUDES}
    ${KDE4_INCLUDES}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PC_AVCODEC_INCLUDEDIR}
    ${PC_AVFORMAT_INCLUDEDIR}
    ${FFMPEG_INCLUDE_DIR}
    )

# Certain versions of FFMPEG need this to be defined
SET( CMAKE_CXX_FLAGS "-D __STDC_CONSTANT_MACROS" )

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
