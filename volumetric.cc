#include "Mesh.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "Framebuffer.hpp"
#include "Demo.hpp"
#include <GL/glew.h>
#define NO_SDL_GLEXT
#include <SDL2/SDL.h>
#include <iostream>
#include <random>
#include <deque>
#include <boost/math/distributions/beta.hpp>

#define USE_OCTREE
//#define USE_GRASS

std::random_device rdev;
std::mt19937 rng(rdev());

#ifdef USE_OCTREE

void generate_impulse(Texture &tex, const IndexedMesh<LocTexNormVertex> &mesh,
    unsigned k, unsigned i, const AABB &bounds, const glm::vec3 &orientation, float minrad)
{
    std::uniform_real_distribution<> unif1(0.0f, 1.0f);
    std::uniform_real_distribution<> unif2(-1.0f, 1.0f);
    boost::math::beta_distribution<> bathtub(0.5, 0.5);

    glm::vec3 a = mesh.verts[mesh.indices[i]].loc;
    glm::vec3 na = mesh.verts[mesh.indices[i]].norm;
    glm::vec3 b = mesh.verts[mesh.indices[i+1]].loc;
    glm::vec3 nb = mesh.verts[mesh.indices[i+1]].norm;
    glm::vec3 c = mesh.verts[mesh.indices[i+2]].loc;
    glm::vec3 nc = mesh.verts[mesh.indices[i+2]].norm;
    glm::vec3 n = glm::normalize((na+nb+nc)/3.0f);

    glm::vec3 radius = bounds.radius();
    glm::vec3 loc = bounds.center() + minrad*glm::vec3(unif2(rng),unif2(rng),unif2(rng));
    loc -= n*dot(loc-a,n);

    glm::vec3 forward = glm::normalize(0.5f*n+(float)unif1(rng)*(b-a)+(float)unif1(rng)*(c-a));
    glm::vec3 side = glm::normalize(glm::cross(glm::vec3(0,1,0), forward));
    glm::vec3 up = glm::cross(forward, side);
    glm::mat3 rot = glm::mat3(forward, side, up);

#ifdef USE_GRASS
    glm::vec4 color = glm::vec4(0.55f,0.71f,0.0f,1.0f);
    glm::vec3 size = glm::vec3(100.0,2.0,2.0);
#else
    glm::vec4 color = glm::mix(
        glm::vec4(147.0f, 92.0f, 66.0f,255.0f),
        glm::vec4(67.0f, 35.0f, 27.0f,255.0f),
        (float)unif1(rng))/255.0f;
    glm::vec3 size = glm::vec3(100.0,2.0,2.0);
#endif

    tex.subimage({(6*k+0)%tex.width(),(6*k+0)/tex.width()},
        {1,1}, {{loc, 0}});
    tex.subimage({(6*k+1)%tex.width(),(6*k+1)/tex.width()},
        {1,1}, {{column(rot,0), 0}});
    tex.subimage({(6*k+2)%tex.width(),(6*k+2)/tex.width()},
        {1,1}, {{column(rot,1), 0}});
    tex.subimage({(6*k+3)%tex.width(),(6*k+3)/tex.width()},
        {1,1}, {{column(rot,2), 0}});
    tex.subimage({(6*k+4)%tex.width(),(6*k+4)/tex.width()},
        {1,1}, {{size, 0}});
    tex.subimage({(6*k+5)%tex.width(),(6*k+5)/tex.width()},
        {1,1}, {color});
}

int generate_impulses(Texture &tex, const IndexedMesh<LocTexNormVertex> &mesh, float radius = 0.2)
{
    int k = 0;
    std::deque<AABB> pending = {mesh.bound().squarify()};
    do {
        AABB bounds = pending.front(); pending.pop_front();
        if (auto idx = mesh.collides(bounds)) {
            if (bounds.radius().x < radius) {
                for (glm::vec3 disp : {
                        glm::vec3(0.0,0.0,0.0),
                        //glm::vec3(1.0,1.0,1.0),
                        //glm::vec3(1.0,-1.0,-1.0),
                        //glm::vec3(-1.0,1.0,-1.0),
                        //glm::vec3(-1.0,-1.0,1.0)
                    })
                for (int i = 0; i < 15; i++)
                    generate_impulse(tex, mesh, k+=1, *idx, bounds, disp, radius);
            } else {
                glm::vec3 c = bounds.center();
                glm::vec3 r = bounds.radius();
                glm::vec3 e1(r.x,0,0), e2(0,r.y,0), e3(0,0,r.z);
                for (glm::vec3 o : {
                    c-e1-e2-e3, c-e1-e2, c-e1-e3, c-e1,
                    c-e2-e3, c-e2, c-e3, c,
                })
                    pending.push_back(AABB{o,o+r});
            }
        }
    } while (!pending.empty());
    return k;
}

#else // USE_OCTREE

