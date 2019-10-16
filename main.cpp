#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include <map>
#include <queue>
#include <stack>
#include <iterator>
#include <algorithm>
#include "ray_trace.h"

using namespace std;
float const PI = 3.14159; // good enough

// for any 3d coordinate
struct _3d_ {
    double x, y, z;
};

struct _2d_ {
    int x, y;
};

// read these values
_3d_ eye, viewdir, updir, bkgcolor, mtlcolor, sptcolor, vertex, v_norm, face;
_2d_ v_t;
float ka, ks, kd, n;
// sphere
struct SphereType {
    double x, y, z, r;
};

struct Light_Struct {
    float x, y, z, w, r, g, b;
};

struct Triangle_Elements {
    queue<_3d_values> vertices;
    queue<_3d_values> normal_vertices;
    queue<_3d_values> faces;
    queue<_2d_values> texture_coords;
};

struct Light_Att_Struct {
    float x, y, z, w, r, g, b, c1, c2, c3;
};
// Global variables
vector<_3d_values> triangle_vectors;
SphereType sphere;
Triangle_Elements t;
double vfov;
float width, height;
Light_Struct light1;
Light_Att_Struct light_att1;
queue<_3d_values> sphere_colors;
queue<_3d_values> spt_colors;
queue<Light> many_lights;
queue<AttenuationLight> many_a_lights;
_3d_values** a;
queue<_3d_values> texture_picker;
queue<_3d_values> normal_picker;
_3d_values vector_map[1000];
_3d_values normal_map[1000];
_3d_values face_map[1000];
_2d_values texture_map[1000];
_3d_values su_normal_pick[1000];
_3d_values st_normal_pick[1000];
_3d_values ut_normal_pick[1000];
queue<string> textures;

bool read_f = false;
int GLOBAL_ST = 0;
int GLOBAL_SU = 0;
int GLOBAL_UT = 0;
vector<Objects*> objects;
enum State{SMOOTH_UNTEXTURED, SMOOTH_TEXTURED, UNSMOOTH_TEXTURED};
State shade_state;

int ppm_width;
int ppm_height;
int v_n_count =0, v_count =0, f_count = 0, vt_count = 0, su_count = 0, st_count = 0, ut_count =0;
int bkg_check = 0, eye_check = 0, up_check =0, view_check =0, mtl_check =0, old_mtl_check = 0, vf_check = 0, w_h = 0;

