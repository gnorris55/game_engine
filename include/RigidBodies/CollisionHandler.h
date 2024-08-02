#ifndef COLLISION_HANDLER
#define COLLISION_HANDLER

#include <RigidBodies/Cube.h>
#include <RigidBodies/Plane.h>

class PhysicsManager {

public:
	
	void add_box(Cube* box) {
		std::cout << "added box\n";
		boxes.push_back(box);
	}
	void add_plane(Plane* plane) {
		planes.push_back(plane);
	}
	void physics_step() {
		handle_cubes();
		//cube_on_cube();
	}

private:

	std::vector<Cube*> boxes;
	std::vector<Plane*> planes;

	/*void handle_cube_on_cube(Cube* boxA, Cube* boxB) {
		//two types of collisions:
		// vertex on face edge on edge
		glm::vec3 minA = boxA->get_mins();
		glm::vec3 minB = boxB->get_mins();
		glm::vec3 maxA = boxA->get_maxes();
		glm::vec3 maxB = boxB->get_maxes();

		//std::cout << "minA: " << glm::to_string(minA) << "\n";
		//std::cout << "minB: " << glm::to_string(minB) << "\n";
		//std::cout << "maxA: " << glm::to_string(maxA) << "\n";
		//std::cout << "maxB: " << glm::to_string(maxB) << "\n";
		

		//TODO:
		if ((minB.x > maxA.x) || (minA.x > maxB.x)
			|| (minB.y > maxA.y) || (minA.y > maxB.y)
			|| (minB.z > maxA.z) || (minA.z > maxB.z))
			return;
		else {
			std::cout << "box collision!\n";
			for (int i = 0; i < boxA->box_mesh.size(); i++) {
				glm::vec4 world_point = boxA->box_mesh[i] * boxA->local_coords_matrix;
				for (int j = 0; j < boxB->faces.size(); j++) {
					FacePlane curr_face = boxB->faces[j];
					glm::vec4 difference = world_point - boxB->box_mesh[curr_face.points[0]] * boxB->local_coords_matrix;
					float plane_dot = glm::dot(difference.xyz(), curr_face.normal.xyz()*boxB->rotation_matrix);
					if (plane_dot < 0)
						in_polygon(world_point, curr_face, boxA, boxB, plane_dot);
				}
			}
		}
	}*/
	/*
	bool in_polygon(glm::vec4 point, FacePlane face, Cube *boxA, Cube *boxB, float plane_dot) {
		//std::cout << "dot of plane: " << plane_dot << "\n";
		//std::cout << "box A linear velocity: " << glm::to_string(boxA->linear_velocity) << "\n";
		
		glm::vec3 curr_normal = face.normal.xyz() * boxB->current_state.rotation_matrix;
		glm::vec4 p1 = boxB->box_mesh[face.points[0]] * boxB->local_coords_matrix;
		glm::vec4 p2 = boxB->box_mesh[face.points[1]] * boxB->local_coords_matrix;
		glm::vec4 p3 = boxB->box_mesh[face.points[2]] * boxB->local_coords_matrix;
		glm::vec4 p4 = boxB->box_mesh[face.points[3]] * boxB->local_coords_matrix;
		//float hit = plane_dot / (glm::dot(boxA->linear_velocity.xyz(), face.normal.xyz()*boxB->rotation_matrix));
		//std::cout << "hit scalar: " << hit << "\n";
		//glm::vec4 hit_point = point - hit * boxA->linear_velocity;
		std::cout << "our hit point: " << glm::to_string(point) << "\n";
		std::cout << "normal: " << glm::to_string(curr_normal) << "\n";
		

		glm::vec4 rA = point - boxA->center_of_mass;
		glm::vec3 boxA_vel = boxA->linear_velocity.xyz() + glm::cross(boxA->angular_velocity.xyz(), rA.xyz());
		
		glm::vec4 rB = point - boxB->center_of_mass;
		glm::vec3 boxB_vel = boxB->linear_velocity.xyz() + glm::cross(boxB->angular_velocity.xyz(), rB.xyz());

		float relative_velocity = glm::dot(curr_normal.xyz(), (boxA_vel - boxB_vel));
		std::cout << "relative velocity: " << relative_velocity << "\n";

		if (curr_normal.x > curr_normal.y && curr_normal.x > curr_normal.z) {
			determinant(point.yz(), p1.yz(), p2.yz(), p3.yz(), p4.yz());
		}
		else if (curr_normal.y > curr_normal.x && curr_normal.y > curr_normal.z) {
			determinant(point.xz(), p1.xz(), p2.xz(), p3.xz(), p4.xz());
		}
		else {
			determinant(point.xy(), p1.xy(), p2.xy(), p3.xy(), p4.xy());
		}
		return false;
	}*/

