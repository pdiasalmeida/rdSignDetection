CC=			g++
CFLAGS=		-c -g -O0 -Wall `pkg-config --cflags opencv`
LDFLAGS=	`pkg-config --libs opencv`
SOURCES=	auxiliar/Files.cpp ihls_nhs/math_utils.cpp \
			ihls_nhs/ihls.cpp ihls_nhs/nhs.cpp \
			postProcess/PostProcess.cpp \
			RSDS.cpp main.cpp
OBJECTS=	$(SOURCES:.cpp=.o)
EXECUTABLE=	rs_detection

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	rm -f $(OBJECTS) $(EXECUTABLE) result/*