project(codeutils)

########### install target ###############

set(kdevswitchtobuddy_PART_SRCS
    switchtobuddyplugin.cpp
)

include_directories(CMAKE_CURRENT_BINARY_DIR)

kde4_add_plugin(kdevswitchtobuddy ${kdevswitchtobuddy_PART_SRCS})

target_link_libraries(kdevswitchtobuddy
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
)

install(TARGETS kdevswitchtobuddy DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevswitchtobuddy.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
