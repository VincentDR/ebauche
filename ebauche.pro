HEADERS += \
    openglwindow.h \
    camera.h \
    castlewindow.h

SOURCES += \
    main.cpp \
    openglwindow.cpp \
    camera.cpp \
    castlewindow.cpp
	
target.path = $$[QT_INSTALL_EXAMPLES]/gui/openglwindow
INSTALLS += target

RESOURCES += \
    gestionnaire.qrc

LIBS += -lGLU -lm