/*This method sets all of the values read from the input file and checks for error correction */
void set_coords(string coords) {
    // Split the values that we are reading to assign them accordingly
    vector<string> elements;
    istringstream is(coords);
    for(string eye_coords; is >> eye_coords;)
        elements.push_back(eye_coords);
    // input checking
    if(elements.size() != 4) {
        if((coords.find("mtlcolor") != string::npos)) {
            if(elements.size() != 11) {
                cout << "The mtlcolor element should have 11 elements";
                exit(1);
            }
        }else {
            cout << "The eye element should have 4 elements";
            exit(1);
        }
    }
    // input checking
    for(int i = 1; i < 4; i++) {
        if (elements.at(i).find_first_not_of("-1-2-3-4-5-6-7-8-90123456789.") != string::npos) {
            cout << elements.at(i).c_str() << endl;
            cout << "Please have only digits in value" << endl;
            exit(1);
        }
    }
    //assigning accordingly
    if((coords.find("eye") != string::npos)) {
        eye_check++;
        eye.x = atof(elements.at(1).c_str());eye.y = atof(elements.at(2).c_str());eye.z = atof(elements.at(3).c_str());
    }else if((coords.find("viewdir") != string::npos)) {
        view_check++;
        viewdir.x = atof(elements.at(1).c_str());viewdir.y = atof(elements.at(2).c_str());viewdir.z = atof(elements.at(3).c_str());
    }else if((coords.find("updir") != string::npos)) {
        up_check++;
        updir.x = atof(elements.at(1).c_str());updir.y = atof(elements.at(2).c_str());updir.z = atof(elements.at(3).c_str());
    }else if((coords.find("bkgcolor") != string::npos)) {
        bkg_check++;
        bkgcolor.x = atof(elements.at(1).c_str());bkgcolor.y = atof(elements.at(2).c_str());bkgcolor.z = atof(elements.at(3).c_str());
        if(bkgcolor.x < 0 || bkgcolor.x > 1 ||  bkgcolor.y < 0 || bkgcolor.y > 1 ||  bkgcolor.z < 0 || bkgcolor.z > 1 ) {
            cout << "color value may not be less than 0 or greater than 1 for 0-1 scale" << endl;
            exit(1);
        }
    }else if((coords.find("mtlcolor") != string::npos)) {
        mtl_check++;
        mtlcolor.x = atof(elements.at(1).c_str());
        mtlcolor.y = atof(elements.at(2).c_str());
        mtlcolor.z = atof(elements.at(3).c_str());
        sptcolor.x = atof(elements.at(4).c_str());
        sptcolor.y = atof(elements.at(5).c_str());
        sptcolor.z = atof(elements.at(6).c_str());
        ka = (float) atof(elements.at(7).c_str());
        kd = (float) atof(elements.at(8).c_str());
        ks = (float) atof(elements.at(9).c_str());
        n = (float) atof(elements.at(10).c_str());
        if (mtlcolor.x < 0 || mtlcolor.x > 1 || mtlcolor.y < 0 || mtlcolor.y > 1 || mtlcolor.z < 0 || mtlcolor.z > 1 ||
            sptcolor.x < 0 || sptcolor.x > 1 || sptcolor.y < 0 || sptcolor.y > 1 || sptcolor.z < 0 || sptcolor.z > 1) {
            cout << "color value may not be less than 0 or greater than 1 for 0-1 scale" << endl;
            exit(1);
        }
        _3d_values s_colors(int(mtlcolor.x * 255), int(mtlcolor.y * 255), int(mtlcolor.z * 255));
        _3d_values sp_color(int(sptcolor.x * 255), int(sptcolor.y * 255), int(sptcolor.z * 255));
        // cout << s_colors.X << " " << s_colors.Y << " " << s_colors.Z << " " << endl;
        // pop old colors out push new ones in
        if (sphere_colors.size() != 0 && spt_colors.size() != 0) {
            sphere_colors.pop();
            spt_colors.pop();
        }
        sphere_colors.push(s_colors);
        spt_colors.push(sp_color);
    }else {
        cout << "Not a valid input for 3 elements" << endl;
        exit(1);
    }
}

