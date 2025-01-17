LIB_DIR := -L/usr/local/Cellar/glew/2.1.0/lib
INCLUDES := -I/usr/local/Cellar/glew/2.1.0/include -Iinclude

LIBS := -lglew
FRAMEWORKS := -framework OpenGL -framework GLUT

SRC_DIR := src
OBJ_DIR := build
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))
CPPFLAGS := -DGL_SILENCE_DEPRECATION $(INCLUDES)

project: $(OBJ_FILES)
	g++ $(FRAMEWORKS) $(LIB_DIRS) $(LIBS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	g++ $(CPPFLAGS) -c -o $@ $<
	
clean:
	rm build/*.o project