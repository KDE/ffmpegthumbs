cmake_minimum_required(VERSION 3.16)

# Release Service version, managed by release script
set(RELEASE_SERVICE_VERSION_MAJOR "24")
set(RELEASE_SERVICE_VERSION_MINOR "07")
set(RELEASE_SERVICE_VERSION_MICRO "70")

set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(ffmpegthumbs VERSION ${RELEASE_SERVICE_VERSION})

set(QT_MIN_VERSION "5.15.0")
set(KF_MIN_VERSION "5.92.0")

if (QT_MAJOR_VERSION STREQUAL "6")
    set(QT_REQUIRED_VERSION "6.5.0")
    set(KF_MIN_VERSION "5.240.0")
endif()


find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH}  ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMQtDeclareLoggingCategory)
include(FeatureSummary)
include(ECMDeprecationSettings)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Gui)
find_package(KF${QT_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED COMPONENTS KIO Config)
find_package(FFmpeg COMPONENTS AVCODEC AVFORMAT SWSCALE)

include_directories(
    ${FFMPEG_INCLUDE_DIRS}
    )

# Certain versions of FFMPEG need this to be defined
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D __STDC_CONSTANT_MACROS")

add_definitions(-DTRANSLATION_DOMAIN=\"ffmpegthumbs\")
set(KDE_COMPILERSETTINGS_LEVEL "5.82.0")
ecm_set_disabled_deprecation_versions(QT 5.15.2
    KF 5.81
)

kcoreaddons_add_plugin(ffmpegthumbs INSTALL_NAMESPACE "kf${QT_MAJOR_VERSION}/thumbcreator")

target_sources(ffmpegthumbs PRIVATE
  ffmpegthumbnailer.cpp
  ffmpegthumbnailer/filmstripfilter.cpp
  ffmpegthumbnailer/moviedecoder.cpp
  ffmpegthumbnailer/imagewriter.cpp
  ffmpegthumbnailer/videothumbnailer.cpp
  ffmpegthumbnailer.h
  ffmpegthumbnailer/filmstripfilter.h
  ffmpegthumbnailer/moviedecoder.h
  ffmpegthumbnailer/imagewriter.h
  ffmpegthumbnailer/videothumbnailer.h
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

target_link_libraries(ffmpegthumbs
    Qt::Core
    Qt::Gui
    KF${QT_MAJOR_VERSION}::KIOWidgets
    KF${QT_MAJOR_VERSION}::KIOCore
    KF${QT_MAJOR_VERSION}::ConfigCore
    KF${QT_MAJOR_VERSION}::ConfigGui
    ${AVUTIL_LIBRARIES}
    ${AVFILTER_LIBRARIES}
    ${AVFORMAT_LIBRARIES}
    ${AVCODEC_LIBRARIES}
    ${SWSCALE_LIBRARIES}
)

install(FILES ffmpegthumbnailersettings5.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

ecm_qt_install_logging_categories(
  EXPORT ffmpegthumbs
  DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

########### install files ###############

install(FILES org.kde.ffmpegthumbs.metainfo.xml
        DESTINATION ${KDE_INSTALL_METAINFODIR})

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
