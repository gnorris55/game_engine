#ifndef SCENE_RENDERER_H
#define SCENE_RENDERER_H


class SceneRenderer {


public:
	
	std::vector<Entity *> entities;
	Camera* camera;

	SceneRenderer(Camera* camera) {
		this->camera = camera;
	}

	void render_scene() {
		for (Entity *entity : entities) {
			entity->draw(camera->GetProjection(), camera->GetViewMatrix(), camera->Position);
		}
	}

	void add_entity(Entity *new_entity) {
		entities.push_back(new_entity);
	}

	



};

#endif