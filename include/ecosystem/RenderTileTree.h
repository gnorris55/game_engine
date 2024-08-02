#ifndef RENDER_TILE_TREE_H
#define RENDER_TILE_TREE_H 

#include <ecosystem/render_tile.h>

class RenderTileTree {


public:

	Shader* grass_shader;
	ComputeShader* clipping_shader;
	unsigned int grass_heights_tex, grass_color_tex, wind_tex;
	unsigned int clipping_tex;
	Loader loader;
	glm::mat4 terrain_model;


	struct TileTreeNode {
		TileTreeNode* top_left;
		TileTreeNode* top_right;
		TileTreeNode* bottom_left;
		TileTreeNode* bottom_right;
		float size;
		int i;
		int j;
		glm::vec4 position;
	};

	TileTreeNode root;
	std::vector<std::vector<RenderTile*>> render_tiles;

	RenderTileTree(Shader *grass_shader, ComputeShader *clipping_shader, glm::mat4 terrain_model, float terrain_size, float num_division, glm::vec4 terrain_position, std::vector<float>height_map) {
		

		this->grass_shader = grass_shader;
		this->clipping_shader = clipping_shader;
		this->terrain_model = terrain_model;
		root.position = glm::vec4(0, 0, 0, 1);
		root.size = 128;
		load_textures();
		create_render_tile_tree(&root, 0);
		define_render_tiles(terrain_size, num_division, terrain_position, height_map);
	}

	~RenderTileTree() {
		for (int i = 0; i < render_tiles.size(); i++) {
			for (RenderTile* renderTile : render_tiles[i]) {
				delete renderTile;
			}
		}
	}


	void draw(float time, glm::mat4 proj, glm::mat4 view, glm::vec3 view_pos, Frustum* frustum) {
		get_clipping_tex(proj * view, view_pos);
		render_grass_tiles(time, proj * view, view_pos, frustum);
	}



private:


	//creating class
	void create_render_tile_tree(TileTreeNode* node, int count) {

		//std::cout << count << "\n";
		if (node->size >= 2) {
			float step = node->size / 2.0f;
			node->top_left = create_node(node->position, step);
			node->top_right = create_node(node->position + glm::vec4(1.0, 0.0, 0.0, 0.0) * step, step);
			node->bottom_left = create_node(node->position + glm::vec4(0.0, 0.0, 1.0, 0.0) * step, step);
			node->bottom_right = create_node(node->position + glm::vec4(1.0, 0.0, 1.0, 0.0) * step, step);

			create_render_tile_tree(node->top_left, count + 1);
			create_render_tile_tree(node->top_right, count + 1);
			create_render_tile_tree(node->bottom_left, count + 1);
			create_render_tile_tree(node->bottom_right, count + 1);
		}
		if (node->size == 2) {
			node->i = abs(node->position.z / node->size);
			node->j = (node->position.x) / node->size;

		}


	}
	void create_clipping_texture() {
		glGenTextures(1, &clipping_tex);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, clipping_tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 64, 64, 0, GL_RGBA, GL_FLOAT, NULL);

		glBindImageTexture(0, clipping_tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	}

	void load_textures() {
		//const char* texture_name = "Textures / dirt.jpg";
		grass_heights_tex = loader.load_texture("Textures/clumping2.jpg");
		grass_color_tex = loader.load_texture("Textures/grass_color.jpg");
		wind_tex = loader.load_texture("Textures/wind.jpg");
		create_clipping_texture();
	}


	TileTreeNode* create_node(glm::vec4 position, float size) {
		TileTreeNode* temp_node = new TileTreeNode;
		temp_node->size = size;
		temp_node->position = position;
		return temp_node;
	}


	glm::vec3 get_vec3_from_texture_array(int i, int j, float tex_array[]) {
		float winds[65 * 65 * 3];
		glm::vec3 rtn_vec;
		rtn_vec.x = tex_array[(i * 64 + j) * 3];
		rtn_vec.y = tex_array[(i * 64 + j) * 3 + 1];
		rtn_vec.z = tex_array[(i * 64 + j) * 3 + 2];

		return rtn_vec;
	}

