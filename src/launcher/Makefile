CXX=g++
CXXFLAGS=-std=c++17 -Wall -fno-exceptions -O3 -g $(shell sdl2-config --cflags) -I../include -Iinclude
LDFLAGS=$(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf -ldl -lpthread
SRC=$(wildcard *.cpp)
OBJ_DIR=obj
OBJ=$(addprefix $(OBJ_DIR)/, $(notdir $(SRC:.cpp=.o)))
EXEC_DIR=../../
EXEC=game_launcher
ICON=logo128x128.ico
DESKTOP_ENTRY=$(EXEC_DIR)/cc_launcher.desktop

all: $(EXEC) desktop_entry

$(EXEC): $(OBJ)
	$(CXX) $(OBJ) -o $(EXEC_DIR)/$(EXEC) $(LDFLAGS)

$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	$(CXX) -c $< -o $@ $(CXXFLAGS)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

desktop_entry:
	@echo "[Desktop Entry]" > $(DESKTOP_ENTRY)
	@echo "Name=Cube Conflict" >> $(DESKTOP_ENTRY)
	@echo "Exec=$(realpath $(EXEC_DIR)/$(EXEC))" >> $(DESKTOP_ENTRY)
	@echo "Path=$(realpath $(EXEC_DIR))" >> $(DESKTOP_ENTRY)
	@echo "Icon=$(realpath $(ICON))" >> $(DESKTOP_ENTRY)
	@echo "Type=Application" >> $(DESKTOP_ENTRY)
	@echo "Terminal=false" >> $(DESKTOP_ENTRY)
	@echo "Categories=Game;" >> $(DESKTOP_ENTRY)

clean:
	rm -f $(OBJ) $(EXEC_DIR)/$(EXEC) $(DESKTOP_ENTRY)

.PHONY: all clean desktop_entry