	void determinant(glm::vec2 point, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4) {
		std::vector<glm::vec2> edge_vectors;
		std::vector<glm::vec2> middle_vectors;
		edge_vectors.push_back(p2 - p1);
		edge_vectors.push_back(p3 - p2);
		edge_vectors.push_back(p4 - p3);
		edge_vectors.push_back(p1 - p4);

		middle_vectors.push_back(point - p1);
		middle_vectors.push_back(point - p2);
		middle_vectors.push_back(point - p3);
		middle_vectors.push_back(point - p4);

		glm::mat2 p1_matrix = glm::mat2(edge_vectors[0], middle_vectors[0]);
		glm::mat2 p2_matrix = glm::mat2(edge_vectors[1], middle_vectors[1]);
		glm::mat2 p3_matrix = glm::mat2(edge_vectors[2], middle_vectors[2]);
		glm::mat2 p4_matrix = glm::mat2(edge_vectors[3], middle_vectors[3]);

		float p1_det = glm::determinant(p1_matrix);
		float p2_det = glm::determinant(p2_matrix);
		float p3_det = glm::determinant(p3_matrix);
		float p4_det = glm::determinant(p4_matrix);

		if (p1_det < 0 && p2_det < 0 && p3_det < 0 && p4_det < 0 || p1_det > 0 && p2_det > 0 && p3_det > 0 && p4_det > 0) {
			std::cout << "point: " << glm::to_string(point) << "has collided\n";
		}

	}


	void handle_cubes() {
		for (int i = 0; i < boxes.size(); i++) {
				integrate_cube(boxes[i]);	
		}
	}

	void integrate_cube(Cube* cube) {
		float epsilon = 0.001;
		float time_step_remaining = cube->time_step;
		float curr_time_step = time_step_remaining;
		int count = 0;
		while (time_step_remaining > 0) {
			
			cube->set_forces(); // sets forces for force and torque
			cube->integrate(curr_time_step);	
			
			int plane_index = -1;
			int point_index = -1;
			float time_frac = 1.0;
			plane_collision_checker(cube, plane_index, point_index, time_frac);

			if (plane_index >= 0) {
				float cube_plane_vel = glm::dot(cube->current_state.linear_velocity.xyz(), planes[plane_index]->normal.xyz());

				glm::vec4 prev_point = cube->box_mesh[point_index] * cube->prev_state.local_coords_matrix;
				glm::vec4 current_point = cube->box_mesh[point_index] * cube->current_state.local_coords_matrix;

				glm::vec4 difference = cube->box_mesh[point_index] * cube->current_state.local_coords_matrix - planes[plane_index]->position;
				float height_from_plane = glm::dot(difference.xyz(), planes[plane_index]->normal.xyz());
				
				if (height_from_plane > -0.00001) {
					glm::vec4 collision_point = cube->box_mesh[point_index] * cube->current_state.local_coords_matrix;
					project_point_onto_plane(cube, planes[plane_index], collision_point);
					collision_point = cube->box_mesh[point_index] * cube->current_state.local_coords_matrix;
				}
				else {
					if (time_frac > 0.0) {
						curr_time_step = time_frac * curr_time_step;
						cube->reintegrate(curr_time_step);
					}

					glm::vec4 collision_point = cube->box_mesh[point_index] * cube->current_state.local_coords_matrix;
					project_point_onto_plane(cube, planes[plane_index], collision_point);
					collision_point = cube->box_mesh[point_index] * cube->current_state.local_coords_matrix;

					plane_cube_collision_response(collision_point, cube, planes[plane_index]);
					cube_plane_vel = glm::dot(cube->current_state.linear_velocity.xyz(), planes[plane_index]->normal.xyz());
				}
			}
			count++;
			time_step_remaining -= curr_time_step;
			curr_time_step = time_step_remaining;
		}
	}

	void check_if_resting(Cube *cube, Plane *plane) {

		int count = 0;
		for (int i = 0; i < cube->box_mesh.size(); i++) {
				glm::vec4 point_velocity = cube->current_state.linear_velocity + glm::vec4(glm::cross(cube->current_state.angular_velocity.xyz(), (cube->box_mesh[i] * cube->current_state.local_coords_matrix - cube->current_state.center_of_mass).xyz()), 0.0);
				float relative_velocity = glm::dot(point_velocity.xyz(), plane->normal.xyz());
				
				glm::vec4 difference = cube->box_mesh[i] * cube->current_state.local_coords_matrix - plane->position;
				float height_from_plane = glm::dot(difference.xyz(), plane->normal.xyz());
				//std::cout << "rel vel: " << relative_velocity << "\n";
				//std::cout << "pos: " << abs(height_from_plane) << "\n";
				if (relative_velocity > -1.0 && relative_velocity < 1.0 && abs(height_from_plane) < 0.005)
					count++;

		}

		//std::cout << count << "\n";
		if (count == 4) {
			//std::cout << "should be resting\n";
			//cube->resting = true;
		}

	}

	float get_determination_time_step(Cube* cube, Plane* plane, int point_index) {
		
		glm::vec4 prev_diff = cube->box_mesh[point_index] * cube->prev_state.local_coords_matrix - plane->position;
		glm::vec4 curr_diff = cube->box_mesh[point_index] * cube->current_state.local_coords_matrix - plane->position;

		float prev_height = glm::dot(prev_diff.xyz(), plane->normal.xyz());
		float curr_height = glm::dot(curr_diff.xyz(), plane->normal.xyz());
		float tot_diff = ((double)prev_height - (double)curr_height);
		return (double)1.0 / (1- ((double)curr_height / (double)prev_height));


	}