	void define_render_tiles(float terrain_size, float num_division, glm::vec4 terrain_position, std::vector<float> height_map) {
		float tile_step = (float)terrain_size / (float)num_division;
		ComputeShader compute_shader("Shaders/computeShader.cs");
		float heights[64 * 64];
		float winds[64 * 64 * 3];
		float grass_color[64 * 64 * 3];

		glBindTexture(GL_TEXTURE_2D, grass_heights_tex);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, heights);

		glBindTexture(GL_TEXTURE_2D, wind_tex);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, winds);

		glBindTexture(GL_TEXTURE_2D, grass_color_tex);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, grass_color);

		std::cout << "loading\n";
		for (int i = 0; i < 64; i++) {
			std::vector<RenderTile*> temp_vec;
			for (int j = 0; j < 64; j++) {
				//std::cout << i << ", " << j << ": " << heights[i * 64 + j] << "\n";
				glm::vec3 wind_normal = get_vec3_from_texture_array(i, j, winds);
				glm::vec3 tile_color = get_vec3_from_texture_array(i, j, grass_color);


				glm::vec4 tile_position = terrain_position;
				tile_position.x += j * tile_step;
				tile_position.y = terrain_position.y;
				tile_position.z += i * tile_step;
				std::vector<float> point_heights = { height_map[i * 65 + j],  height_map[(i * 65) + (j + 1)], height_map[(i + 1) * 65 + j], height_map[(i + 1) * 65 + (j + 1)] };


				RenderTile* temp_tile = new RenderTile(tile_position, point_heights, tile_step, heights[i * 64 + j], glm::normalize(wind_normal), tile_color, &compute_shader, grass_shader);
				temp_vec.push_back(temp_tile);

			}
			render_tiles.push_back(temp_vec);

		}

	}

	//drawing
	void render_grass_tiles(float time, glm::mat4 proj_view, glm::vec3 view_pos, Frustum* frustum) {

		grass_shader->use();
		grass_shader->setVec3("viewPos", view_pos);
		grass_shader->setMat4("projView", proj_view);
		grass_shader->setMat4("model", terrain_model);


		float clipping_coord[64 * 64 * 3];
		glBindTexture(GL_TEXTURE_2D, clipping_tex);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, clipping_coord);


		draw_with_frustum_culling(&root, frustum, time, clipping_coord);
		/*for (int i = 0; i < 64; i++) {
			for (int j = 0; j < 64; j++) {
				render_tiles[i][j]->drawGrass(time);
			}

		}*/
	}

	void draw_with_frustum_culling(TileTreeNode* node, Frustum* frustum, float time, float clipping_coord[]) {

		if (node->size > 2.0) {

			glm::vec3 extents = glm::vec3(node->size / 2.0, 2.0, node->size / 2.0);
			glm::vec3 center = ((node->position * terrain_model + glm::vec4(node->size / 2.0f, 0.0, node->size / 2.0f, 0.0))).xyz();

			bool is_in_frustum = frustum->in_frustum(extents, center);
			//std::cout << "in frustum: " << is_in_frustum << "\n";
			if (is_in_frustum) {
				draw_with_frustum_culling(node->top_left, frustum, time, clipping_coord);
				draw_with_frustum_culling(node->top_right, frustum, time, clipping_coord);
				draw_with_frustum_culling(node->bottom_left, frustum, time, clipping_coord);
				draw_with_frustum_culling(node->bottom_right, frustum, time, clipping_coord);
			}
		}
		else {
			if (clipping_coord[(node->i * 64 + node->j) * 3] == 0.5) {
				render_tiles[node->i][node->j]->set_poly(1);
				render_tiles[node->i][node->j]->drawGrass(time);
			}
			else if (clipping_coord[(node->i * 64 + node->j) * 3] > 0.5) {
				render_tiles[node->i][node->j]->set_poly(0);
				render_tiles[node->i][node->j]->drawGrass(time);
			}
			else {
				render_tiles[node->i][node->j]->set_poly(2);

			}

			//render_tiles[node->i][node->j]->drawGrass(time);

		}
	}

	void get_clipping_tex(glm::mat4 proj_view, glm::vec3 camera_position) {
		clipping_shader->use();
		clipping_shader->setMat4("projView", proj_view);
		clipping_shader->setMat4("model", terrain_model);
		clipping_shader->setVec3("camera_position", camera_position);

		glBindImageTexture(0, clipping_tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glDispatchCompute((unsigned int)64, (unsigned int)64, 1);

		// make sure writing to image has finished before read
		//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}


};

#endif