/*setting for vfov*/
void set_vfov(string eye_coords) {
    vector<string> elements;
    istringstream is(eye_coords);
    for(string eye_coords; is >> eye_coords;)
        elements.push_back(eye_coords);
    vf_check++;
    // input checkings
    if(elements.size() != 2) {
        cout << "The eye element should have 2 elements";
        exit(1);
    }
    if (elements.at(1).find_first_not_of("0123456789.") != string::npos) {
        cout << "Please have only digits in value" << endl;
        exit(1);
    }
    vfov = (double) atoi(elements.at(1).c_str());
    if(vfov <=  0 || vfov > 180) {
        cout << "not in bounds for vfov (degrees)" << endl;
        exit(1);
    }
}
int k = 0;
void build_triangle(int state_chooser) {
    if(state_chooser == 0)
        shade_state = SMOOTH_TEXTURED;
    if(state_chooser == 1)
        shade_state = UNSMOOTH_TEXTURED;
    if(state_chooser == 2)
        shade_state = SMOOTH_UNTEXTURED;
    switch(shade_state) {
        case SMOOTH_TEXTURED: {
            face_map[st_count-1] = t.faces.front();
            t.faces.pop();

            for (int i = 0; i < vt_count; i++) {
                texture_map[k++] = t.texture_coords.front();
                t.texture_coords.pop();
            }
            st_normal_pick[GLOBAL_ST] = normal_picker.front();
            cout << st_count << endl;
                _3d_values val1 = vector_map[int(face_map[GLOBAL_ST].X) - 1];
                _3d_values val2 = vector_map[int(face_map[GLOBAL_ST].Y) - 1];
                _3d_values val3 = vector_map[int(face_map[GLOBAL_ST].Z) - 1];
                _3d_values nval1 = normal_map[int(st_normal_pick[GLOBAL_ST].X) - 1];
                _3d_values nval2 = normal_map[int(st_normal_pick[GLOBAL_ST].Y) - 1];
                _3d_values nval3 = normal_map[int(st_normal_pick[GLOBAL_ST].Z) - 1];
                cout << "Normal: " << normal_picker.front().X << " " << normal_picker.front().Y << " " << normal_picker.front().Z << " " << endl;
                _2d_values vt1 = texture_map[(int)texture_picker.front().X - 1];
                _2d_values vt2 = texture_map[(int)texture_picker.front().Y - 1];
                _2d_values vt3 = texture_map[(int)texture_picker.front().Z - 1];
                texture_picker.pop();
                normal_picker.pop();
                // number of triangles to make
                Triangle *triangle = new Triangle(val1, val2, val3, // regular vectors
                                                  nval1, nval2, nval3, // normal vectors
                                                  _3d_values(face_map[GLOBAL_ST].X, face_map[GLOBAL_ST].Y, face_map[GLOBAL_ST].Z), // face
                                                  _3d_values(255, 50, 20),
                                                  _3d_values(204, 204, 204),  // color, spec light
                                                  ka, kd, ks,
                                                  n,                                      // phong model elements
                                                  vt1, vt2, vt3); // add a check to ensure these are not negative
                objects.push_back(triangle);
                if(v_count == 3) {
                    v_count = 0; // RESET FOR THE NEXT TRIANGLE :)
                }
                else
                {}

            GLOBAL_ST++;
        }
            break;
        case SMOOTH_UNTEXTURED:{
            face_map[GLOBAL_SU] = t.faces.front();
            t.faces.pop();
            // normal vectors
            cout << v_n_count << endl;
            su_normal_pick[su_count-1] = t.normal_vertices.front();
            t.normal_vertices.pop();
            cout << "v_count: " <<  v_count << endl;
            for(int k = 0; k < v_count; k++) {
                cout << vector_map[k].X << " " << vector_map[k].Y << " " << vector_map[k].Z << " " << endl;
            }

            cout << face_map[GLOBAL_SU].X << " " <<  face_map[GLOBAL_SU].Y << " " << face_map[GLOBAL_SU].Z << " " << endl;

            _3d_values val1 = vector_map[int(face_map[GLOBAL_SU].X) - 1];
            _3d_values val2 = vector_map[int(face_map[GLOBAL_SU].Y) - 1];
            _3d_values val3 = vector_map[int(face_map[GLOBAL_SU].Z) - 1];
            _3d_values nval1 = normal_map[int(su_normal_pick[GLOBAL_SU].X) - 1];
            _3d_values nval2 = normal_map[int(su_normal_pick[GLOBAL_SU].Y) - 1];
            _3d_values nval3 = normal_map[int(su_normal_pick[GLOBAL_SU].Z) - 1];
            _2d_values vt1(0,0);
            _2d_values vt2(0,0);
            _2d_values vt3(0,0);
            // number of triangles to make
            Triangle *triangle = new Triangle(val1, val2, val3, // regular vectors
                                              nval1, nval2, nval3, // normal vectors
                                              _3d_values(face_map[GLOBAL_SU].X, face_map[GLOBAL_SU].Y, face_map[GLOBAL_SU].Z), // face
                                              _3d_values(255, 50, 20),
                                              _3d_values(204, 204, 204),  // color, spec light
                                              ka, kd, ks,n,                                      // phong model elements
                                              vt1, vt2, vt3); // add a check to ensure these are not negative
            objects.push_back(triangle);
            GLOBAL_SU++;
        }
            break;
        case UNSMOOTH_TEXTURED: {
            cout << "UNSMOOTH_TEXTURED" << endl;
            // faces
            face_map[GLOBAL_UT] = t.faces.front();
            t.faces.pop();

            texture_map[GLOBAL_UT] = t.texture_coords.front();
            t.texture_coords.pop();

            _3d_values val1 = vector_map[int(face_map[GLOBAL_UT].X) - 1];
            _3d_values val2 = vector_map[int(face_map[GLOBAL_UT].Y) - 1];
            _3d_values val3 = vector_map[int(face_map[GLOBAL_UT].Z) - 1];
            _3d_values nval1(0,0,0);
            _3d_values nval2(0,0,0);
            _3d_values nval3(0,0,0);
            _2d_values vt1 = texture_map[(int)texture_picker.front().X - 1];
            _2d_values vt2 = texture_map[(int)texture_picker.front().Y - 1];
            _2d_values vt3 = texture_map[(int)texture_picker.front().Z - 1];

            // number of triangles to make
            Triangle *triangle = new Triangle(val1, val2, val3, // regular vectors
                                              nval1, nval2, nval3, // normal vectors
                                              _3d_values(face_map[GLOBAL_UT].X, face_map[GLOBAL_UT].Y, face_map[GLOBAL_UT].Z), // face
                                              _3d_values(255, 50, 20),
                                              _3d_values(204, 204, 204),  // color, spec light
                                              ka, kd, ks,
                                              n,                                      // phong model elements
                                              vt1, vt2, vt3); // add a check to ensure these are not negative
            objects.push_back(triangle);

        }
            break;
        default: {
            cout << "IN DEFAULT" << endl;
            int k = 0, l = 0, m = 0, n = 0;
            for (int i = 0; i < v_count; i++) {
                vector_map[k++] = t.vertices.front();;
                t.vertices.pop();
            }
            // faces
            for (int i = 0; i < f_count; i++) {
                face_map[l++] = t.faces.front();
                t.faces.pop();
            }
            // normal vectors
            for (int i = 0; i < v_n_count; i++) {
                normal_map[m++] = t.normal_vertices.front();
                t.normal_vertices.pop();
            }

            for (int i = 0; i < vt_count; i++) {
                texture_map[n++] = t.texture_coords.front();
                t.texture_coords.pop();
            }

            for (int i = 0; i < f_count; i++) {
                cout << "Faces: " << face_map[i].X << " " << face_map[i].Y << " " << face_map[i].Z << endl;
            }
            cout << f_count << endl;
            for (int i = 0; i < f_count; i++) {
                _3d_values val1 = vector_map[int(face_map[i].X) - 1];
                _3d_values val2 = vector_map[int(face_map[i].Y) - 1];
                _3d_values val3 = vector_map[int(face_map[i].Z) - 1];
                _3d_values nval1 = normal_map[int(face_map[i].X) - 1];
                _3d_values nval2 = normal_map[int(face_map[i].Y) - 1];
                _3d_values nval3 = normal_map[int(face_map[i].Z) - 1];
                _2d_values vt1 = texture_map[int(face_map[i].X) - 1];
                _2d_values vt2 = texture_map[int(face_map[i].Y) - 1];
                _2d_values vt3 = texture_map[int(face_map[i].Z) - 1];

                // number of triangles to make
                Triangle *triangle = new Triangle(val1, val2, val3, // regular vectors
                                                  nval1, nval2, nval3, // normal vectors
                                                  _3d_values(face_map[i].X, face_map[i].Y, face_map[i].Z), // face
                                                  _3d_values(255, 50, 20),
                                                  _3d_values(204, 204, 204),  // color, spec light
                                                  ka, kd, ks,
                                                  n,                                      // phong model elements
                                                  vt1, vt2, vt3); // add a check to ensure these are not negative
                objects.push_back(triangle);
            }
        }
    }
}

