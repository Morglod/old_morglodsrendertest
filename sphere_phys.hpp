#pragma once

#include <glm/glm.hpp>
#include <mu/Containers.hpp>

namespace phys {


class Sphere {
public:
    float r;
    glm::vec3 pos;
    glm::vec3 vel;
    float density;

    inline float OverlapStr(Sphere const& other) const {
        return glm::max((glm::distance(pos, other.pos) - r - other.r), 0);
    }

    inline glm::vec3 Proc(Sphere const& other) const {
        glm::vec3 nvel = (pos - other.pos) * OverlapStr(other) * density * other.density;
        vel = std::mix(vel, nvel);
    }
};

class World {
public:
    mu::ArrayHandle<Sphere> spheres;
    float vel_reduce = 0.2f;

    inline void Update(float const& delta) {
        Sphere* ar = spheres.GetArray();
        for(size_t i = 0; i < spheres.GetNum(); ++i) {
            for(size_t j = 0; j < spheres.GetNum(); ++j) {
                if(i == j) continue;
                ar[i].Proc(ar[j]);
                ar[i].pos += ar[i].vel * delta;
                ar[i].vel -= vel_reduce * delta;
            }
        }
    }
};

}
