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
MODULE2 := agents

INCL1 := $(SRCDIR)/$(MODULE1)
INCL2 := $(SRCDIR)/$(MODULE2)

$(TARGET): $(OBJECTS)
	@echo "Linking..."
	@mkdir -p bin
	$(CC) -std=$(STD) $^ -o $(TARGET)


# build main files
build/%.o: $(SRCDIR)/%.$(SRCEXT)
	@echo "Build main..."
	@mkdir -p $(BUILDDIR)
	$(CC) -std=$(STD) -c -o $@ $< -I $(INCL2) -I $(INCL1)

# build modules
build/%.o: src/$(MODULE1)/%.$(SRCEXT)
	@echo "Building bboard"
	@mkdir -p $(BUILDDIR)
	$(CC) -std=$(STD) -c -o $@ $<

build/%.o: src/$(MODULE2)/%.$(SRCEXT)
	@echo "Building agents"
	@mkdir -p $(BUILDDIR)
	$(CC) -std=$(STD) -c -o $@ $< -I $(INCL1)


clean:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET)
	@echo
