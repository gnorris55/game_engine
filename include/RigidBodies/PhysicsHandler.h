#ifndef PHYSICS_HANDLER
#define PHYSICS_HANDLER

struct State {
	glm::vec4 position = glm::vec4(0.0, 0.0, 0.0, 1.0);
	glm::quat rotation_quat = glm::quat(0.0, 0.0, 0.0, 0.0);
	glm::vec4 linear_velocity = glm::vec4(0.0, 0.0, 0.0, 0.0);
	glm::vec4 angular_velocity = glm::vec4(0, 0, 0, 0);
	glm::mat4 local_coords_matrix = glm::mat4(1.0f);
	glm::mat3 rotation_matrix = glm::mat3(1.0f);
	glm::vec4 force = glm::vec4(0.0, 0.0, 0.0, 0.0);
	glm::vec4 torque = glm::vec4(0.0, 0.0, 0.0, 0.0);
};

#include <RigidBodies/Plane.h>
#include <RigidBodies/sphere.h>

class PhysicsManager {

public:

	void add_sphere(Sphere* sphere) {
		std::cout << "added box\n";
		spheres.push_back(sphere);
	}
	void add_plane(Plane* plane) {
		planes.push_back(plane);
	}
	void physics_step(float time_step) {
		handle_spheres();
		this->time_step = time_step;
		//cube_on_cube();
	}

	void test_bisection() {
		std::cout << bisection(1.0f, -6.8) << "\n";
	}

private:

	std::vector<Sphere*> spheres;
	std::vector<Plane*> planes;
	float time_step = 0.01;

	void handle_spheres() {
		for (int i = 0; i < spheres.size(); i++) {
			integrate_sphere(spheres[i]);
		}
	}

	void integrate_sphere(Sphere* sphere) {
		float epsilon = 0.001;
		float time_step_remaining = time_step;
		float curr_time_step = time_step_remaining;
		int count = 0;
		int plane_index = -1;

		while (time_step_remaining > 0) {
			// determine accelerations


			sphere->set_forces();
			
			//


			State new_state = sphere->integrate(curr_time_step);

			// will make the plane index = to the collided with plane index
			float collision_frac = plane_collision_checker(sphere, &new_state, plane_index);

			//std::cout << "collision fraction: " << collision_frac << "\n";
			if (plane_index >= 0.0) {
				Plane* plane = planes[plane_index];
				curr_time_step = time_step_remaining * collision_frac;
				new_state = sphere->integrate(curr_time_step);
				plane_collision_response(&new_state, sphere, plane);
			}
			if (count > 5) {
				std::cout << "limit reached, time remaining: " << time_step_remaining << "\n";
				break;
			}
			count++;

			time_step_remaining -= curr_time_step;
			curr_time_step = time_step_remaining;
			plane_index = -1;
			sphere->update_states(new_state);
		}
		sphere->input_acc = glm::vec4(0.0, 0.0, 0.0, 0.0);
		sphere->input_acc = glm::vec4(0.0, 0.0, 0.0, 0.0);
	}


	float plane_collision_checker(Sphere *sphere, State *new_state, int &plane_index) {
		
		float smallest_height = 0.0f;
		float smallest_height_index = -1.0f;
		for (int i = 0; i < planes.size(); i++) {
			Plane* plane = planes[i];

			glm::vec4 r = plane->normal * -sphere->radius;
			glm::vec4 angular_velocity = glm::vec4(glm::cross(new_state->angular_velocity.xyz(), r.xyz()), 0.0);
			glm::vec4 total_velocity = new_state->linear_velocity + angular_velocity;
			float init_relative_velocity = glm::dot(total_velocity.xyz(), plane->normal.xyz());
			float init_relative_acc = glm::dot((1.0f / sphere->mass) * sphere->current_state.force.xyz(), plane->normal.xyz());
			glm::vec4 point_velocity = sphere->current_state.linear_velocity;
			float relative_height = plane->get_height_from_plane(new_state->position) - sphere->radius;

			//if (relative_height < smallest_height)
				//plane_index = i;
			if (relative_height < 0.0)
				coulomb_friction_LCP(sphere, plane, new_state);
		
			resting_LCP(sphere, plane, relative_height, init_relative_velocity, init_relative_acc);

			if (abs(relative_height) < 0.001) {
				glm::vec4 collision_point = new_state->position + plane->normal * -sphere->radius;
				glm::vec4 displacement = project_point_onto_plane(plane, collision_point);
				new_state->position = new_state->position + displacement;
				
				return 0.0;
			}

			if (relative_height < 0.0 && init_relative_velocity < 0.0) {
				bool prev = true;
				float prev_relative_height = plane->get_height_from_plane(sphere->current_state.position) - sphere->radius;
				plane_index = i;
				return prev_relative_height / (prev_relative_height - relative_height);
			}
			
		}

		return 0.0;

	}

