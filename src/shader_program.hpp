#ifndef SHADER_PROG_HPP
#define SHADER_PROG_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

using namespace std;

class ShaderProgram {
    private:
        static GLuint currentlyUsedProgram; 
        GLuint m_shaderProgram = 0;
        vector<GLuint> m_shaders = {};
        vector<const char*> m_paths = {};
        vector<int> m_types = {};
        string m_name;

        fs::path extractPath(const string& line);
        string getShaderSource(const char *path);

    public:
        ShaderProgram();
        GLuint id();
        void create();
        void load(int type, const char *path);
        void reload();
        void link();
        void use();
        void dispatch(GLuint x = 1, GLuint y = 1, GLuint z = 1);
        void destroy();
        static GLuint getVarLoc(const string& name);

        template<typename T>
        static tuple<GLuint, GLuint, GLuint> 
        addData(const vector<T>& data, const vector<GLuint>& indices){
            GLuint VBObj, VAObj, EBObj;
            
            glGenVertexArrays(1, &VAObj);
            glBindVertexArray(VAObj);
            
            glGenBuffers(1, &VBObj);
            glBindBuffer(GL_ARRAY_BUFFER, VBObj);
            glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(T), data.data(), GL_STATIC_DRAW);
            
            glGenBuffers(1, &EBObj);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBObj);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

            return {VBObj, VAObj, EBObj};
        }

        static void linkData(int numCoords, int typesize, int layout, int glType = GL_FLOAT, int normalize = GL_FALSE){
            glVertexAttribPointer(layout, numCoords, glType, (GLboolean)normalize, numCoords * typesize, (void*)0);
            glEnableVertexAttribArray(layout);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }
};

#endif