#include "PostProcess.hpp"
#include <fstream>
#include <iostream>

std::string PostProcess::readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "Error: Could not open shader file: " << path << std::endl;
        return "";
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return content;
}

GLuint PostProcess::compileShader(const std::string& source, GLenum type) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cout << "Shader compilation failed: " << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool PostProcess::linkProgram(GLuint vertShader, GLuint fragShader, GLuint& program) {
    program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cout << "Program linking failed: " << infoLog << std::endl;
        glDeleteProgram(program);
        return false;
    }
    glDetachShader(program, vertShader);
    glDetachShader(program, fragShader);
    return true;
}

bool PostProcess::init(int w, int h) {
    width = w;
    height = h;

    // Create 4 temp FBOs and textures for chaining passes
    const int numTemps = 4;
    tempFBOs.resize(numTemps);
    tempTexs.resize(numTemps);
    glGenFramebuffers(numTemps, tempFBOs.data());
    glGenTextures(numTemps, tempTexs.data());

    for (int i = 0; i < numTemps; ++i) {
        glBindTexture(GL_TEXTURE_2D, tempTexs[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindFramebuffer(GL_FRAMEBUFFER, tempFBOs[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tempTexs[i], 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "Error: Framebuffer " << i << " is not complete!" << std::endl;
            return false;
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Create full-screen quad VBO
    float vertices[] = {
        -1.0f, -1.0f, 0.0f, 0.0f,  // Bottom-left
         1.0f, -1.0f, 1.0f, 0.0f,  // Bottom-right
        -1.0f,  1.0f, 0.0f, 1.0f,  // Top-left
         1.0f,  1.0f, 1.0f, 1.0f   // Top-right
    };
    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    return true;
}

void PostProcess::addPass(const std::string& vertPath, const std::string& fragPath, const std::string& name) {
    // Read shader source files
    std::string vertSource = readFile(vertPath);
    std::string fragSource = readFile(fragPath);
    if (vertSource.empty() || fragSource.empty()) {
        std::cout << "Error: Failed to read shader files for pass '" << name << "'" << std::endl;
        return;
    }

    // Compile vertex shader
    GLuint vertShader = compileShader(vertSource, GL_VERTEX_SHADER);
    if (!vertShader) {
        std::cout << "Error: Failed to compile vertex shader for pass '" << name << "'" << std::endl;
        return;
    }

    // Compile fragment shader
    GLuint fragShader = compileShader(fragSource, GL_FRAGMENT_SHADER);
    if (!fragShader) {
        std::cout << "Error: Failed to compile fragment shader for pass '" << name << "'" << std::endl;
        glDeleteShader(vertShader);
        return;
    }

    // Create program and attach shaders
    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);

    // Bind attribute locations (for compatibility, but we'll check actual locations)
    glBindAttribLocation(program, 0, "aPos");
    glBindAttribLocation(program, 1, "aTex");

    // Link the program
    if (!linkProgram(vertShader, fragShader, program)) {
        std::cout << "Error: Failed to link program for pass '" << name << "'" << std::endl;
        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
        glDeleteProgram(program);
        return;
    }

    // Detach and delete shaders
    glDetachShader(program, vertShader);
    glDetachShader(program, fragShader);
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    // Create the pass struct
    PostProcessPass pass;
    pass.program = program;
    pass.name = name;

    // Get actual attrib locations (critical for GLES 2.0)
    pass.posLoc = glGetAttribLocation(program, "aPos");
    pass.texLoc = glGetAttribLocation(program, "aTex");
    //std::cout << "Pass '" << name << "': aPos loc=" << pass.posLoc << ", aTex loc=" << pass.texLoc << std::endl;
    if (pass.posLoc == -1 || pass.texLoc == -1) {
        std::cout << "Error: Attrib locations not found for pass '" << name << "'" << std::endl;
        glDeleteProgram(program);
        return;
    }

    // Cache uniform locations
    pass.uniforms["tex"] = glGetUniformLocation(program, "tex");
    pass.uniforms["time"] = glGetUniformLocation(program, "time");

    // Add to passes vector
    passes.push_back(pass);
    //std::cout << "Added post-process pass: " << name << std::endl;
}

void PostProcess::setUniform(const std::string& passName, const std::string& uniformName, float value) {
    for (auto& pass : passes) {
        if (pass.name == passName) {
            auto it = pass.uniforms.find(uniformName);
            if (it != pass.uniforms.end()) {
                glUseProgram(pass.program);
                glUniform1f(it->second, value);
            }
        }
    }
}

GLuint PostProcess::process(GLuint inputTex) {
    if (passes.empty()) return inputTex;

    //std::cout << "Processing " << passes.size() << " passes" << std::endl;  // Debug: Should be 1
    GLuint currentTex = inputTex;
    for (size_t i = 0; i < passes.size(); ++i) {
        glBindFramebuffer(GL_FRAMEBUFFER, tempFBOs[i]);
        glViewport(0, 0, width, height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // Ensure clear color is set
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(passes[i].program);
        GLenum err = glGetError();
        if (err) std::cout << "GL Error after glUseProgram: " << err << std::endl;

        glUniform1i(passes[i].uniforms["tex"], 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, currentTex);

        // Draw quad using actual attrib locations
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glEnableVertexAttribArray(passes[i].posLoc);
        glVertexAttribPointer(passes[i].posLoc, 2, GL_FLOAT, GL_FALSE, 16, (void*)0);
        glEnableVertexAttribArray(passes[i].texLoc);
        glVertexAttribPointer(passes[i].texLoc, 2, GL_FLOAT, GL_FALSE, 16, (void*)(8));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        err = glGetError();
        if (err) std::cout << "GL Error after glDrawArrays: " << err << std::endl;

        currentTex = tempTexs[i];
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //std::cout << "Returning processed texture ID: " << currentTex << std::endl;
    return currentTex;
}

void PostProcess::shutdown() {
    for (auto& pass : passes) {
        glDeleteProgram(pass.program);
    }
    passes.clear();

    glDeleteFramebuffers(tempFBOs.size(), tempFBOs.data());
    glDeleteTextures(tempTexs.size(), tempTexs.data());
    glDeleteBuffers(1, &quadVBO);
}