	void plane_collision_response(State* new_state, Sphere *sphere, Plane* plane) {
		glm::vec4 r = plane->normal * -sphere->radius;
		glm::vec4 angular_velocity = glm::vec4(glm::cross(new_state->angular_velocity.xyz(), r.xyz()), 0.0);
		glm::vec4 total_velocity = new_state->linear_velocity + angular_velocity;
		float init_relative_velocity = glm::dot(total_velocity.xyz(), plane->normal.xyz());
		//std::cout << "total velocity: " << glm::to_string(total_velocity) << "\n";
		if (init_relative_velocity < 0.0) {
			glm::mat3 inertia_matrix = new_state->rotation_matrix*glm::inverse(sphere->inertia_matrix)*glm::transpose(new_state->rotation_matrix);
			
			// drag here
			float cr = 0.5;

			float bottom = glm::dot(plane->normal.xyz(), glm::cross(inertia_matrix * glm::cross(r.xyz(), plane->normal.xyz()), r.xyz()));
			float j = (-(1 + cr) * init_relative_velocity) / ((1 / sphere->mass) + bottom);
			glm::vec4 delta_v = (1 / sphere->mass) * j * plane->normal;
			glm::vec4 delta_omega = glm::vec4(j * inertia_matrix * glm::cross(r.xyz(), plane->normal.xyz()), 0.0);

			new_state->linear_velocity = new_state->linear_velocity + delta_v;
			new_state->angular_velocity = new_state->angular_velocity + delta_omega;
		

			
		}
	}
	
	void coulomb_friction_LCP(Sphere* sphere, Plane* plane, State *new_state) {
		glm::vec4 r = plane->normal * -sphere->radius;
		glm::vec4 angular_velocity = glm::vec4(glm::cross(new_state->angular_velocity.xyz(), r.xyz()), 0.0);
		glm::vec4 total_velocity = new_state->linear_velocity + angular_velocity;
		float init_relative_velocity = glm::dot(total_velocity.xyz(), plane->normal.xyz());
		float init_relative_acc = glm::dot((1.0f / sphere->mass) * sphere->current_state.force.xyz(), plane->normal.xyz());
		glm::vec4 point_velocity = sphere->current_state.linear_velocity;
		float relative_height = plane->get_height_from_plane(new_state->position) - sphere->radius;

		// tempary tangent vector
		glm::vec3 plane_tangent = glm::cross(plane->normal.xyz(), glm::normalize((glm::normalize(glm::normalize(plane->plane_points[0]) - glm::normalize(plane->plane_points[1]))).xyz()));
		float rel_tangent_acc = glm::dot((1.0f / sphere->mass) * sphere->current_state.force.xyz(), plane_tangent);
		glm::vec4 tangent_vel = total_velocity - glm::dot(total_velocity.xyz(), plane->normal.xyz())* plane->normal;
		float rel_tangent_vel = glm::length(tangent_vel);
		//std::cout << "total velocity" << glm::to_string(total_velocity) << "\n";

		//std::cout << "tangent vel: " << glm::to_string(tangent_vel) << "\n";
		//std::cout << "rel tangent vel: " << rel_tangent_vel << "\n";
		float friction_c = 0.01;
		glm::vec4 friction = new_state->linear_velocity - new_state->linear_velocity *friction_c;
		glm::vec4 angular_friction = new_state->linear_velocity - tangent_vel * friction_c;
		
		//sphere->friction_acc = friction;
		new_state->linear_velocity = friction;
		new_state->angular_velocity = angular_friction;


		//std::cout << "tangent: " << glm::to_string(tangent_vel) << "\n";
		//std::cout << "relative tangent vel: " << rel_tangent_vel << "\n";
		//std::cout << "relative tangent vel: " << init_relative_acc << "\n";
		//std::cout << "relative normal vel: " << init_relative_velocity << "\n";
		//std::cout << "relative tangent acc: " << rel_tangent_acc << "\n";
		float knn = 1 / sphere->mass;
		float ktt = 1 / sphere->mass;
		float knt = 0;
		float friction_coefficient = 0.1;

	}

	double bisection(float knn, float rel_acc) {
		double iterative_error = 0.0001;
		double left = 0.0;
		double right = 100.0;
		int i = 0;

		while (right - left >= iterative_error) {

			
			double middle = (right + left) / 2;
			double left_length = sqrt(pow(left,2) + pow(knn*left+rel_acc, 2)) - left - (knn * left + rel_acc);
			double right_length = sqrt(pow(right, 2) + pow(knn * right + rel_acc, 2)) - right - (knn * right + rel_acc);
			double middle_length = sqrt(pow(middle, 2) + pow(knn * middle + rel_acc, 2)) - middle - (knn * middle + rel_acc);

			std::cout << "y: " << left_length << "\n";
			std::cout << "x: " << left << "\n";
			if (middle_length == 0) return middle;
			else if (left_length * middle_length <= 0) right = middle;
			else left = middle;
			i++;

		}
		return left;
	}


	void resting_LCP(Sphere *sphere, Plane *plane, float relative_height, float init_relative_velocity, float init_relative_acc) {
			if (init_relative_acc > 0.0 && glm::dot(sphere->additional_acc.xyz(), plane->normal.xyz()) > 0) {
				sphere->additional_acc = glm::vec4(0.0, 0.0, 0.0, 0.0);
				std::cout << "not resting\n";
			}

			if (abs(relative_height) < 0.001) {
				if (init_relative_velocity > -0.1 && init_relative_velocity < 0.0 && init_relative_acc < 0.0) {
					sphere->additional_acc = plane->normal * -init_relative_acc;
					std::cout << "resting\n";
				}
			}
	}

	glm::vec4 project_point_onto_plane(Plane* plane, glm::vec4 point) {
		glm::vec3 PQ = point.xyz() - plane->position.xyz();

		float projectedComponent = glm::dot(PQ, plane->normal.xyz());
		glm::vec3 projectedPoint = point.xyz() - projectedComponent * plane->normal.xyz();
		return -projectedComponent * plane->normal;
	}



};

#endif #pragma once
