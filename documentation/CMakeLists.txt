set(kdevplatformdocumentation_LIB_SRCS
    standarddocumentationview.cpp
    documentationfindwidget.cpp
    documentationview.cpp
)

kde4_add_ui_files(kdevplatformdocumentation_LIB_SRCS documentationfindwidget.ui)
kde4_add_library(kdevplatformdocumentation SHARED ${kdevplatformdocumentation_LIB_SRCS})

install(TARGETS kdevplatformdocumentation EXPORT KDevPlatformTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

target_link_libraries(kdevplatformdocumentation
        ${QT_QTWEBKIT_LIBRARY}
        kdevplatforminterfaces)

target_link_libraries(kdevplatformdocumentation LINK_INTERFACE_LIBRARIES
        kdevplatforminterfaces)

set_target_properties(kdevplatformdocumentation PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${KDEVPLATFORM_LIB_SOVERSION})

install(FILES
    documentationexport.h
    documentationfindwidget.h
    standarddocumentationview.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/documentation COMPONENT Devel
 )