void set_triangle(string coords) {
    // Split the values that we are reading to assign them accordingly
    vector<string> elements;
    istringstream is(coords);
    for(string eye_coords; is >> eye_coords;)
        elements.push_back(eye_coords);

    if(elements.size() != 4) {
        if(!strcmp(elements.at(0).c_str(), "vt")) { // textured coordinates
            if(elements.size() != 3) {
                cout << "vt should have 3 elements" << endl;
                exit(1);
            }
        }else {
            cout << "The element should have 4 elements" << endl;
            exit(1);
        }
    }
    // input checking
    if(!strcmp(elements.at(0).c_str(), "vt")) {
        for (int i = 1; i < 3; i++) {
            if (elements.at(i).find_first_not_of("-1-2-3-4-5-6-7-8-90123456789./") != string::npos) {
                cout << elements.at(i).c_str() << endl;
                cout << "Please have only digits in value" << endl;
                exit(1);
            }
        }
    }else {
        for (int i = 1; i < 4; i++) {
            if (elements.at(i).find_first_not_of("-1-2-3-4-5-6-7-8-90123456789./") != string::npos) {
                cout << elements.at(i).c_str() << endl;
                cout << "Please have only digits in value" << endl;
                exit(1);
            }
        }
    }
    if(!strcmp(elements.at(0).c_str(), "v")) {
        if(read_f) {
            v_count = 0;
            for(int i = 0; i < t.vertices.size(); i++)
                t.vertices.pop();
            for(int i = 0; i < t.faces.size(); i++)
                t.faces.pop();
            read_f = false;
        }
        cout << t.vertices.size() << endl;
        vertex.x = atof(elements.at(1).c_str());
        vertex.y = atof(elements.at(2).c_str());
        vertex.z = atof(elements.at(3).c_str());
        t.vertices.push(_3d_values(vertex.x, vertex.y, vertex.z));

        vector_map[v_count++] = t.vertices.front();
        t.vertices.pop();
    }else if(!strcmp(elements.at(0).c_str(), "f")) { // the number of faces is the number of triangles
        read_f = true; // we have read a face
        if((elements.at(1).length() == 5)) { // SMOOTH_TEXTURED
            int x = (elements.at(1).at(2) - 48);int y = (elements.at(2).at(2) - 48);int z = (elements.at(3).at(2) - 48);
            texture_picker.push(_3d_values(x, y, z));

            int a = (elements.at(1).at(4) - 48);int b = (elements.at(2).at(4) - 48);int c = (elements.at(3).at(4) - 48);
            normal_picker.push(_3d_values(a, b, c));

            int d = (elements.at(1).at(0) - 48);int e = (elements.at(2).at(0) - 48);int f = (elements.at(3).at(0) - 48);
            t.faces.push(_3d_values(d, e, f));
            // t.texture_coords.push(_2d_values(x, y));
            st_count++;
            build_triangle(0);
        }else if((elements.at(1).length() == 3)) { // UNSMOOTH_TEXTURED
            int x = (elements.at(1).at(2) - 48);
            int y = (elements.at(2).at(2) - 48);
            int z = (elements.at(3).at(2) - 48);
            texture_picker.push(_3d_values(x, y, z));
            int d = (elements.at(1).at(0) - 48);int e = (elements.at(2).at(0) - 48);
            int f = (elements.at(3).at(0) - 48);
            t.faces.push(_3d_values(d, e, f));
            ut_count++;
            build_triangle(1);
        }else if((elements.at(1)).find("//") != string::npos) { // SMOOTH_UNTEXTURED
            int a = (elements.at(1).at(0) - 48);
            int b = (elements.at(2).at(0) - 48);
            int c = (elements.at(3).at(0) - 48);
            t.normal_vertices.push(_3d_values(a, b, c));
            int d = (elements.at(1).at(0) - 48);int e = (elements.at(2).at(0) - 48);
            int f = (elements.at(3).at(0) - 48);
            t.faces.push(_3d_values(d, e, f));
            su_count++;
            build_triangle(2);
        }else {
            face.x = atof(elements.at(1).c_str());
            face.y = atof(elements.at(2).c_str());
            face.z = atof(elements.at(3).c_str());
            t.faces.push(_3d_values(face.x, face.y, face.z));// = _3d_values(face.x, face.y, face.z);
            f_count++;
        }
    }else if(!strcmp(elements.at(0).c_str(), "vn")) {
        v_norm.x = atof(elements.at(1).c_str());
        v_norm.y = atof(elements.at(2).c_str());
        v_norm.z = atof(elements.at(3).c_str());
        t.normal_vertices.push(_3d_values(v_norm.x, v_norm.y, v_norm.z));
        normal_map[v_n_count++] = t.normal_vertices.front();
        t.normal_vertices.pop();
    }else if(!strcmp(elements.at(0).c_str(), "vt")) { // check here for more modifications
        v_t.x = atoi(elements.at(1).c_str());
        v_t.y = atoi(elements.at(2).c_str());
        t.texture_coords.push(_2d_values(v_t.x, v_t.y));
        vt_count++;
    }else {}
}


