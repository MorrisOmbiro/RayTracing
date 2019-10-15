#ifndef Tracer_h
#define Tracer_h

#include <cmath>
#include "Objects.h"
#include <vector>
#include <iostream>
#include <typeinfo>
#include <algorithm>
using namespace std;

// maybe ray_cast is better name?
class Ray_Tracer {
public:
    double shadow(Light light, _3d_values intersection_point, vector<Objects*> objects, float &k, int type_light) {
        int hit_count = 0;
        double no_hit = 0;
        int bundle_of_rays = 1; // samples
        // shadow ray from the ray/surface intersection point towards the light source(s)
        Objects* obj;
        for(Objects* object: objects) {
            obj = object;
            for (int i = 0; i < bundle_of_rays; i++) {
                // try to approximate, including negative numbers so power -1
                _3d_values rand_point(.1 * (rand()% 100), .1 * (rand()% 100), .1 * (rand()% 100));
                // new = old + Epsilon*N
                _3d_values rand_vec = (rand_point - obj->get_center()).normalize() * obj->get_radius();
                if(type_light == 0) { // directional light
                    // ray direction is negative of the light direction
                    _3d_values shadow_ray = light.get_pos() * -1;// - (rand_vec + obj->get_center());
                    Ray_Vector ray(intersection_point, shadow_ray);
                    if (obj->intersect(ray, k) && k > 0)
                        hit_count++;
                }else {
                    _3d_values shadow_ray = light.get_pos() - (rand_vec + obj->get_center());
                    Ray_Vector ray(intersection_point, shadow_ray);
                    if (obj->intersect(ray, k) && k > 0) // in shadow
                        hit_count++;
                }
            }
        }
        no_hit = hit_count > no_hit ? hit_count : no_hit;
        return 1-no_hit;
    }

    /*This checks for where there was an intersetion, and if so, get an approximate color (was not needed for this homework)
     * Without finding the normal at a point on a sphere, then we could just have returrned the regular sphere color
     * but that would just look likea a cricle*/
    _3d_values Shade_Ray(float width, float height, Ray_Vector ray, float &t, _3d_values background_color,
                         vector<Objects*> objects, _3d_values** a, _3d_values view_dir,
                         vector<Light> lights, vector<AttenuationLight> att_lights) {
        _3d_values color = background_color * view_dir.cal_angle(ray.get_direction());
        vector<_3d_values> summation;
        // Sphere s(_3d_values(0, 0, 0), 0, _3d_values(0, 0, 0), _3d_values(0, 0, 0), 0, 0, 0,0);
        Objects* o;
        float t_min = 10000000; // infinite

        // check for intersection and if so then return the color of the element
        for(Objects* obj: objects) {
            if (obj->intersect(ray, t)) {
                if(t_min > t) {
                    o = obj;
                    t_min = t;
                }
                _3d_values ray_dir = ray.get_direction();
                _3d_values POI = ray.get_origin() + ray_dir * t; // ray equation
                float ka = o->get_ka();
                float kd = o->get_kd();
                float ks = o->get_ks();
                float n = o->get_n();
                _3d_values sum(0,0,0);
                _3d_values temp_color(0,0,0);
                double attn, shadow_strength;
                // check which of the two are empty lights or atten_lights
                // get values dor L, H, N and apply phong model equation
                if(lights.size() == 0) {
                    for (AttenuationLight l: att_lights) { // perform summation on Phong illumination equation
                        _3d_values L(0,0,0);
                        float d;
                        if(l.get_w() != 0) {
                            L = (l.get_pos() - POI);
                            d = L.magnitude();
                            L = L.normalize();
                            attn = 1 / (l.get_c1() + (d * l.get_c2()) + (pow(d, 2) * (l.get_c3())));
                        }else {
                            L = l.get_pos().normalize(); // constant - directional light
                            attn = 1;
                        }
                        _3d_values N = o->get_normal(POI).normalize();
                        _3d_values H = (L + ray.get_origin()).normalize();
                        // normalize view_dir and normal from the sphere
                        ray_dir = ray_dir.normalize();
                        _2d_values tex_coords = o->get_texture_coords(POI, int(width), int(height));
                        _3d_values c = a[tex_coords.X][tex_coords.Y];
                        // sum the part of the Phong model that will be added to ka*Od(lambda)
                        sum = sum + ((c * kd * fmax(0, (N.dot(L)))) +
                                     o->get_light() * ks * pow(fmax(0, N.dot(H)), n)) * attn * shadow_strength;
                    }
                }else {
                    _3d_values dir_L = (lights[0].get_pos() - POI).normalize();
                    for (Light l: lights) { // perform summation on Phong illumination equation
                        _3d_values L(0,0,0);
                        if(l.get_w() != 0) {
                            L = (POI - l.get_pos()).normalize();
                        }else {
                            L = l.get_pos().normalize(); // constant - directional light
                        }
                        _3d_values N = o->get_normal(POI).normalize(); // * -1;
                        _3d_values H = (L + ray.get_origin()).normalize();
                        float d = L.magnitude();
                        // normalize view_dir and normal from the sphere
                        ray_dir = ray_dir.normalize();
                        // N = N.normalize();
                        shadow_strength = shadow(l, POI, objects, t, l.get_w());
                        _2d_values tex_coords = o->get_texture_coords(POI, int(width), int(height));
                        _3d_values c = a[tex_coords.X][tex_coords.Y];
                        // cout << c.X << " " << c.Y << " " << c.Z << endl;
                        sum = sum + ((c * kd * fmax(0, (N.dot(L)))) +
                                     o->get_light() * ks * pow(fmax(0, N.dot(H)), n)) * shadow_strength;

                    }
                }
                /*Triangle* triangle = new Triangle();
                 if(typeid(*obj) == typeid(*triangle)) {*/
                 _2d_values tex_coords = o->get_texture_coords(POI, int(width), int(height));
                 _3d_values c = a[tex_coords.X][tex_coords.Y];
                 temp_color = (c * ka) + sum;
                // clamp Iλ values to 1
                color.X = fmin(1, temp_color.X/255)*255;color.Y = fmin(1, temp_color.Y/255)*255;color.Z = fmin(1, temp_color.Z/255)*255;
            }
        }
        return color;
    }

    /*This should be able to return either the background color or the sphere color based on whether the intersection happens*/
    _3d_values Trace_Ray(float width, float height, _3d_values ray_origin, _3d_values ray_direction, _3d_values background_color,
                         vector<Objects*> objects, _3d_values** a, _3d_values view_dir, vector<Light> light, vector<AttenuationLight> att_lights) {
        float t; // moving variance of ray
        // Now the ray has an origin and a direction to point towards in the darkness in hopes of hitting a sphere
        Ray_Vector ray(ray_origin, ray_direction);
        return Shade_Ray(width, height, ray, t, background_color, objects, a, view_dir, light, att_lights);
    }
};
#endif