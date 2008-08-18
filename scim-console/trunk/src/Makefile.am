bin_PROGRAMS = scim-console

scim_console_SOURCES = scim_console.cpp scim_console.h \
                        scim_console_client.cpp scim_console_client.h \
                        scim_console_common.h \
                        scim_console_debug.cpp scim_console_debug.h \
                        scim_console_display.cpp scim_console_display.h \
                        scim_console_event_listener.cpp scim_console_event_listener.h \
                        scim_console_helper.cpp scim_console_helper.h \
                        scim_console_helper_manager.cpp scim_console_helper_manager.h \
                        scim_console_imcontext.cpp scim_console_imcontext.h \
                        scim_console_interruption_listener.cpp scim_console_interruption_listener.h \
                        scim_console_key_event_reader.h \
                        scim_console_key_event_reader_linux.cpp \
                        scim_console_menu.cpp scim_console_menu.h \
                        scim_console_panel.cpp scim_console_panel.h \
                        scim_console_point.cpp scim_console_point.h \
                        scim_console_stdin_reader.cpp  scim_console_stdin_reader.h 

scim_console_CXXFLAGS = @SCIM_CFLAGS@ \
                        -DSCIM_LIBEXECDIR=\"@SCIM_LIBEXECDIR@\" \
                        -DSCIM_BINARY_VERSION=\"@SCIM_BINARY_VERSION@\" \
                        -DSCIM_VERSION=\"@SCIM_VERSION@\" \
                        @CURSES_CFLAGS@ @UTIL_CFLAGS@ @ROTE_CFLAGS@

scim_console_LDFLAGS  = @SCIM_LIBS@ @CURSES_LIBS@ @UTIL_LIBS@ @ROTE_LIBS@
