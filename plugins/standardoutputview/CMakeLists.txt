
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9522 )

add_subdirectory(tests)

########### next target ###############

set(standardoutputview_LIB_SRCS
    standardoutputview.cpp
    outputwidget.cpp
    toolviewdata.cpp
)

kde4_add_plugin(kdevstandardoutputview ${standardoutputview_LIB_SRCS})
target_link_libraries(kdevstandardoutputview ${KDE4_KDEUI_LIBS}
    kdevplatforminterfaces sublime kdevplatformutil kdevplatformoutputview)

install(TARGETS kdevstandardoutputview DESTINATION ${PLUGIN_INSTALL_DIR} )

install( FILES kdevstandardoutputview.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevstandardoutputview.rc DESTINATION ${DATA_INSTALL_DIR}/kdevstandardoutputview )

