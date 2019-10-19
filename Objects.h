#ifndef Objects_h
#define Objects_h

#include <math.h>
#include "_3d_values.h"
#include "ray_config.h"

/*Contains all the elements that will be added to the 3d space*/
class Objects {
public:
    virtual bool intersect(Ray_Vector& ray, float &t) {}
    virtual _3d_values get_normal(_3d_values vector) {}
    virtual _3d_values get_color(){}
    virtual _3d_values get_light() {}
    virtual _3d_values get_center() {}
    virtual float get_radius() {}
    virtual float get_ka() {}
    virtual float get_kd() {}
    virtual float get_ks() {}
    virtual float get_n() {}
    virtual _2d_values get_texture_coords(_3d_values vector, int w, int h) {}
    virtual _2d_values get_vt1() {}
    virtual _2d_values get_vt2() {}
    virtual _2d_values get_vt3() {}
    virtual int get_texture() {}
    virtual int get_width() {}
    virtual int get_height() {}
};

/*Object element: sphere */
class Sphere : public Objects{
// Encapsulate the actual variable elements, use getters, setters are redundant for now
private:
    _3d_values Center, Color, Spec_Light;
    float R, KA, KD, KS, N;
    int which_texture, ppm_height, ppm_width;
public:
    // set constructor for default
    Sphere(_3d_values center, float radius, _3d_values color, _3d_values spec_light, float ka, float kd, float ks, float n, int w, int p_h, int p_w) :
    Center(center), R(radius), Color(color), Spec_Light(spec_light), KA(ka), KD(kd), KS(ks), N(n), which_texture(w), ppm_height(p_h), ppm_width(p_w) {}

    // return color
    _3d_values get_color(){
        return Color;
    }
    _3d_values get_light() {
        return Spec_Light;
    }
    _3d_values get_center() {
            return Center;
    }
    float get_radius() {
        return R;
    }
    float get_ka() {
        return KA;
    }
    float get_kd() {
        return KD;
    }
    float get_ks() {
        return KS;
    }
    float get_n() {
        return N;
    }

    int get_texture() {
        return which_texture;
    }

    int get_width() {
        return ppm_width;
    }

    int get_height() {
        return ppm_height;
    }

    // return the normal from a point on the sphere without using partial derivatives
    _3d_values get_normal(_3d_values POI) {
        return (POI - get_center()) * (1/R);
    }

    _2d_values get_texture_coords(_3d_values poi, int height, int width) {
        _3d_values c = get_center();
        float r = get_radius();
        float phi = acos((poi.Z-c.Z)/r);
        float theta = atan((poi.Y-c.Y)/(poi.X-c.X));
        float u = 0;
        if(theta < 0) {
            theta += 2*M_PI;
            u =(float) (theta/(2*M_PI)*(width-1));
        }else
            u = (float) ((theta/(2*M_PI))*(width-1));
        auto v = (float)((phi/M_PI)*(height-1));
        return _2d_values(u, v);
    }
    // function that checks for ray and sphere intersectiopn based on simple quadratic function
    bool intersect(Ray_Vector& ray, float &t) {
        _3d_values eye = ray.get_origin();                                              // This is based on mapping from points in viewing window
        _3d_values view_dir = ray.get_direction().normalize();                                     // This is constant from file
        _3d_values x0minusxC = eye - get_center();                                          // x0-xc part of equation eye-center of sphere
        float a = 1;                                                                 // A= (xd2+ yd2 +zd2) = 1
        float b = 2 * (x0minusxC.dot(view_dir));                                      // B= 2×(xd×(x0–xc) + yd ×(y0–yc) + zd×(z0–zc))
        float c = x0minusxC.dot(x0minusxC) - float(pow(get_radius(), 2));                      // C= (x0–xc)2 +(y0–yc)2 +(z0–zc)2 –r2
        auto disc = float(pow(b, 2) - 4*c*a);                                      // discriminant

        // Ignore second intersection of sphere (2 solution cases)
        t = ((-b - sqrt(disc))/2 < (-b + sqrt(disc))/2) ? (-b - sqrt(disc))/2 : (-b + sqrt(disc))/2;
        // did we intersect, 0+ yes, less no
        return (disc >= 0);
    }
};

// Light object
class Light : public Objects {
private:
    _3d_values Light_Pos, Light_Col;
    float W;
public :
    Light(_3d_values light_pos, float w, _3d_values light_col): Light_Col(light_col), W(w), Light_Pos(light_pos) {}
    _3d_values get_pos() {
        return Light_Pos;
    }
    _3d_values get_col() {
        return Light_Col;
    }
    float get_w() {
        return W;
    }
};

