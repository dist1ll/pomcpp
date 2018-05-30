CC := g++
SRCDIR := src
BUILDDIR := build
TARGET := bin/exec

SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
SWITCH := $(addprefix build/,$(notdir $(SOURCES:.cpp=.o)))
OBJECTS := $(SWITCH)
INC := -I include

$(TARGET): $(OBJECTS)
	@echo "Linking..."
	@mkdir -p bin
	$(CC) $^ -o $(TARGET)

build/%.o: $(SOURCES)
	@echo "Build..."
	@mkdir -p $(BUILDDIR)
	$(CC) $(INC) -c $< -o $@

clean:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET)
	@echo
