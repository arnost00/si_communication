TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += C++11
TARGET = si_read

win32 {
    message(SI Read project win32 settings)
    message(Qt version: $$[QT_VERSION])

    # need to set 4 system envinroment variable (examples added):
    #BOOST_MINGW_PATH_INCLUDE = "c:\boost\1.56.0\include"
    #BOOST_MINGW_PATH_LIB = "c:\boost\1.56.0\lib"
    #BOOST_VER = 1_56
    #BOOST_COMPILER = mgw48

    LIBS += -lws2_32

    INCLUDEPATH += $(BOOST_MINGW_PATH_INCLUDE)
    DEPENDPATH += $(BOOST_MINGW_PATH_INCLUDE)
    LIBS += -L$(BOOST_MINGW_PATH_LIB)

    CONFIG(debug, debug|release) {
        BOOST_SUFFIX = $(BOOST_COMPILER)-mt-d-$(BOOST_VER)
    } else {
        BOOST_SUFFIX = $(BOOST_COMPILER)-mt-$(BOOST_VER)
    }

    LIBS += -lboost_program_options-$$BOOST_SUFFIX \
            -lboost_system-$$BOOST_SUFFIX \
            -lboost_filesystem-$$BOOST_SUFFIX \
            -lboost_thread-$$BOOST_SUFFIX \
            -lboost_date_time-$$BOOST_SUFFIX \

    message(The project will use boost suffix: $$BOOST_SUFFIX)
}

unix {
	QMAKE_CXXFLAGS += -std=c++11

	LIBS +=-lboost_thread\
		-lboost_system\
		-lpthread\
		-lboost_program_options\
		-lboost_date_time
}

SOURCES += SI.cpp

OTHER_FILES += \
    info.txt \
    LICENSE_1_0.txt \
    README.TXT

HEADERS += \
    blocks_read.h \
    card_readouts.h \
    card_record.h \
    command.h \
    command_interface.h \
    command_parameter.h \
    command_parameter2.h \
    command_response_interface.h \
    commands_definitions.h \
    control_sequence_base.h \
    crc529.h \
    create_parameter_sequence.h \
    filelog.h \
    fixed_command.h \
    channel_input.h \
    channel_input_interface.h \
    channel_interface.h \
    channel_io_serial_port.h \
    channel_loopback.h \
    channel_output.h \
    channel_output_interface.h \
    channel_protocol_implementations.h \
    channel_protocol_interface.h \
    channel_std_out.h \
    chip_readout.h \
    input_channel.h \
    input_processor_interface.h \
    io_base.h \
    make_pointer.h \
    program_info.h \
    protocol_encoders.h \
    punch_record.h \
    response.h \
    response_dynamic.h \
    response_interface.h \
    rw_structure.h \
    sequence_position.h \
    si_auxilary.h \
    si_constants.h \
    si_static_command.h \
    startup_sequence.h \
    static_multicommand_parallel.h \
    tuple_child.hpp \
    tuple_type.h \
    type_forwarder.h