	// returns the index of plane if a collision occurs
	int plane_collision_checker(Cube* cube, int& plane_i, int& point_i, float &time_frac) {

		// check collisions between planes and cube
		float min_time_step = 1.0f;
		int min_point_index = -1;
		int min_plane_index = -1;
		int count = 0;
		for (int i = 0; i < planes.size(); i++) {
			Plane* plane = planes[i];
			for (int j = 0; j < cube->box_mesh.size(); j++) {
				glm::vec4 point_velocity = cube->current_state.linear_velocity + glm::vec4(glm::cross(cube->current_state.angular_velocity.xyz(), (cube->box_mesh[j] * cube->current_state.local_coords_matrix - cube->current_state.center_of_mass).xyz()), 0.0);
				float init_relative_velocity = glm::dot(point_velocity.xyz(), plane->normal.xyz());
				//std::cout << "rel velocity for " << j << ": " << init_relative_velocity << "\n";

				glm::vec4 difference = cube->box_mesh[j] * cube->current_state.local_coords_matrix - plane->position;
				float height_from_plane = glm::dot(difference.xyz(), plane->normal.xyz());
				float curr_time_step = get_determination_time_step(cube, plane, j);
				if (height_from_plane < 0.0 && init_relative_velocity < 0.0) {
					count++;
					if (curr_time_step < min_time_step) {
						min_time_step = curr_time_step;
						min_point_index = j;
						min_plane_index = i;
					}
				}
			}
		}
		if (min_point_index > -1) {
			plane_i = min_plane_index;
			point_i = min_point_index;
			time_frac = min_time_step;
		}
		return count;
	}

	void project_point_onto_plane(Cube* cube, Plane* plane, glm::vec4 point) {
		
		// important for roundoff error because collision determination is almost always slightly inaccurate
		glm::vec3 PQ = point.xyz() - plane->position.xyz();

	    float projectedComponent = glm::dot(PQ, plane->normal.xyz());
		glm::vec3 projectedPoint = point.xyz() - projectedComponent*plane->normal.xyz();
		cube->displace_position(-projectedComponent * plane->normal);
	}


	void plane_cube_collision_response(glm::vec4 point, Cube* cube, Plane* plane) {
		glm::vec4 point_velocity = cube->current_state.linear_velocity + glm::vec4(glm::cross(cube->current_state.angular_velocity.xyz(), (point - cube->current_state.center_of_mass).xyz()), 0.0);
		float init_relative_velocity = glm::dot(point_velocity.xyz(), plane->normal.xyz());
		//std::cout << "relative velocity" << init_relative_velocity << "\n";
		if (init_relative_velocity < 0.0) {
		

			glm::mat3 inertia_matrix = cube->get_world_inertia_matrix();
			glm::vec4 r = point - cube->current_state.center_of_mass;
			glm::vec3 tot_vel = cube->current_state.linear_velocity.xyz() + glm::cross(r.xyz(), cube->current_state.angular_velocity.xyz());

			float cr = 0.1f;

			float bottom = glm::dot(plane->normal.xyz(), glm::cross(inertia_matrix * glm::cross(r.xyz(), plane->normal.xyz()), r.xyz()));
			float j = (-(1 + cr) * init_relative_velocity) / ((1 / cube->mass) + bottom);
			glm::vec4 delta_v = (1/cube->mass) * j * plane->normal;
			glm::vec4 delta_omega = glm::vec4(j * inertia_matrix * glm::cross(r.xyz(), plane->normal.xyz()), 0.0);
			
			cube->prev_state.linear_velocity = cube->current_state.linear_velocity;
			cube->prev_state.angular_velocity = cube->current_state.angular_velocity;

			cube->current_state.linear_velocity = cube->current_state.linear_velocity + delta_v;
			cube->current_state.angular_velocity = cube->current_state.angular_velocity + delta_omega;

			point_velocity = cube->current_state.linear_velocity + glm::vec4(glm::cross(cube->current_state.angular_velocity.xyz(), (point - cube->current_state.center_of_mass).xyz()), 0.0);
			tot_vel = cube->current_state.linear_velocity.xyz() + glm::cross(r.xyz(), cube->current_state.angular_velocity.xyz());
			float new_relative_velocity = glm::dot(point_velocity.xyz(), plane->normal.xyz());
			
			// now add friction
			float friction_c = 0.01;
			glm::vec4 friction_linear_velocity = cube->current_state.linear_velocity * friction_c;
			glm::vec4 friction_angular_velocity = cube->current_state.angular_velocity * friction_c;
			cube->current_state.linear_velocity = cube->current_state.linear_velocity - friction_linear_velocity;
			cube->current_state.angular_velocity = cube->current_state.angular_velocity - friction_angular_velocity;

			//std::cout << "new relative velocity: " << new_relative_velocity << "\n";
		}
	}

		

};

#endif 