/*Setting the values for the sphere*/
void set_sphere(string eye_coords) {
    vector<string> elements;
    istringstream is(eye_coords);
    for(string eye_coords; is >> eye_coords;)
        elements.push_back(eye_coords);
    // Input checking
    if(elements.size() != 5) {
        cout << "The eye element should have 4 elements";
        exit(1);
    }
    for(int i = 1; i < 5; i++) {
        if (elements.at(i).find_first_not_of("-1-2-3-4-5-6-7-8-90123456789.") != string::npos) {
            cout << "Please have only digits in value" << endl;
            exit(1);
        }
    }
    sphere.x = atof(elements.at(1).c_str());sphere.y = atof(elements.at(2).c_str());
    sphere.z = atof(elements.at(3).c_str());sphere.r = atof(elements.at(4).c_str());
    if(sphere.r <= 0) {
        cout << "sphere should have value > 0" << endl;
        exit(1);
    }
}

/*Setting for the image_width and image_height*/
void setWidth_Height(string img_desc) {
    string w, h;
    // split the image desc. string by spaces
    vector<string> elements;
    istringstream is(img_desc);
    for(string img_desc; is >> img_desc;)
        elements.push_back(img_desc);
    w_h++;
    // We only expect three elements in string otherwise exit
    if(elements.size() != 3) {
        cout << "Please try to only have three elements as: imsize width height";
        exit(1);
    }

    // make sure only numbers for width and height values
    w = elements.at(1).c_str(); h = elements.at(2).c_str();
    if(w.find_first_not_of("0123456789") != string::npos) {
        cout << "Please have only digits in width value" << endl;
        exit(1);
    }
    if(h.find_first_not_of("0123456789") != string::npos) {
        cout << "Please have only digits in height value" << endl;
        exit(1);
    }

    // get the width and height elements and convert the strings for width and height to ints
    width = atoi(elements.at(1).c_str());
    height = atoi(elements.at(2).c_str());

    // if any value is less than 0
    if(width < 0 || height < 0) {
        cout << "Please values only greater than 0" << endl;
        exit(1);
    }
}

