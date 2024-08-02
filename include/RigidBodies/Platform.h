#ifndef PLATFORM_H
#define PLATFORM_H

#include <RigidBodies/RigidMain.h>
#include <vector>

class Platform {

public:
	Shader* program;

	glm::vec4 position;
	glm::vec4 normal;
	std::vector<glm::vec4> platform_mesh = std::vector<glm::vec4>(8);
	std::vector<RawModel> platform_meshes;
	std::vector<FacePlane> faces;
	glm::vec3 scale;

	Platform(glm::vec4 position, glm::vec4 normal, Shader* program, glm::vec3 scale) {
		this->position = position;
		this->normal = normal;
		this->program = program;
		this->scale = scale;
		load_model();
	}

	void draw_box() {
		RawModel platform_mesh = platform_meshes[0];
		glm::mat4 model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(0, 0, -15));

		//model = model * current_state.local_coords_matrix;
		glUniformMatrix4fv(glGetUniformLocation(program->ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINES);
		renderer.render(platform_mesh, GL_TRIANGLES);
	}
	
private:
	Loader loader;
	Renderer renderer;

	void form_triangle(float arr[], float normals[], int triangle_index, glm::vec4 p1, glm::vec4 p2, glm::vec4 p3, glm::vec4 normal) {
		int arr_index = triangle_index * 9;
		normals[arr_index] = normal.x;
		normals[++arr_index] = normal.y;
		normals[++arr_index] = normal.z;
		normals[++arr_index] = normal.x;
		normals[++arr_index] = normal.y;
		normals[++arr_index] = normal.z;
		normals[++arr_index] = normal.x;
		normals[++arr_index] = normal.y;
		normals[++arr_index] = normal.z;


		arr_index = triangle_index * 9;
		arr[arr_index] = p1.x;
		arr[++arr_index] = p1.y;
		arr[++arr_index] = p1.z;
		arr[++arr_index] = p2.x;
		arr[++arr_index] = p2.y;
		arr[++arr_index] = p2.z;
		arr[++arr_index] = p3.x;
		arr[++arr_index] = p3.y;
		arr[++arr_index] = p3.z;
	}


	void add_face_plane(glm::vec4 n, int p1, int p2, int p3, int p4) {
		FacePlane curr_plane;
		curr_plane.normal = n;
		curr_plane.points[0] = p1;
		curr_plane.points[1] = p2;
		curr_plane.points[2] = p3;
		curr_plane.points[3] = p4;
		faces.push_back(curr_plane);
	}

	void load_model() {

		const int num_vertices = 12 * 3 * 3;
		float mesh_vertices[num_vertices];
		float normals[num_vertices];

		glm::mat4 scale_matrix = glm::mat4(1.0f);
		scale_matrix = glm::scale(scale_matrix, this->scale.xyz());

		glm::vec4 p1 = glm::vec4(-0.5, -0.5, 0.5, 1) * scale_matrix;
		glm::vec4 p2 = glm::vec4(-0.5, 0.5, 0.5, 1) * scale_matrix;
		glm::vec4 p3 = glm::vec4(0.5, 0.5, 0.5, 1) * scale_matrix;
		glm::vec4 p4 = glm::vec4(0.5, -0.5, 0.5, 1) * scale_matrix;

		glm::vec4 p5 = glm::vec4(-0.5, -0.5, -0.5, 1) * scale_matrix;
		glm::vec4 p6 = glm::vec4(-0.5, 0.5, -0.5, 1) * scale_matrix;
		glm::vec4 p7 = glm::vec4(0.5, 0.5, -0.5, 1) * scale_matrix;
		glm::vec4 p8 = glm::vec4(0.5, -0.5, -0.5, 1) * scale_matrix;

		form_triangle(mesh_vertices, normals, 0, p1, p2, p3, normal);
		form_triangle(mesh_vertices, normals, 1, p1, p3, p4, normal);

		platform_mesh[0] = p1;
		platform_mesh[1] = p2;
		platform_mesh[2] = p3;
		platform_mesh[3] = p4;
		platform_mesh[4] = p5;
		platform_mesh[5] = p6;
		platform_mesh[6] = p7;
		platform_mesh[7] = p8;

		//front side
		form_triangle(mesh_vertices, normals, 0, p1, p2, p3, glm::vec4(0, 0, 1, 0));
		form_triangle(mesh_vertices, normals, 1, p1, p3, p4, glm::vec4(0, 0, 1, 0));
		add_face_plane(glm::vec4(0, 0, 2, 0), 0, 1, 2, 3);


		//left side  
		form_triangle(mesh_vertices, normals, 2, p1, p2, p6, glm::vec4(-1, 0, 0, 0));
		form_triangle(mesh_vertices, normals, 3, p1, p6, p5, glm::vec4(-1, 0, 0, 0));
		add_face_plane(glm::vec4(-1, 0, 0, 0), 0, 1, 5, 4);

		//right side
		form_triangle(mesh_vertices, normals, 4, p4, p3, p7, glm::vec4(1, 0, 0, 0));
		form_triangle(mesh_vertices, normals, 5, p4, p7, p8, glm::vec4(1, 0, 0, 0));
		add_face_plane(glm::vec4(1, 0, 0, 0), 3, 2, 6, 7);

		//back side
		form_triangle(mesh_vertices, normals, 6, p5, p6, p7, glm::vec4(0, 0, -1, 0));
		form_triangle(mesh_vertices, normals, 7, p5, p7, p8, glm::vec4(0, 0, -1, 0));
		add_face_plane(glm::vec4(0, 0, -1, 0), 4, 5, 6, 7);

		//top
		form_triangle(mesh_vertices, normals, 8, p2, p6, p7, glm::vec4(0, 1, 0, 0));
		form_triangle(mesh_vertices, normals, 9, p2, p7, p3, glm::vec4(0, 1, 0, 0));
		add_face_plane(glm::vec4(0, 1, 0, 0), 1, 5, 6, 2);

		//bottom
		form_triangle(mesh_vertices, normals, 10, p1, p5, p8, glm::vec4(0, -1, 0, 0));
		form_triangle(mesh_vertices, normals, 11, p1, p8, p4, glm::vec4(0, -1, 0, 0));
		add_face_plane(glm::vec4(0, -1, 0, 0), 0, 4, 7, 3);

		RawModel new_rawModel = loader.loadToVAO(mesh_vertices, normals, num_vertices * sizeof(float));
		platform_meshes.push_back(new_rawModel);
	}

};



#endif
