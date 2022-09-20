#pragma once
#define GLM_FORCE_PURE
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/string_cast.hpp>
#include <tuple>
#include <vector>
#include <array>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <optional>
#include <map>

extern "C"{
    int triBoxOverlap(float boxcenter[3], float boxhalfsize[3], float triverts[3][3]);
};

struct AABB {
    glm::vec3 min;
    glm::vec3 max;

    glm::vec3 center() const { return (min+max)/2.0f; }
    glm::vec3 radius() const { return (max-min)/2.0f; }
    AABB squarify() const;
};

AABB AABB::squarify() const {
    auto c = center();
    auto r = radius();
    r.x = r.y = r.z = std::max({r.x,r.y,r.z});
    return AABB{c-r,c+r};
}

template <typename Vertex>
class IndexedMesh {
public:
    unsigned push_vert(const Vertex &v) {
        return (unsigned)std::distance(verts.begin(), verts.insert(verts.end(), v));
    }
    void push_face(std::initializer_list<unsigned> &&indices) {
        this->indices.insert(this->indices.end(), indices);
    }

    void rebuild();

    void draw(GLenum primitive) const;
    void draw(GLenum primitive, int count) const;

    AABB bound() const;
    std::optional<unsigned> collides(const AABB &bound) const;

    IndexedMesh()
        : vao(0), vbo(0), ibo(0)
        {}
    IndexedMesh(std::vector<unsigned> &&indices, std::vector<Vertex> &&verts)
        : indices(std::move(indices)), verts(std::move(verts)),
          vao(0), vbo(0), ibo(0)
        {}

    std::vector<unsigned> indices;
    std::vector<Vertex> verts;
private:
    GLuint vao;
    GLuint vbo;
    GLuint ibo;
};

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
bool load_obj(IndexedMesh<Vertex> &mesh, const char *fname) {
    std::ifstream is{fname};
    if (!is)
        return false;

    std::string linebuf;
    std::vector<glm::vec3> locs;
    std::vector<glm::vec2> texs;
    std::vector<glm::vec3> norms;
    std::map<std::tuple<unsigned,unsigned,unsigned>,unsigned> verts;
    while (std::getline(is, linebuf)) {
        std::stringstream ls{linebuf};
        std::string cmd;

        if (!(ls >> cmd))
            continue;  // Ignore empty lines
        if (cmd == "vt") {
            float x, y;
            ls >> x >> y;
            texs.push_back({x, y});
        }
        else if (cmd == "vn") {
            float x, y, z;
            ls >> x >> y >> z;
            norms.push_back({x, y, z});
        }
        else if (cmd == "v") {
            float x, y, z;
            ls >> x >> y >> z;
            locs.push_back({x, y, z});
        }
        else if (cmd == "f") {
            unsigned a,b,c;
            while (ls >> a) {
                if (ls.get() == '/') {
                    ls >> b;
                    ls.clear();
                    if (ls.get() == '/')
                        ls >> c;
                }

                // Combine the location and normal to form a vertex, and
                // insert if not already added.
                auto it = verts.find({a-1, b-1, c-1});
                if (it == verts.end()) {
                    unsigned v = mesh.push_vert({locs[a-1], texs[b-1], norms[c-1], -1});
                    it = verts.insert({{a-1, b-1, c-1}, v}).first;
                }
                mesh.indices.push_back(it->second);
            }
        }
        else
            continue;  // Silently ignore unrecognized commands
    }
    return true;
}

IndexedMesh<LocVertex> makequad() {
    std::vector<LocVertex> verts {
        {glm::vec3(-1,-1,0)},
        {glm::vec3(-1,1,0)},
        {glm::vec3(1,1,0)},
        {glm::vec3(1,-1,0)}
    };
    std::vector<unsigned> indices {
        0,1,2,
        0,2,3
    };
    return IndexedMesh<LocVertex>(std::move(indices),std::move(verts));
}

IndexedMesh<LocVertex> cube() {
    std::vector<LocVertex> verts {
        {glm::vec3(1.000000, 1.000000, -1.000000)},
        {glm::vec3(1.000000, -1.000000, -1.000000)},
        {glm::vec3(1.000000, 1.000000, 1.000000)},
        {glm::vec3(1.000000, -1.000000, 1.000000)},
        {glm::vec3(-1.000000, 1.000000, -1.000000)},
        {glm::vec3(-1.000000, -1.000000, -1.000000)},
        {glm::vec3(-1.000000, 1.000000, 1.000000)},
        {glm::vec3(-1.000000, -1.000000, 1.000000)},
    };
    std::vector<unsigned> indices {
        6-1, 8-1,
        2-1, 6-1,
        1-1, 2-1,
        8-1, 7-1,
        3-1, 4-1,
        5-1, 6-1,
        3-1, 7-1,
        1-1, 3-1,
        8-1, 4-1,
        7-1, 5-1,
        5-1, 1-1,
        4-1, 2-1,
    };
    return IndexedMesh<LocVertex>(std::move(indices),std::move(verts));
}

template <typename Vertex>
void IndexedMesh<Vertex>::rebuild() {
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
}

template <typename Vertex>
void IndexedMesh<Vertex>::draw(GLenum primitive) const {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glDrawElements(primitive, indices.size(), GL_UNSIGNED_INT, (void *)0);
}

template <typename Vertex>
void IndexedMesh<Vertex>::draw(GLenum primitive, int count) const {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glDrawElementsInstanced(primitive, indices.size(), GL_UNSIGNED_INT, (void *)0, count);
}

template <typename Vertex>
AABB IndexedMesh<Vertex>::bound() const {
    AABB result;
    for (int i = 0; i < verts.size(); i++) {
        const glm::vec3 &pt = verts[i].loc;
        result.min = glm::vec3{
            std::min(result.min.x, pt.x),
            std::min(result.min.y, pt.y),
            std::min(result.min.z, pt.z)};
        result.max = glm::vec3{
            std::max(result.max.x, pt.x),
            std::max(result.max.y, pt.y),
            std::max(result.max.z, pt.z)};
    }
    return result;
}

template <typename Vertex>
std::optional<unsigned>
IndexedMesh<Vertex>::collides(const AABB &bound) const {
    glm::vec3 center = bound.center();
    glm::vec3 radius = bound.radius();
    for (size_t i = 0; i < indices.size(); i+=3) {
        float triverts[][3] = {
            verts[indices[i+0]].loc.x, verts[indices[i+0]].loc.y, verts[indices[i+0]].loc.z,
            verts[indices[i+1]].loc.x, verts[indices[i+1]].loc.y, verts[indices[i+1]].loc.z,
            verts[indices[i+2]].loc.x, verts[indices[i+2]].loc.y, verts[indices[i+2]].loc.z
        };
        if (triBoxOverlap(glm::value_ptr(center), glm::value_ptr(radius), triverts))
            return std::optional<size_t>(i);
    }
    return std::nullopt;
}