void set_light(string str_input) {
    vector<string> elements;
    istringstream is(str_input);
    for(string str_in; is >> str_input;)
        elements.push_back(str_input);

    // We only expect three elements in string otherwise exit
    if(elements.size() != 8) {
        cout << "Please try to only have 8 elements for light element";
        exit(1);
    }
    for(int i = 1; i < 8; i++) {
        if (elements.at(i).find_first_not_of("-1-2-3-4-5-6-7-8-90123456789.") != string::npos) {
            cout << "Please have only digits in value" << endl;
            exit(1);
        }
    }
    light1.x = stof(elements.at(1));light1.y = stof(elements.at(2));
    light1.z = stof(elements.at(3));light1.w = stof(elements.at(4));
    light1.r = stof(elements.at(5));light1.g = stof(elements.at(6));
    light1.b = stof(elements.at(7));
    Light l(_3d_values(light1.x, light1.y, light1.z), light1.w, _3d_values(light1.r, light1.g, light1.b));
    many_lights.push(l);
}

void set_att_light(string str_input) {
    vector<string> elements;
    istringstream is(str_input);
    for(string str_in; is >> str_input;)
        elements.push_back(str_input);

    // We only expect three elements in string otherwise exit
    if(elements.size() != 11) {
        cout << "Please try to only have 11 elements for light element";
        exit(1);
    }
    for(int i = 1; i < 11; i++) {
        if (elements.at(i).find_first_not_of("-1-2-3-4-5-6-7-8-90123456789.") != string::npos) {
            cout << "Please have only digits in value" << endl;
            exit(1);
        }
    }
    light_att1.x = stof(elements.at(1));light_att1.y = stof(elements.at(2));
    light_att1.z = stof(elements.at(3));light_att1.w = stof(elements.at(4));
    light_att1.r = stof(elements.at(5));light_att1.g = stof(elements.at(6));
    light_att1.b = stof(elements.at(7));light_att1.c1 = stof(elements.at(8));
    light_att1.c2 = stof(elements.at(9));light_att1.c3 = stof(elements.at(10));
    AttenuationLight l(_3d_values(light_att1.x, light_att1.y, light_att1.z), light_att1.w,
                       _3d_values(light_att1.r, light_att1.g, light_att1.b), light_att1.c1, light_att1.c2, light_att1.c3);
    many_a_lights.push(l);
}

void check_ppm_format(string str_input) {
    vector<string> elements;
    istringstream is(str_input);
    for(string str_in; is >> str_input;)
        elements.push_back(str_input);

    if(elements.size() != 4) {
        cout << "Format required: P3 Width Height MaxColor";
        exit(1);
    }
}

