#include <array>
#include <cmath>
#include <cstddef>
#include <vector>

//Used in the function below
std::array<float, 3> sphere_to_cartesian(float u, float v, float r)
{
    float x = r * std::sin(v) * std::cos(u);
    float y = r * std::sin(v) * std::sin(u);
    float z = r * std::cos(v);

    return {x, y, z};
}

//Get the vertices (To be drawn with GL_TRIANGLES) of a sphere centered at origin.
std::vector<float> sphere_vertex(float r, int num_sectors, int num_stacks) {
    float sector_step = (2 * M_PI) / num_sectors;
    float stack_step = M_PI / num_stacks;
    std::vector<float> vertex_list;

    for(int i = 0; i < num_sectors; i++) {
        for(int j =0; j < num_stacks; j++) {
            float u = i * sector_step;
            float v = j * stack_step;

            float un = 0.0;
            if(i+1 == num_sectors) un = 2 * M_PI;
            else un = (i+1) * sector_step;

            float vn = 0.0;
            if(j+1 == num_stacks) vn = M_PI;
            else vn = (j+1) * stack_step;

            std::array<float, 3> p0 = sphere_to_cartesian(u, v, r);
            std::array<float, 3> p1 = sphere_to_cartesian(u, vn, r);
            std::array<float, 3> p2 = sphere_to_cartesian(un, v, r);
            std::array<float, 3> p3 = sphere_to_cartesian(un, vn, r);
            
            vertex_list.push_back(p0[0]); vertex_list.push_back(p0[1]); vertex_list.push_back(p0[2]);
            vertex_list.push_back(p2[0]); vertex_list.push_back(p2[1]); vertex_list.push_back(p2[2]);
            vertex_list.push_back(p1[0]); vertex_list.push_back(p1[1]); vertex_list.push_back(p1[2]);

            vertex_list.push_back(p3[0]); vertex_list.push_back(p3[1]); vertex_list.push_back(p3[2]);
            vertex_list.push_back(p1[0]); vertex_list.push_back(p1[1]); vertex_list.push_back(p1[2]);
            vertex_list.push_back(p2[0]); vertex_list.push_back(p2[1]); vertex_list.push_back(p2[2]);
        }
    }

    return vertex_list;
}

//Concatenate 2 std::vectors at the one that is passed as the first argument
template <typename T>
void concat(std::vector<T>& a, const std::vector<T>& b)
{
    a.insert(a.end(), b.begin(), b.end());
}

//Given a global vector "vertex" and a object vector "object", adds object to vertex and returns the initial position of the object and the number of floats
template <typename T>
void add_object(const std::vector<T>& object, std::vector<T>& vertex, size_t& ini_pos, size_t& num_added) {
    num_added =object.size();
    ini_pos = vertex.size();
    concat(vertex, object);
}