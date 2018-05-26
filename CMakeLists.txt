project(ffmpegthumbs)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set(QT_MIN_VERSION "5.2.0")

find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(FeatureSummary)
include(WriteBasicConfigVersionFile)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Gui)
find_package(KF5 REQUIRED COMPONENTS KIO I18n Config)
find_package(FFmpeg COMPONENTS AVCODEC AVFORMAT SWSCALE)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PC_AVCODEC_INCLUDEDIR}
    ${PC_AVFORMAT_INCLUDEDIR}
    ${FFMPEG_INCLUDE_DIR}
    )

# Certain versions of FFMPEG need this to be defined
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D __STDC_CONSTANT_MACROS")

set( ffmpegthumbs_PART_SRCS
  ffmpegthumbnailer.cpp
  ffmpegthumbnailer/filmstripfilter.cpp
  ffmpegthumbnailer/moviedecoder.cpp
  ffmpegthumbnailer/imagewriter.cpp
  ffmpegthumbnailer/videothumbnailer.cpp
)

kconfig_add_kcfg_files(ffmpegthumbs_PART_SRCS ffmpegthumbnailersettings5.kcfgc)
add_library(ffmpegthumbs MODULE ${ffmpegthumbs_PART_SRCS})

target_link_libraries(ffmpegthumbs Qt5::Core Qt5::Gui KF5::KIOWidgets KF5::KIOCore KF5::I18n KF5::ConfigCore KF5::ConfigGui ${AVUTIL_LIBRARIES} ${AVFILTER_LIBRARIES} ${AVFORMAT_LIBRARIES} ${AVCODEC_LIBRARIES} ${SWSCALE_LIBRARIES} )

install(FILES ffmpegthumbnailersettings5.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(TARGETS ffmpegthumbs DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES  ffmpegthumbs.desktop DESTINATION ${SERVICES_INSTALL_DIR})

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
