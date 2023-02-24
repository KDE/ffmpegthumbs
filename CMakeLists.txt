cmake_minimum_required(VERSION 3.16)

# Release Service version, managed by release script
set(RELEASE_SERVICE_VERSION_MAJOR "22")
set(RELEASE_SERVICE_VERSION_MINOR "12")
set(RELEASE_SERVICE_VERSION_MICRO "3")

set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(ffmpegthumbs VERSION ${RELEASE_SERVICE_VERSION})

set(QT_MIN_VERSION "5.15.0")
set(KF_MIN_VERSION "5.83.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH}  ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMQtDeclareLoggingCategory)
include(FeatureSummary)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Gui)
find_package(KF5 ${KF_MIN_VERSION} REQUIRED COMPONENTS KIO I18n Config)
find_package(FFmpeg COMPONENTS AVCODEC AVFORMAT SWSCALE)

include_directories(
    ${FFMPEG_INCLUDE_DIRS}
    )

# Certain versions of FFMPEG need this to be defined
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D __STDC_CONSTANT_MACROS")

add_definitions(-DTRANSLATION_DOMAIN=\"ffmpegthumbs\")

add_library(ffmpegthumbs MODULE)

target_sources(ffmpegthumbs PRIVATE
  ffmpegthumbnailer.cpp
  ffmpegthumbnailer/filmstripfilter.cpp
  ffmpegthumbnailer/moviedecoder.cpp
  ffmpegthumbnailer/imagewriter.cpp
  ffmpegthumbnailer/videothumbnailer.cpp
)

ecm_qt_declare_logging_category(ffmpegthumbs
  HEADER ffmpegthumbs_debug.h
  IDENTIFIER ffmpegthumbs_LOG
  CATEGORY_NAME org.kde.kdemultimedia.ffmpegthumbs
  DEFAULT_SEVERITY Critical
  EXPORT ffmpegthumbs
  DESCRIPTION "FFmpeg video thumbnailer"
)

kconfig_add_kcfg_files(ffmpegthumbs ffmpegthumbnailersettings5.kcfgc)

target_link_libraries(ffmpegthumbs Qt::Core Qt::Gui KF5::KIOWidgets KF5::KIOCore KF5::I18n KF5::ConfigCore KF5::ConfigGui ${AVUTIL_LIBRARIES} ${AVFILTER_LIBRARIES} ${AVFORMAT_LIBRARIES} ${AVCODEC_LIBRARIES} ${SWSCALE_LIBRARIES})

install(FILES ffmpegthumbnailersettings5.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(TARGETS ffmpegthumbs DESTINATION ${KDE_INSTALL_PLUGINDIR})

ecm_qt_install_logging_categories(
  EXPORT ffmpegthumbs
  DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

########### install files ###############

install(FILES  ffmpegthumbs.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR})

install(FILES org.kde.ffmpegthumbs.metainfo.xml
        DESTINATION ${KDE_INSTALL_METAINFODIR})

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
