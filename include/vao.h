#pragma once

#include <glad/glad.h>

// TODO: In future, expand this into a proper RAII class that
// handles different types of VAOs. For now, we just use it as
// a way to get a singleton dummy.
class VAO {
public:
    // OpenGL requires a VAO to be bound in order for the call not
    // to be discarded. We attach a dummy one, even though the
    // vertex data is hardcoded into the vertex shader. 
	static GLuint get_dummy() {
        static GLuint dummy_vao = 0;
        if (dummy_vao == 0) {
            glGenVertexArrays(1, &dummy_vao);
        }
        return dummy_vao;
	}
};
