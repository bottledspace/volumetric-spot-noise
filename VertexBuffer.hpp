#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>


template <typename T>
struct VertexDescriptor {
    static void describe();
};

struct LocVertex {
    glm::vec3 loc;
};

template <>
struct VertexDescriptor<LocVertex> {
    static void describe() {
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            sizeof(float)*3, (void *)0);
    }
};

struct LocTexNormVertex {
    glm::vec3 loc;
    glm::vec2 uv;
    glm::vec3 norm;
    int id;
};

template <>
struct VertexDescriptor<LocTexNormVertex> {
    static void describe() {
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            sizeof(LocTexNormVertex), (void *)offsetof(LocTexNormVertex, loc));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
            sizeof(LocTexNormVertex), (void *)offsetof(LocTexNormVertex, uv));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
            sizeof(LocTexNormVertex), (void *)offsetof(LocTexNormVertex, norm));
        glEnableVertexAttribArray(3);
        glVertexAttribIPointer(3, 1, GL_INT,
            sizeof(LocTexNormVertex), (void *)offsetof(LocTexNormVertex, id));
    }
};

template <typename Vertex>
struct IndexedVertexBuffer {
    int count;
    GLuint vao;
    GLuint vbo;
    GLuint ibo;

    void recompile(const std::vector<Vertex> &verts,
                 const std::vector<unsigned> &indices) {
        if (!vao) {
            glGenBuffers(1, &vbo);
            glGenBuffers(1, &ibo);
            
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            VertexDescriptor<Vertex>::describe();
            
        }
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
            verts.size()*sizeof(Vertex),
            verts.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            indices.size()*sizeof(unsigned),
            indices.data(), GL_STATIC_DRAW);
        count = indices.size();
    }

    void draw(int primtype = GL_TRIANGLES) const {
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glDrawElements(primtype, count, GL_UNSIGNED_INT, (void *)0);
    }
    void draw(int ntimes, int primtype) const {
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glDrawElementsInstanced(primtype, count, GL_UNSIGNED_INT, (void *)0, ntimes);
    }
};

template <typename Vertex>
struct VertexBuffer {
    int count;
    GLuint vao;
    GLuint vbo;

    void recompile(const std::vector<Vertex> &verts) {
        if (!vao) {
            glGenBuffers(1, &vbo);
            
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            VertexDescriptor<Vertex>::describe();
        }
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
            verts.size()*sizeof(Vertex),
            verts.data(), GL_STATIC_DRAW);
        count = verts.size();
    }

    void draw(int primtype = GL_TRIANGLES) const {
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glDrawArrays(primtype, 0, count);
    }
    void draw(int ntimes, int primtype) const {
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glDrawArraysInstanced(primtype, 0, count, ntimes);
    }
};