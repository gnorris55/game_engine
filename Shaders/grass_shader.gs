#version 460 core
layout (points) in;
layout (triangle_strip, max_vertices = 15) out;
// wind stuff
uniform vec4 sin_values;
uniform vec4 sin_values2;
uniform vec3 wind_normal;
uniform int poly_mode = 0;

uniform mat4 model;
uniform mat4 projView;
uniform float tile_height;

in VS_OUT {
    float height;
    vec3 grass_params;
} gs_in[];  

out vec3 Normal;
out vec3 FragPos;

mat3 create_rot_matrix(float theta) {
    
    float angle = radians(theta*360);
    float sinAngle = sin(angle);
    float cosAngle = cos(angle);

    mat3 rotationMatrix = mat3(
        cosAngle, 0, -sinAngle,
        0, 1, 0,
        sinAngle, 0, cosAngle
    );

    return rotationMatrix;
}
vec3 get_point_in_curve(float t, vec3 control_points[4]) {
    int n = 3;
    vec3 result;
    
    result = pow((1-t), 3)*control_points[0] + 3*pow((1-t), 2)*t*control_points[1] 
           + 3*(1-t)*pow(t, 2)*control_points[2] + pow(t, 3)*control_points[3];

    return result;
}

vec3 get_normal_in_curve(float t, vec3 control_points[4], mat3 rotation_matrix) {

   //dP(t) / dt =  -3(1-t)^2 * P0 + 3(1-t)^2 * P1 - 6t(1-t) * P1 - 3t^2 * P2 + 6t(1-t) * P2 + 3t^2 * P3 
   vec3 tangent = -3*pow((1-t), 2)*control_points[0] + 3 *pow(1-t, 2)*control_points[1] - 6*t*(1-t)*control_points[1] - 3*pow(t, 2)*control_points[2] + 6*t*(1-t)*control_points[2] + 3*pow(t, 2)*control_points[3];

   vec3 facing = vec3(0.0, 0.0, 1.0);
   facing = facing*rotation_matrix;
   vec3 orthogonal = vec3(facing.z, facing.y, -facing.x);
   return normalize(cross(tangent, orthogonal));   
}

vec4 to_clipping_space(vec4 position) {
    return projView*position*model;
}

void build_blade_segment(vec3 render_point, vec3 normal, vec3 offset, vec3 anim_displacement) {


     mat3 rot_matrix = create_rot_matrix(10.0f);

     vec3 first_position = render_point + offset + anim_displacement; 
     gl_Position = to_clipping_space(vec4(first_position, 1.0));    // 1:bottom-left
     Normal = normal*rot_matrix;
     FragPos = first_position;
     EmitVertex();

     rot_matrix = create_rot_matrix(-10.0f);
     vec3 second_position = render_point + offset*-1 + + anim_displacement;
     gl_Position = to_clipping_space(vec4(second_position, 1.0));    // 1:bottom-right
     FragPos = second_position;
     Normal = normal*rot_matrix;
     EmitVertex();   
}

void build_grass_blade(vec4 position, vec3 render_points[8], vec3 normals[8], mat3 rotation_matrix) {    
    
    float animation_offset;
    float wind_strength = wind_normal.y*0.1;
    float stiffness = gs_in[0].grass_params.z;
    vec3 wind_c = wind_normal*wind_strength*stiffness; 

    vec3 offset = vec3(0.02, 0.0, 0.0)*rotation_matrix;

    if (poly_mode == 1) {
        build_blade_segment(render_points[0], normals[0], vec3(0.015, 0.0, 0.0)*rotation_matrix, vec3(0,0,0));
        build_blade_segment(render_points[3], normals[3], vec3(0.014, 0.0, 0.0)*rotation_matrix, wind_c*sin_values.z);
        build_blade_segment(render_points[6], normals[6], vec3(0.011, 0.0, 0.0)*rotation_matrix, wind_c*sin_values2.y);
        // top part

        vec3 temp_position = render_points[7] + wind_c*sin_values2.z;
        Normal = normals[7];
        FragPos = temp_position;
        gl_Position = to_clipping_space(vec4(temp_position, 1.0));   // 1:top
        EmitVertex();   
    } else if (poly_mode == 0){
    
        build_blade_segment(render_points[0], normals[0], vec3(0.015, 0.0, 0.0)*rotation_matrix, vec3(0,0,0));
        build_blade_segment(render_points[1], normals[1], vec3(0.015, 0.0, 0.0)*rotation_matrix, wind_c*sin_values.x );
        build_blade_segment(render_points[2], normals[2], vec3(0.014, 0.0, 0.0)*rotation_matrix, wind_c*sin_values.y);
        build_blade_segment(render_points[3], normals[3], vec3(0.014, 0.0, 0.0)*rotation_matrix, wind_c*sin_values.z);
        build_blade_segment(render_points[4], normals[4], vec3(0.013, 0.0, 0.0)*rotation_matrix, wind_c*sin_values.w);
        build_blade_segment(render_points[5], normals[5], vec3(0.015, 0.0, 0.0)*rotation_matrix, wind_c*sin_values2.x);
        build_blade_segment(render_points[6], normals[6], vec3(0.011, 0.0, 0.0)*rotation_matrix, wind_c*sin_values2.y);
        // top part

        vec3 temp_position = render_points[7] + wind_c*sin_values2.z;
        Normal = normals[7];
        FragPos = temp_position;
        gl_Position = to_clipping_space(vec4(temp_position, 1.0));   // 1:top
        EmitVertex();   
    } else  {
         build_blade_segment(render_points[0], normals[0], vec3(0.015, 0.0, 0.0)*rotation_matrix, vec3(0,0,0));
         vec3 temp_position = render_points[7] + wind_c*sin_values2.z;
            Normal = normals[7];
            FragPos = temp_position;
            gl_Position = to_clipping_space(vec4(temp_position, 1.0));   // 1:top
            EmitVertex();   
    }
    

    EndPrimitive();

}


void main() {    

    vec3 render_points[8];
    vec3 normals[8];
    float num_points = 8;
    vec4 position = gl_in[0].gl_Position;

    float grass_offset = gs_in[0].height*0.5;
    float grass_curve = gs_in[0].grass_params.x;
    mat3 rotation_matrix = create_rot_matrix(gs_in[0].grass_params.y);
    

    vec3 controlA = position.xyz;
    vec3 controlB = position.xyz + vec3(0.0, tile_height*(0.25 + grass_offset), -0.05 + -0.05*tile_height*grass_curve)*rotation_matrix;
    vec3 controlC = position.xyz + vec3(0.0, tile_height*(0.5 + grass_offset), 0.0)*rotation_matrix;
    vec3 controlD = position.xyz + vec3(0.0, tile_height*(0.75 + grass_offset), 0.05 + 0.4*tile_height*grass_curve)*rotation_matrix;

    vec3 control_points[4] = {controlA, controlB, controlC, controlD};

    for(int i = 0; i < num_points; i++) {
        float t = i / num_points;
        vec4 point = vec4(get_point_in_curve(t, control_points), 1);
        vec3 normal = get_normal_in_curve(t, control_points, rotation_matrix);

        render_points[i] =  point.xyz;
        normals[i] = normal;
    }

    build_grass_blade(position, render_points, normals, rotation_matrix);
}  