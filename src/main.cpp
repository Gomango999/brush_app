#include <iostream>

#include "app.h"

const unsigned int SCREEN_WIDTH = 2560;
const unsigned int SCREEN_HEIGHT = 1440;
const unsigned int CANVAS_WIDTH = 6500;
const unsigned int CANVAS_HEIGHT = 10000;
const unsigned int CANVAS_DISPLAY_WIDTH = 1200;
const unsigned int CANVAS_DISPLAY_HEIGHT = 1200;

int main() {
    
    try {
        auto app = App(
            SCREEN_WIDTH, SCREEN_HEIGHT,
            CANVAS_WIDTH, CANVAS_HEIGHT,
            CANVAS_DISPLAY_WIDTH, CANVAS_DISPLAY_HEIGHT
        );

        app.run();
    } catch (const std::runtime_error& e) {
        std::cerr << "Brush App failed for the following reason: " << e.what() << std::endl;
    }

    return 0;
}


