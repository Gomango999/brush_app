#include "app.hpp"

const unsigned int SCREEN_WIDTH = 800;
const unsigned int SCREEN_HEIGHT = 800;
const unsigned int CANVAS_WIDTH = 8000;
const unsigned int CANVAS_HEIGHT = 8000;

int main() {

    auto app = App(
        SCREEN_WIDTH, SCREEN_HEIGHT, 
        CANVAS_WIDTH, CANVAS_HEIGHT
    );

    app.run();
    
    return 0;
}


