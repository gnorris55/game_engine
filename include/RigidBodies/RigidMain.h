#ifndef RIGID_MAIN_H
#define RIGID_MAIN_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <learnopengl/model.h>


class MainRigidBody : public Entity {

public:
	
	Shader* shader;
	unsigned int VAO;
	int num_indices;
	unsigned int texture;
	Loader loader;
		float mass;

	bool resting = false;
	glm::vec4 collision_force = glm::vec4(0, 0, 0, 0);
	glm::vec4 collision_torque = glm::vec4(0, 0, 0, 0);
	glm::vec4 force;
	glm::vec4 torque;

	glm::vec3 scale;

	State current_state;
	State prev_state;
	State backup_state;
	//glm::quat rotation_quat = glm::quat(1.0f, 0.7f, 0.5f, 0.0f);
	glm::mat3 inertia_matrix;
	glm::vec4 center_of_mass = glm::vec4(0.0, 0.0, 0.0, 1.0);
	glm::vec4 additional_acc = glm::vec4(0.0, 0.0, 0.0, 0.0);
	glm::vec4 input_acc = glm::vec4(0.0, 0.0, 0.0, 0.0);
	glm::vec4 input_angular = glm::vec4(0.0, 0.0, 0.0, 0.0);
	glm::vec3 color;

	//glm::vec4 friction_acc = glm::vec4(0.0, 0.0, 0.0, 0.0);

	MainRigidBody(Shader *shader, glm::vec4 starting_pos, glm::vec4 starting_velocity, glm::vec4 starting_angular, glm::vec3 scale, glm::vec3 color, float mass) : Entity(shader, starting_pos.xyz()) {
		this->shader = shader;
		this->scale = scale;
		this->mass = mass;
		this->color = color;
		this->current_state.position = starting_pos;
		this->current_state.linear_velocity = starting_velocity;
		this->current_state.angular_velocity = starting_angular;
		//create_properties();
	}

	glm::vec4 get_world_pos(bool prev = false) {
		if (prev)
			return center_of_mass * prev_state.local_coords_matrix;
		else
			return center_of_mass * current_state.local_coords_matrix;
	}

	void set_forces(/*CollisionHandler* collision_handler*/) {

		current_state.torque = glm::vec4(0, 0, 0, 0);
		current_state.force = glm::vec4(0, 0, 0, 0);

		glm::vec4 example_torque = glm::vec4(inertia_matrix * glm::vec3(0.0, 1.0, 0.0), 1.0);

		current_state.force = mass*(force + glm::vec4(0, -9.8f, 0.0, 0.0) + additional_acc + input_acc);
		current_state.torque = current_state.torque + input_angular;

	}

	void load_to_mesh(float vertices[], float normals[], float texture_coord[], int indices[], int num_vertices, int num_indices) {
		this->VAO = loader.createVAO();
		this->num_indices = num_indices;
		loader.bindIndicesBuffer(indices, num_indices);
		loader.storeDataInAttributeList(0, num_vertices*3, 3, vertices);
		loader.storeDataInAttributeList(1, num_vertices*3, 3, normals);
		loader.storeDataInTexCoordList(2, num_vertices*2, 2, texture_coord);
		glBindVertexArray(0);
	}

	void draw(glm::mat4 proj, glm::mat4 view, glm::vec3 camera_position) {

		shader->use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		
		shader->setMat4("model", current_state.local_coords_matrix);
		shader->setMat4("projView", proj*view);
		shader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);
		//shader->setVec3("lightPos", -30.0f, 50.0f, 20.0f);
		shader->setVec3("viewPos", camera_position);
		shader->setMat4("projView", proj * view);
		shader->setVec3("objectColor", color);
		shader->setBool("hasTexture", false);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, 0);
		//glDrawElements(GL_LINES, num_indices, GL_UNSIGNED_INT, 0);
		//glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		glBindVertexArray(VAO);

	}

	void update_states(State new_state) {

		prev_state = current_state;
		current_state = new_state;
	}

	State integrate(float h) {


		State new_state;
		// linear movement
		glm::vec4 new_velocity = current_state.linear_velocity + (h * current_state.force) / mass;
		new_state.position = current_state.position + h * new_velocity;

		new_state.linear_velocity = new_velocity;

		// angular movement
		glm::mat3 rotation_matrix = glm::mat3_cast(current_state.rotation_quat);
		glm::mat3 world_inertia = rotation_matrix * glm::inverse(inertia_matrix) * glm::transpose(rotation_matrix);

		glm::vec3 angular_momentum = inertia_matrix * current_state.angular_velocity;

		//angular velocity
		glm::vec3 omega = world_inertia * angular_momentum;
		glm::quat omega_quat = glm::quat(0, omega);
		glm::quat omega_quat_prime = (0.5f) * omega_quat * current_state.rotation_quat;
		new_state.angular_velocity = current_state.angular_velocity + h * glm::vec4(world_inertia * current_state.torque.xyz(), 0.0);
		new_state.rotation_quat = current_state.rotation_quat + h * omega_quat_prime;

		new_state.rotation_quat = glm::normalize(new_state.rotation_quat);
		update_local_coords(&new_state);

		return new_state;
	}



	void update_local_coords(State *new_state) {

		new_state->rotation_matrix = glm::mat3_cast(new_state->rotation_quat);
		new_state->local_coords_matrix = glm::transpose(glm::mat4(glm::vec4(new_state->rotation_matrix[0], 0.0),
			glm::vec4(new_state->rotation_matrix[1], 0.0),
			glm::vec4(new_state->rotation_matrix[2], 0.0),
			new_state->position));
	}

	void create_properties() {
		create_local_coords();
		create_inertia_matrix();
	}

	void create_local_coords() {
		glm::vec3 sum_of_position = glm::vec4(0.0, 0.0, 0.0, 0.0);
		//calculate center of mass
		float total_mass = 0;
		current_state.position = current_state.position + glm::vec4(sum_of_position * (1.0f / total_mass), 1.0);
		current_state.rotation_matrix = glm::mat3_cast(current_state.rotation_quat);

		current_state.local_coords_matrix = glm::transpose(glm::mat4(glm::vec4(current_state.rotation_matrix[0], 0.0),
			glm::vec4(current_state.rotation_matrix[1], 0.0),
			glm::vec4(current_state.rotation_matrix[2], 0.0),
			current_state.position));

	}

	
	virtual void create_inertia_matrix() {
		std::cout << "inertia matrix for rigid body\n";
		//since our object is a box, the moment of inertia is quite simple to compute
		/*
		float length = glm::length(box_mesh[0] - box_mesh[3]);
		float width = glm::length(box_mesh[0] - box_mesh[4]);
		float height = glm::length(box_mesh[0] - box_mesh[1]);

		glm::vec3 row1 = glm::vec3((mass / 12.0f) * (pow(width, 2) + pow(height, 2)), 0.0, 0.0);
		glm::vec3 row2 = glm::vec3(0.0, (mass / 12.0f) * ((length, 2) + pow(width, 2)), 0.0);
		glm::vec3 row3 = glm::vec3(0.0, 0.0, (mass / 12.0f) * (pow(length, 2) + pow(height, 2)));

		inertia_matrix = glm::mat3(row1, row2, row3);
		*/
	}


	void load_model() {
		//for loading model
	}

	void integrate_body(float curr_time) {
		//what integration is best for rigid bodies, let not do verlet this time
	}

private:


};


#endif