void set_ppm(string str_input) {
    vector<string> elements;
    istringstream is(str_input);
    for(string str_in; is >> str_input;)
        elements.push_back(str_input);
    // We only expect three elements in string otherwise exit
    if(elements.size() != 2) {
        cout << "texture line should only have 4 elements";
        exit(1);
    }
    // remove current material
    if (sphere_colors.size() != 0 && spt_colors.size() != 0) {
        sphere_colors.pop();
        spt_colors.pop();
    }
    sphere_colors.push(_3d_values(0,0,0));
    spt_colors.push(_3d_values(0,0,0));
    textures.push(elements.at(1).c_str());
    ifstream ifstream1(elements.at(1).c_str());
    if(ifstream1.is_open()) {
        vector<string> ppm_values;
        string line_1;
        getline(ifstream1, line_1);
        check_ppm_format(line_1);
        cout << line_1 << endl;
        istringstream is2(line_1);
        for(string str_in; is2 >> line_1;)
            ppm_values.push_back(line_1);

        cout << ppm_values.size() << endl;
        ppm_width = stoi(ppm_values.at(1).c_str());
        ppm_height = stoi(ppm_values.at(2).c_str());

        int red, green, blue;
        a = new _3d_values*[ppm_width];
        // initialize pointer
        for(int i =0; i < ppm_width; i++)
            a[i] =new _3d_values[ppm_height];

        for(int i  =0 ; i < ppm_width; i++) {
            for (int j = 0; j < ppm_height; j++) {
                ifstream1 >> red >> green >> blue;
                a[i][j] = _3d_values(red, green, blue);
            }
        }
    }else {
        cout << "Invalid ppm file" << endl;
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    ifstream is(argv[1]);
    // check to see if its junk
    if(!is.is_open()) {
        cout << "Invalid file" << endl;
        exit(1);
    }
    string str_input;
    vector<Sphere*> spheres;
    vector<Triangle> triangles;
    vector<Light> lights;
    vector<AttenuationLight> att_lights;

    while(!is.eof()) {
        getline(is, str_input);
        if((str_input.find("#") != string::npos)) {} // IGNORE COMMENT
        if((str_input.find("eye") != string::npos)     ||
           (str_input.find("viewdir") != string::npos) ||
           (str_input.find("updir") != string::npos)   ||
           (str_input.find("bkgcolor") != string::npos)||
           (str_input.find("mtlcolor") != string::npos)) {
            set_coords(str_input);
        }
        if((str_input.find("texture") != string::npos)) {
            if(textures.size() == 0) {
                set_ppm(str_input);
            }
            else {
                textures.pop();
                delete[] a;
                set_ppm(str_input);
            }
        }
        if((str_input.find("v ") != string::npos)||
           (str_input.find("f ") != string::npos)||
           (str_input.find("vn ") != string::npos) ||
           (str_input.find("vt ") != string::npos)) {
            if((str_input.find("vfov") != string::npos))
                set_vfov(str_input);
            else
                set_triangle(str_input);
        }
        if((str_input.find("sphere") != string::npos)) {
            set_sphere(str_input);
            if(sphere_colors.empty()) {
                cout<< "Please insert mtlcolor before inserting sphere values" << endl;
                exit(1);
            }
            _3d_values sphere_color = sphere_colors.front();
            _3d_values spec_light = spt_colors.front();
            Sphere* sphere1 = new Sphere(_3d_values(sphere.x, sphere.y, sphere.z), sphere.r, sphere_color, spec_light, ka, kd, ks,n);
            cout << spec_light.X << " " << spec_light.Y << " " << spec_light.Z << endl;
            objects.push_back(sphere1);
        }
        if((str_input.find("imsize") != string::npos))
            setWidth_Height(str_input);
        if((str_input.find("light") != string::npos)) { // allow for multiple light
            if((str_input.find("att") != string::npos)) { // attenuation lights
                set_att_light(str_input);
                att_lights.push_back(many_a_lights.front());
                many_a_lights.pop();
            }else {
                set_light(str_input);
                lights.push_back(many_lights.front());
                many_lights.pop();
            }
        }
    } // once all of the values have been set

    cout << "Objects Size: " << objects.size() << endl;

    // check if critical values were provided
    if(objects.size() == 0) {
        cout << " Please input a value for any object" << endl;
        exit(1);
    }
    /*if(bkg_check == 0 || eye_check == 0 || up_check == 0 || view_check == 0 || mtl_check == 0 || vf_check == 0 || w_h == 0) {
        cout << "You are missing a critical value for the input" << endl;
        exit(1);
    }*/


    //triangles.push_back(triangle);
    // set the pixel color and the ray that will be shooting to detect the sphere
    _3d_values pixel_color(int(bkgcolor.x * 255), int(bkgcolor.x * 255), int(bkgcolor.x * 255));
    Ray_Tracer r_t = Ray_Tracer();

    // Define view_dir and up_dir
    _3d_values view_ray_dir = _3d_values(viewdir.x, viewdir.y, viewdir.z);
    _3d_values up_dir = _3d_values(updir.x, updir.y, updir.z);
    _3d_values eye_dir = _3d_values(eye.x, eye.y, eye.z);
    // Check if up_dir and view _dir are in parallel, and exit if so
    _3d_values view_dir_norm = view_ray_dir.normalize();
    _3d_values up_dir_norm = up_dir.normalize();
    if (!up_dir.magnitude()) {
        cout << up_dir.magnitude() << endl;
        cout << "Can't have a 0 vector for up" << endl;
        exit(1);
    }
    // check if up is in parallel with view_dir
    if (fabs((up_dir_norm.X == view_dir_norm.X)) < .00001 &&
        fabs((up_dir_norm.Y == view_dir_norm.Y)) < .00001 &&
        fabs((up_dir_norm.Z == view_dir_norm.Z)) < .00001) {
        cout << "up_dir is in parallel to view_dir" << endl;
        exit(1);
    }

    // set the u and v
    _3d_values u = (view_ray_dir.cross(up_dir)).normalize();
    _3d_values v = (u.cross(view_ray_dir)).normalize();

    // assuming vfov == vfoh
    float d = 5.0; // arbitrary
    float h = ceil((float(2 * d * tan(vfov * PI / 360.0))));
    // check viewieing window is ok using aspect ratio
    float asp = width/height;
    float w = h * asp;
    // calculate the upper left of the view window
    _3d_values ul = eye_dir + view_dir_norm * d - u * (w / 2) + v * (h / 2);
    _3d_values ur = eye_dir + view_dir_norm * d + u * (w / 2) + v * (h / 2);
    _3d_values ll = eye_dir + view_dir_norm * d - u * (w / 2) - v * (h / 2);
    _3d_values lr = eye_dir + view_dir_norm * d + u * (w / 2) - v * (h / 2);
    // ∆h= (ur–ul)/(image_width_in_pixels–1)
    // ∆v= (ll–ul)/(image_height_in_pixels–1)
    float view_asp = (ur-ul).magnitude()/(ll-ul).magnitude();
    _3d_values delta_h = (ur - ul) * (1 / width);
    _3d_values delta_v = (lr - ur) * (1 / height);
    cout << delta_h.X << " " << delta_h.Y << " " << delta_h.Z << endl;
    cout << ul.X << " " << ul.Y << " " << ul.Z << endl;
    cout << ur.X << " " << ur.Y << " " << ur.Z << endl;
    // Set the background color
    _3d_values background_color = _3d_values(int(bkgcolor.x * 255), int(bkgcolor.y * 255), int(bkgcolor.z * 255));

    // Output file to match the Input file name
    if (argc != 2) {
        cout << "Input should be as: ./a.out name_of_file < name_of_file.txt" << endl;
        exit(1);
    }
    // get input name
    string output_file = argv[1];
    string txt = ".txt";
    int size = output_file.find(txt);
    output_file.erase(size, txt.length());
    // read till the . and stop there
    output_file += ".ppm";
    ofstream output(output_file);

    // Error checking for opening
    if (!output.is_open()) {
        cout << "Problem opening file" << endl;
        exit(1);
    }
    cout << "objects: " << objects.size() << endl;
    // Start populating the .ppm file
    output << "P3 " << width << " " << height << " 255\n";
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            // Map point from view window to pixel
            // ul + (i)∆h+ (j)∆v
            _3d_values pixel = ul + (delta_h * j) + (delta_v * i) + delta_h * .5 + delta_v * .5;
            _3d_values ray_dir = (pixel - eye_dir).normalize();
            // return the pixel_color seen at the view window
            pixel_color = r_t.Trace_Ray(ppm_width, ppm_height, eye_dir, ray_dir, background_color, objects, a, view_ray_dir, lights, att_lights);
            output << int(pixel_color.X) << ' ' << int(pixel_color.Y) << ' ' << int(pixel_color.Z) << "\n";
        }
    }
    output.close();
    return 0;
}