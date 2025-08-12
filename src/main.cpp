#include <iostream>

#include "app.h"

const unsigned int SCREEN_WIDTH = 2560;
const unsigned int SCREEN_HEIGHT = 1440;
const unsigned int CANVAS_WIDTH = 8000;
const unsigned int CANVAS_HEIGHT = 8000;

int main() {
    
    try {
        auto app = App(
            SCREEN_WIDTH, SCREEN_HEIGHT, 
            CANVAS_WIDTH, CANVAS_HEIGHT
        );

        app.run();
    } catch (const std::runtime_error& e) {
        std::cerr << "Brush App failed for the following reason: " << e.what() << std::endl;
    }

    return 0;
}


