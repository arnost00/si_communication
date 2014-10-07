include (si_communication.pri)

# Build statically - no dynamic linking!
CONFIG += staticlib
CONFIG += static
LIBS += -static
DEFINES += STATIC
