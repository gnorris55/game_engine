#ifndef SPHERE_H
#define SPHERE_H

#include <RigidBodies/RigidMain.h>


class Sphere : public MainRigidBody {


public:
	
	float radius;

	Sphere(Shader* shader, float radius, glm::vec4 starting_pos, glm::vec4 starting_velocity, glm::vec4 starting_angular, glm::vec3 color, float mass) : MainRigidBody(shader, starting_pos, starting_velocity, starting_angular, glm::vec3(1.0, 1.0, 1.0)*radius, color, mass) {
		this->radius = radius;
		load_model();
		create_properties();
	}
	


private:
	
	void create_properties() {
		create_local_coords();
		create_inertia_matrix();
	}

	void create_inertia_matrix() {
	
		//std::cout << "inertia matrix for sphere\n";
		float inertia_scalar = (2.0f / 5.0f) * mass * pow(radius, 2);
		glm::vec3 row1 = glm::vec3(inertia_scalar, 0.0, 0.0);
		glm::vec3 row2 = glm::vec3(0.0, inertia_scalar, 0.0);
		glm::vec3 row3 = glm::vec3(0.0, 0.0, inertia_scalar);
		inertia_matrix = glm::mat3(row1, row2, row3);
	}

	void create_local_coords() {
		
		current_state.position = current_state.position;
		current_state.rotation_matrix = glm::mat3_cast(current_state.rotation_quat);

		current_state.local_coords_matrix = glm::transpose(glm::mat4(glm::vec4(current_state.rotation_matrix[0], 0.0),
			glm::vec4(current_state.rotation_matrix[1], 0.0),
			glm::vec4(current_state.rotation_matrix[2], 0.0),
			current_state.position));

	}

	void load_model() {
		//this is a test
		const int lat_segment = 60, lon_segment = 60;
		float vertices[lat_segment*lon_segment*3];
		float normals[lat_segment * lon_segment * 3];
		float tex[lat_segment * lon_segment * 2];
		int indices[lat_segment*lon_segment*6];
		create_mesh(vertices, normals, tex, indices, lon_segment, lat_segment);
		load_to_mesh(vertices, normals, tex, indices, lat_segment * lon_segment * sizeof(float), lat_segment * lon_segment * 6);
	}


	void create_mesh(float vertices[], float normals[], float tex[], int indices[], int lon, int lat) {
		for (int i = 0; i < lat; i++) {
			float theta = glm::radians(360.0f*2) * (float)i / lat;
			float sin_theta = sin(theta);
			float cos_theta = cos(theta);

			for (int j = 0; j < lon; j++) {
				float phi = glm::radians(360.0f) * (float)j / lon;
				float sin_phi = sin(phi);
				float cos_phi = cos(phi);

				float x = radius * sin_theta * cos_phi;
				float y = radius * sin_theta * sin_phi;
				float z = radius * cos_theta;
				
				float u = (float) j / lon;
				float v = (float) i/ lat;
				
				vertices[(lat * i + j)*3] = x;
				vertices[(lat * i + j)*3 + 1] = y;
				vertices[(lat * i + j)*3 + 2] = z;
				
				normals[(lat * i + j)*3] = x / radius;
				normals[(lat * i + j)*3 + 1] = y / radius;
				normals[(lat * i + j)*3 + 2] = z / radius;

				tex[(lat * i + j)*2] = u;
				tex[(lat * i + j)*2 + 1] = v;

			}
		}
		//std::cout << "loading indices\n";
		for (int i = 0; i < lat; i++) {
			for (int j = 0; j < lon; j++) {
				float first = i * lat + j;
				float second = (i+1)*lat + j;
				
				indices[(i * lat + j) * 6] = first;
				indices[(i * lat + j) * 6 + 1] = second;
				indices[(i * lat + j) * 6 + 2] = first + 1;

				indices[(i * lat + j) * 6 + 3] = second;
				indices[(i * lat + j) * 6 + 4] = first + 1;
				indices[(i * lat + j) * 6 + 5] = second + 1;
			}
		}

	}

};


#endif