// AttenuationLight object
class AttenuationLight : public Objects {
private:
    _3d_values Light_Pos, Light_Col;
    float W, C1, C2, C3;
public :
    AttenuationLight(_3d_values light_pos, float w, _3d_values light_col, float c1, float c2, float c3):
    Light_Col(light_col), W(w), Light_Pos(light_pos), C1(c1), C2(c2), C3(c3) {}
    _3d_values get_pos() {
        return Light_Pos;
    }
    _3d_values get_col() {
        return Light_Col;
    }
    float get_w() {
        return W;
    }
    float get_c1() {
        return C1;
    }
    float get_c2() {
        return C2;
    }
    float get_c3() {
        return C3;
    }
};
class Plane : public Objects {
private:
    // three points degining a triangle's vertices
    _3d_values Vertex1, Vertex2, Vertex3;
public:
    Plane(_3d_values v1, _3d_values v2, _3d_values v3) : Vertex1(v1), Vertex2(v2), Vertex3(v3) {}
    _3d_values get_plane_v1() {
        return Vertex1;
    }
    _3d_values get_plane_v2() {
        return Vertex2;
    }
    _3d_values get_plane_v3() {
        return Vertex3;
    }

    _3d_values get_e2() { // p2 - p0
        return get_plane_v3() - get_plane_v1();
    }
    _3d_values get_e1() { // p1 - p0
        return get_plane_v2() - get_plane_v1();
    }

    _3d_values get_plane_normal() {
        return get_e1().cross(get_e2());
    }

    bool intersect(Ray_Vector& ray, float &t) {
        double A = get_plane_normal().X;
        double B = get_plane_normal().Y;
        double C = get_plane_normal().Z;
        double D = -(A*get_plane_v1().X + B*get_plane_v1().Y + C*get_plane_v1().Z);

        if((A*ray.get_direction().X + B*ray.get_direction().Y + C*ray.get_direction().Z) != 0) {
            t = -(A * ray.get_origin().X + B * ray.get_origin().Y + C * ray.get_origin().Z + D) /
                (A * ray.get_direction().X + B * ray.get_direction().Y + C * ray.get_direction().Z);
        }
        return t > 0;
    }
};

class Triangle : public Objects {
private:
    _3d_values Vertex1, Vertex2, Vertex3, N_Vertex1, N_Vertex2, N_Vertex3, Face, Color, Spec_Light;
    _2d_values V_T1, V_T2, V_T3;
    float KA, KD, KS, N; // phong model
    int which_texture, ppm_height, ppm_width;
public:
    Triangle() {}
    Triangle(_3d_values v1, _3d_values v2, _3d_values v3, _3d_values nv1, _3d_values nv2,
            _3d_values nv3,_3d_values f, _3d_values c, _3d_values s, float ka, float kd,
            float ks, float n, _2d_values v_t1, _2d_values v_t2, _2d_values v_t3, int w, int p_h, int p_w) :
    Vertex1(v1), Vertex2(v2), Vertex3(v3), N_Vertex1(nv1), N_Vertex2(nv2), N_Vertex3(nv3),Face(f), Color(c), Spec_Light(s),
    KA(ka), KD(kd),KS(ks), N(n), V_T1(v_t1), V_T2(v_t2), V_T3(v_t3), which_texture(w), ppm_height(p_h), ppm_width(p_w) {}

    _3d_values get_v1() {
        return Vertex1;
    }
    _3d_values get_v2() {
        return Vertex2;
    }
    _3d_values get_v3() {
        return Vertex3;
    }
    _3d_values get_nv1() {
        return N_Vertex1;
    }
    _3d_values get_nv2() {
        return N_Vertex2;
    }
    _3d_values get_nv3() {
        return N_Vertex3;
    }
    _3d_values get_face() {
        return Face;
    }
    _3d_values get_color() {
        return Color;
    }
    _3d_values get_light() {
        return Spec_Light;
    }
    float get_ka() {
        return KA;
    }
    float get_kd() {
        return KD;
    }
    float get_ks() {
        return KS;
    }
    float get_n() {
        return N;
    }
    int get_texture() {
        return which_texture;
    }

    int get_width() {
        return ppm_width;
    }

    int get_height() {
        return ppm_height;
    }

    _2d_values get_vt1() {
        return V_T1;
    }_2d_values get_vt2() {
        return V_T2;
    }_2d_values get_vt3() {
        return V_T3;
    }