int generate_impulses(Texture &impulses, const IndexedMesh<LocTexNormVertex> &mesh)
{
    std::random_device rdev;
    std::mt19937 rng(rdev());
    std::uniform_real_distribution<> unif(0.0f, 1.0f);

    int count = 0;
    for (int i = 0; i < mesh.indices.size(); i += 3)
    for (int j = 0; j < 1; j++) {
        glm::vec3 a = mesh.verts[mesh.indices[i]].loc;
        glm::vec3 na = mesh.verts[mesh.indices[i]].norm;
        glm::vec3 b = mesh.verts[mesh.indices[i+1]].loc;
        glm::vec3 nb = mesh.verts[mesh.indices[i+1]].norm;
        glm::vec3 c = mesh.verts[mesh.indices[i+2]].loc;
        glm::vec3 nc = mesh.verts[mesh.indices[i+2]].norm;
        glm::vec3 n = glm::normalize((na+nb+nc)/3.0f);

        float t = unif(rng), s = unif(rng);
        if (s > 1-t)
            s = 1-s;  // Keep within triangle
        glm::vec3 pt = a + t*(b-a) + s*(c-a);
        
        glm::vec3 forward = glm::normalize(n+(2.0f*glm::vec3(unif(rng),unif(rng),unif(rng))-1.0f)*0.25f);
        glm::vec3 side = glm::normalize(glm::cross(glm::vec3(0,1,0), forward));
        glm::vec3 up = glm::cross(forward, side);
        glm::mat3 W = glm::mat3(forward, side, up);
        
        glm::vec4 colorA = glm::vec4(111, 78, 55, 196)/255.0f;
        glm::vec4 colorB = glm::vec4(92, 64, 51, 196)/255.0f;
        glm::vec4 color = mix(colorA,colorB, unif(rng));
        
        glm::vec3 size = glm::vec3(sqrt(unif(rng)*400.0f+100.0f),0.1,0.1);

        int k = count*6;
        impulses.subimage({k%impulses.width(),k/impulses.width()}, {1,1}, {{pt, 0}});
        impulses.subimage({(k+1)%impulses.width(),(k+1)/impulses.width()}, {1,1}, {{column(W,0), 0}});
        impulses.subimage({(k+2)%impulses.width(),(k+2)/impulses.width()}, {1,1}, {{column(W,1), 0}});
        impulses.subimage({(k+3)%impulses.width(),(k+3)/impulses.width()}, {1,1}, {{column(W,2), 0}});
        impulses.subimage({(k+4)%impulses.width(),(k+4)/impulses.width()}, {1,1}, {{size, 0}});
        impulses.subimage({(k+5)%impulses.width(),(k+5)/impulses.width()}, {1,1}, {color});
        count++;
    }
    return count;
}

#endif // USE_OCTREE

int main(int argc, char **argv)
{
    if (argc < 2) {
        std::cerr << "Usage: volumetric MESH" << std::endl;
        return EXIT_FAILURE;
    }
    Demo demo;
    demo.create();

    Shader solid;
    solid.create("../shaders/solid");
    Shader composite;
    composite.create("../shaders/composite");
    Shader accum;
    accum.create("../shaders/accum");

    IndexedMesh<LocTexNormVertex> teapot;
    if (!load_obj(teapot, argv[1])) {
        std::cerr << "Failed to load " << argv[1] << std::endl;
        return EXIT_FAILURE;
    }

    Texture impulses;
    impulses.create(4046, 4046);
    int count = generate_impulses(impulses, teapot);
    std::cout << count << std::endl;
    IndexedMesh<LocVertex> quad = makequad();

    Framebuffer accumbuf;
    accumbuf.channel(GL_COLOR_ATTACHMENT0)
        .create(1024,1024,GL_RGBA32F,GL_RGBA);
    accumbuf.channel(GL_COLOR_ATTACHMENT1)
        .create(1024,1024,GL_R8,GL_RED);
    accumbuf.channel(GL_DEPTH_ATTACHMENT)
        .create(1024,1024,GL_DEPTH_COMPONENT32,GL_DEPTH_COMPONENT);
    accumbuf.complete();

    teapot.rebuild();
    quad.rebuild();

    while (demo.poll()) {

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        // Draw opaque
        accumbuf.bind();
        glDepthMask(GL_TRUE);
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        solid.use();
        solid.uniform("mvp", demo.projection() * demo.modelview());
        solid.uniform("color", glm::vec4(1.0, 1.0, 1.0, 1.0));
        teapot.draw(GL_TRIANGLES);

        // Accumulate

        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);

        glBlendFunci(0, GL_ONE, GL_ONE);
        glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
        glBlendEquation(GL_FUNC_ADD);

        accumbuf.bind();
        glClearBufferfv(GL_COLOR, 0, glm::value_ptr(glm::vec4(0.0f))); 
        glClearBufferfv(GL_COLOR, 1, glm::value_ptr(glm::vec4(1.0f)));

        accum.use();
        accum.uniform("mvp", demo.projection() * demo.modelview());
        accum.uniform("proj", demo.projection());
        accum.uniform("invproj", glm::inverse(demo.projection()));
        accum.uniform("mv", demo.modelview());
        accum.uniform("impulses", impulses, GL_TEXTURE0);
        accum.uniform("impulses2", impulses, GL_TEXTURE1);
        accum.uniform("time", demo.time());
        quad.draw(GL_TRIANGLES, count);

        // Composite

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, 1024, 1024);
        glClearColor(1.0,1.0,1.0,1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        solid.use();
        solid.uniform("mvp", demo.projection() * demo.modelview());
        solid.uniform("color", glm::vec4(0.0, 0.0, 0.0, 1.0));
        teapot.draw(GL_TRIANGLES);

        glDisable(GL_DEPTH);
        glDepthFunc(GL_ALWAYS);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        composite.use();
        composite.uniform("accumbuf", accumbuf.channel(GL_COLOR_ATTACHMENT0), GL_TEXTURE0);
        composite.uniform("countbuf", accumbuf.channel(GL_COLOR_ATTACHMENT1), GL_TEXTURE1);
        quad.draw(GL_TRIANGLES);

        demo.swap();
    }
}
