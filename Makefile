CC := g++
STD := c++11
SRCDIR := src
BUILDDIR := build
TARGET := ./bin/exec

SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
SWITCH := $(addprefix build/,$(notdir $(SOURCES:.cpp=.o)))
OBJECTS := $(SWITCH)

MODULE1 := bboard

$(TARGET): $(OBJECTS)
	@echo "Linking..."
	@mkdir -p bin
	$(CC) -std=$(STD) $^ -o $(TARGET)


# build main files
build/%.o: $(SRCDIR)/%.$(SRCEXT)
	@echo "Build main..."
	@mkdir -p $(BUILDDIR)
	$(CC) -std=$(STD) -I $(SRCDIR)/$(MODULE1) -c -o $@ $<

# build modules
build/%.o: src/$(MODULE1)/%.$(SRCEXT)
	@echo "Build..."
	@mkdir -p $(BUILDDIR)
	$(CC) -std=$(STD) -c -o $@ $<

clean:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET)
	@echo
