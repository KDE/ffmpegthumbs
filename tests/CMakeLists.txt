set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

add_executable(ffmpegthumbtest)

target_sources(ffmpegthumbtest PRIVATE
    ffmpegthumbtest.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../ffmpegthumbnailer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../ffmpegthumbnailer/filmstripfilter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../ffmpegthumbnailer/moviedecoder.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../ffmpegthumbnailer/imagewriter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../ffmpegthumbnailer/videothumbnailer.cpp
)
ecm_qt_declare_logging_category(ffmpegthumbtest
    HEADER ffmpegthumbs_debug.h
    IDENTIFIER ffmpegthumbs_LOG
    CATEGORY_NAME org.kde.kdemultimedia.ffmpegthumbs
    DEFAULT_SEVERITY Critical
)

kconfig_add_kcfg_files(ffmpegthumbtest ${CMAKE_CURRENT_SOURCE_DIR}/../ffmpegthumbnailersettings5.kcfgc)

target_include_directories(ffmpegthumbtest PRIVATE ..)

target_link_libraries(ffmpegthumbtest Qt::Core Qt::Gui KF5::KIOWidgets KF5::KIOCore KF5::I18n KF5::ConfigCore KF5::ConfigGui ${AVUTIL_LIBRARIES} ${AVFILTER_LIBRARIES} ${AVFORMAT_LIBRARIES} ${AVCODEC_LIBRARIES} ${SWSCALE_LIBRARIES})




