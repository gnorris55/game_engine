#ifndef ENTITY_H
#define ENTITY_H

class Entity {


public:
	virtual void draw(glm::mat4 proj, glm::mat4 view, glm::vec3 camera_position) {
		std::cout << "drawing entity\n";

	}

	void set_position(glm::vec3 position) {
		this->transform_matrix = glm::transpose(glm::translate(this->transform_matrix, position));
	}

protected:


	Shader *shader;
	std::vector<Entity *> children;
	glm::mat4 transform_matrix = glm::mat4(1.0f);

	Entity(Shader *shader, glm::vec3 starting_position) {
		this->shader = shader;
		this->transform_matrix = glm::transpose(glm::translate(this->transform_matrix, starting_position));
	}
	
	Shader *get_shader() {
		return shader;
	}

};


#endif