project (kdevssh_client)
set(kdevssh_client_SRCS main.cpp)
kde4_add_executable(kdevssh_client ${kdevssh_client_SRCS})
target_link_libraries(kdevssh_client ${KDE4_KDEUI_LIBS} ssh2)
install(TARGETS kdevssh_client  ${INSTALL_TARGETS_DEFAULT_ARGS})

