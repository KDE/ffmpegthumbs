project(ffmpegthumbs)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set(QT_MIN_VERSION "5.2.0")

find_package(ECM 5.14.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH}  ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(FeatureSummary)
include(WriteBasicConfigVersionFile)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Gui)
find_package(KF5 REQUIRED COMPONENTS KIO I18n Config)
find_package(FFmpeg COMPONENTS AVCODEC AVFORMAT SWSCALE)
find_package(Taglib REQUIRED)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PC_AVCODEC_INCLUDEDIR}
    ${PC_AVFORMAT_INCLUDEDIR}
    ${FFMPEG_INCLUDE_DIR}
    )

# Certain versions of FFMPEG need this to be defined
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D __STDC_CONSTANT_MACROS")

add_definitions(-DTRANSLATION_DOMAIN=\"ffmpegthumbs\")

set( ffmpegthumbs_PART_SRCS
  ffmpegthumbnailer.cpp
  ffmpegthumbnailer/filmstripfilter.cpp
  ffmpegthumbnailer/moviedecoder.cpp
  ffmpegthumbnailer/imagewriter.cpp
  ffmpegthumbnailer/videothumbnailer.cpp
)

ecm_qt_declare_logging_category(
  ffmpegthumbs_PART_SRCS
  HEADER ffmpegthumbs_debug.h
  IDENTIFIER ffmpegthumbs_LOG
  CATEGORY_NAME org.kde.kdemultimedia.ffmpegthumbs
  DEFAULT_SEVERITY Critical
  EXPORT ffmpegthumbs
  DESCRIPTION "FFmpeg video thumbnailer"
)

kconfig_add_kcfg_files(ffmpegthumbs_PART_SRCS ffmpegthumbnailersettings5.kcfgc)
add_library(ffmpegthumbs MODULE ${ffmpegthumbs_PART_SRCS})

target_link_libraries(ffmpegthumbs Qt5::Core Qt5::Gui KF5::KIOWidgets KF5::KIOCore KF5::I18n KF5::ConfigCore KF5::ConfigGui ${AVUTIL_LIBRARIES} ${AVFILTER_LIBRARIES} ${AVFORMAT_LIBRARIES} ${AVCODEC_LIBRARIES} ${SWSCALE_LIBRARIES} Taglib::Taglib )

install(FILES ffmpegthumbnailersettings5.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(TARGETS ffmpegthumbs DESTINATION ${PLUGIN_INSTALL_DIR})

ecm_qt_install_logging_categories(
  EXPORT ffmpegthumbs
  DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

########### install files ###############

install(FILES  ffmpegthumbs.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES org.kde.ffmpegthumbs.metainfo.xml
        DESTINATION ${KDE_INSTALL_METAINFODIR})

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