    _2d_values get_texture_coords(_3d_values poi, int height, int width) {
        _3d_values v1 = get_v1();
        _3d_values v2 = get_v2();
        _3d_values v3 = get_v3();

        _3d_values nv1 = get_nv1();
        _3d_values nv2 = get_nv2();
        _3d_values nv3 = get_nv3();

        _2d_values vt1 = get_vt1();
        _2d_values vt2 = get_vt2();
        _2d_values vt3 = get_vt3();
        float L1 = (v2 - v1).magnitude();
        float L2 = (v3 - v2).magnitude();
        float L3 = (v1 - v3).magnitude();
        // area of triangle
        float Area = sqrt((L1 + L2 + L3) * (-L1 + L2 + L3) * (L1 - L2 + L3) * (L1 + L2 - L3)) / 4;
        float a = ((poi-v2).cross(poi-v3)).magnitude()/2;
        float b = ((poi-v3).cross(v1-v3)).magnitude()/2;
        float c = ((v2-v1).cross(poi-v2)).magnitude()/2;
        float alpha = a/Area;
        float beta = b/Area;
        float gamma = c/Area;
        float u, v;
        // DON'T CAST TO INT NO MATTER WHAT
        u = (alpha*vt1.X + beta*vt2.X + gamma *vt3.X)*(width-1);
        v = (alpha*vt1.Y + beta*vt2.Y + gamma *vt3.Y)*(height-1);
        return _2d_values(u, v);
    }
    
    _3d_values get_normal(_3d_values poi) {
        _3d_values v1 = get_v1();
        _3d_values v2 = get_v2();
        _3d_values v3 = get_v3();

        _3d_values nv1 = get_nv1();
        _3d_values nv2 = get_nv2();
        _3d_values nv3 = get_nv3();


        double L1 = (v2 - v1).magnitude();
        double L2 = (v3 - v2).magnitude();
        double L3 = (v1 - v3).magnitude();
        // area of triangle
        double Area = sqrt((L1 + L2 + L3) * (-L1 + L2 + L3) * (L1 - L2 + L3) * (L1 + L2 - L3)) / 4;
        double a = ((poi-v2).cross(poi-v3)).magnitude()/2;
        double b = ((poi-v3).cross(v1-v3)).magnitude()/2;
        double c = ((v2-v1).cross(poi-v2)).magnitude()/2;
        double alpha = a/Area;
        double beta = b/Area;
        double gamma = c/Area;

        // smooth shading
        return ((nv1*alpha + nv2*beta + nv3*gamma)*(1/(nv1*alpha + nv2*beta + nv3*gamma).magnitude()));
    }

    // barycentric coordinates
    bool intersect(Ray_Vector& ray, float &t) {
        // calculate the lengths of the triangle
        _3d_values v1 = get_v1();
        _3d_values v2 = get_v2();
        _3d_values v3 = get_v3();

        _3d_values nv1 = get_nv1();
        _3d_values nv2 = get_nv2();
        _3d_values nv3 = get_nv3();
        _3d_values interpolated_n(0, 0, 0);
        Plane plane(v1, v2, v3);
        // given we intersected the plane, we check if the point lies in the polygon
        if(plane.intersect(ray, t)) {
            _3d_values point_intersection = ray.get_origin() + ray.get_direction() * t;
            // cout << point_intersection.X << " " << point_intersection.Y << " " << point_intersection.Z << endl;
            double L1 = (v2 - v1).magnitude();
            double L2 = (v3 - v2).magnitude();
            double L3 = (v1 - v3).magnitude();
            // area of triangle
            double Area = sqrt((L1 + L2 + L3) * (-L1 + L2 + L3) * (L1 - L2 + L3) * (L1 + L2 - L3)) / 4;
            double a = ((point_intersection-v2).cross(point_intersection-v3)).magnitude()/2;
            double b = ((point_intersection-v3).cross(v1-v3)).magnitude()/2;
            double c = ((v2-v1).cross(point_intersection-v2)).magnitude()/2;
            double alpha = a/Area;
            double beta = b/Area;
            double gamma = c/Area;
            // cout << "alpha: "<<alpha << "beta: "<< beta << "gamma: "<< gamma << endl;
            return (((alpha >= 0 && alpha <= 1) && (beta >= 0 && beta <= 1) && (gamma >= 0 && gamma <= 1)) &&
                    (alpha + beta + gamma - 1 < .0000001));
        }
        return false;
    }
};

/*Object element: ellipsoid *//*
class Ellipsoid : public Objects {
// Encapsulate variables
private:
    _3d_values Center, Color;
    float R_x, R_y, R_z;
public:
    // set constructor of eillipsoid
    Ellipsoid(_3d_values center, float radius_x, float radius_y, float radius_z, _3d_values color) : Center(center), R_x(radius_x),
                                                                                                     R_y(radius_y), R_z(radius_z), Color(color) {}
    // return color
    _3d_values get_color2() {
        return Color;
    }
    // check for ray and ellipsoid intersection
    bool intersect(Ray_Vector& ray, float &t){
        _3d_values vect_dist(0, 0, 0);
    }
};*/
#endif
