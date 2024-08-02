#ifndef TERRAIN_H
#define TERRAIN_H

class Terrain {

public:


	Shader* shader;
	Shader* grass_shader;
	ComputeShader* clipping_shader;
	Renderer renderer;
	glm::vec4 position;
	glm::mat4 model;
	std::vector<RawModel> terrain_mesh;
	std::vector<std::vector<RenderTile*>> render_tiles;
	std::vector<float> height_map;
	bool has_grass;

	unsigned int texture;
	unsigned int clumping_tex;
	unsigned int grass_heights_tex;
	unsigned int wind_texture;
	unsigned int height_map_tex;
	unsigned int grass_color_tex;
	unsigned int clipping_tex;

	struct TileTreeNode {
		TileTreeNode *top_left;
		TileTreeNode *top_right;
		TileTreeNode *bottom_left;
		TileTreeNode *bottom_right;
		float size;
		int i;
		int j;
		glm::vec4 position;
	};

	TileTreeNode root_render_tiles;

	Terrain(Shader* shader, Shader* grass_shader, ComputeShader *clipping_shader, glm::vec4 position, const char* grass_heights_name, const char* terrain_height_name, bool has_grass = true) {
		this->shader = shader;
		this->grass_shader = grass_shader;
		this->position = position;
		this->has_grass = has_grass;
		
		this->model = glm::transpose(glm::translate(glm::mat4(1.0f), position.xyz()));
		//ComputeShader temp_compute("Shaders/clippingShader.cs");
		this->clipping_shader = clipping_shader;

		root_render_tiles.position = glm::vec4(0, 0, 0, 1);
		root_render_tiles.size = 128;
		create_render_tile_tree(&root_render_tiles, 0);
		load_textures(grass_heights_name, terrain_height_name);
		createTerrainMesh();

		if (has_grass) defineRenderTiles();
		create_clipping_texture();
	}

	~Terrain() {
		for (int i = 0; i < render_tiles.size(); i++) {
			for (RenderTile* renderTile : render_tiles[i]) {
				delete renderTile;
			}
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

	void get_clipping_tex(glm::mat4 proj_view, glm::vec3 camera_position) {
		clipping_shader->use();
		clipping_shader->setMat4("projView", proj_view);
		clipping_shader->setMat4("model", model);
		clipping_shader->setVec3("camera_position", camera_position);


		glBindImageTexture(0, clipping_tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glDispatchCompute((unsigned int)64, (unsigned int)64, 1);

		// make sure writing to image has finished before read
		//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);



	}
	
	bool between_neg_pos_one(float a) {
		return (a > -1.0 && a < 1.0);
	}
	
	void draw(float time, glm::mat4 proj, glm::mat4 view, glm::vec3 view_pos, Frustum *frustum) {

		shader->use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		RawModel temp_mesh = terrain_mesh[0];
		shader->setVec3("viewPos", view_pos);
		shader->setMat4("projection", proj);
		shader->setMat4("view", view);
		glUniform3fv(glGetUniformLocation(shader->ID, "objectColor"), 1, glm::value_ptr(glm::vec3(0.7, 0.7, 0.7)));
		glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
		renderer.render(temp_mesh, GL_TRIANGLES);

		// rendering grass for everytile
		get_clipping_tex(proj*view, view_pos);
		if (has_grass) render_grass_tiles(time, proj*view, view_pos, frustum);


	}

	void render_grass_tiles(float time, glm::mat4 proj_view, glm::vec3 view_pos, Frustum *frustum) {
		
		grass_shader->use();
		grass_shader->setVec3("viewPos", view_pos);
		grass_shader->setMat4("projView", proj_view);
		grass_shader->setMat4("model", model);

		
		float clipping_coord[64 * 64*3];
		glBindTexture(GL_TEXTURE_2D, clipping_tex);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, clipping_coord);
		
		draw_with_frustum_culling(&root_render_tiles, frustum, time, clipping_coord);
	}

	void draw_with_frustum_culling(TileTreeNode *node, Frustum *frustum, float time, float clipping_coord[]) {
		
		if (node->size > 2.0) {

			glm::vec3 extents = glm::vec3(node->size / 2.0, 2.0, node->size / 2.0);
			glm::vec3 center = ((node->position*model + glm::vec4(node->size / 2.0f, 0.0, node->size / 2.0f, 0.0))).xyz();

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

		}
	}

	bool inside_frustum(Frustum *frustum, glm::vec4 p1, glm::vec4 p2, glm::vec4 p3, glm::vec4 p4) {
		//return (frustum->in_frustum(p1.xyz()) || frustum->in_frustum(p2.xyz()) || frustum->in_frustum(p3.xyz()) || frustum->in_frustum(p4.xyz()));
		return true;
	}

	
private:
	const int TERRAIN_SIZE = 128;
	const int SQUARE_SIZE = 18;
	const int NUM_DIVISION = 64;
	glm::vec3 points[65][65];
	glm::vec2 globalTexCoord[65][65];
	Loader loader;


	void create_render_tile_tree(TileTreeNode *node, int count) {

		//std::cout << count << "\n";
		if (node->size >= 2) {
			float step = node->size / 2.0f;
			node->top_left = create_node(node->position, step);
			node->top_right = create_node(node->position + glm::vec4(1.0, 0.0, 0.0, 0.0)*step, step);
			node->bottom_left = create_node(node->position + glm::vec4(0.0, 0.0, 1.0, 0.0)*step, step);
			node->bottom_right = create_node(node->position + glm::vec4(1.0, 0.0, 1.0, 0.0)*step, step);

			create_render_tile_tree(node->top_left, count+1);
			create_render_tile_tree(node->top_right, count+1);
			create_render_tile_tree(node->bottom_left, count+1);
			create_render_tile_tree(node->bottom_right, count+1);
		}
		if (node->size == 2) {
			node->i = abs(node->position.z / node->size);
			node->j = (node->position.x) / node->size;
	
		}


	}

	TileTreeNode *create_node(glm::vec4 position, float size) {
		TileTreeNode *temp_node = new TileTreeNode;
		temp_node->size = size;
		temp_node->position = position;
		return temp_node;
	}
	

	void createTerrainMesh() {
	
		//const int num_vertices = 73728;
		const int num_vertices = 65*65*3*6;
		const int num_tex = 65*65*2*6;
		float vertices[num_vertices];
		float normals[num_vertices];
		float tex_coord[num_tex];

		setPoints();
		mapVertices(vertices, normals, tex_coord);

		RawModel temp_rawModel = loader.loadToVAOTexture(vertices, normals, tex_coord, sizeof(vertices) - (((NUM_DIVISION+1) * sizeof(float) * SQUARE_SIZE) * 2) + (sizeof(float) * SQUARE_SIZE), sizeof(tex_coord) - (((NUM_DIVISION+1) * sizeof(float) * SQUARE_SIZE) * 2) + (sizeof(float) * SQUARE_SIZE));
		//RawModel temp_rawModel = loader.loadToVAOTexture(vertices, normals, tex_coord, sizeof(vertices) - ((NUM_DIVISION * sizeof(float) * SQUARE_SIZE) * 2) + (sizeof(float) * SQUARE_SIZE), num_tex * sizeof(float));
		terrain_mesh.push_back(temp_rawModel);

	}
	unsigned int load_texture(const char *texture_name) {

		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// load image, create texture and generate mipmaps
		int width, height, nrChannels;
		// The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
		unsigned char* data = stbi_load(texture_name, &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}
		stbi_image_free(data);
		return texture;
	}

	void load_textures(const char *grass_heights_name, const char *terrain_heights_name) {
		//const char* texture_name = "Textures / dirt.jpg";
		texture = load_texture("Textures/grass.jpg");
		grass_heights_tex = load_texture(grass_heights_name);
		grass_color_tex= load_texture("Textures/grass_color.jpg");
		height_map_tex = load_texture(terrain_heights_name);
		wind_texture = load_texture("Textures/wind.jpg");
	}

	glm::vec3 get_vec3_from_texture_array(int i, int j, float tex_array[]) {
		float winds[65 * 65*3];
		glm::vec3 rtn_vec;
		rtn_vec.x = tex_array[(i * 64 + j) * 3];
		rtn_vec.y = tex_array[(i * 64 + j) * 3 + 1];
		rtn_vec.z = tex_array[(i * 64 + j) * 3 + 2];

		return rtn_vec;
	}

	void defineRenderTiles() {
		float tile_step = (float)TERRAIN_SIZE / (float)NUM_DIVISION;
		ComputeShader compute_shader("Shaders/computeShader.cs");
		float heights[64*64];
		float winds[64 * 64*3];
		float grass_color[64 * 64*3];

		glBindTexture(GL_TEXTURE_2D, grass_heights_tex);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, heights);
		
		glBindTexture(GL_TEXTURE_2D, wind_texture);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, winds);
		
		glBindTexture(GL_TEXTURE_2D, grass_color_tex);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, grass_color);

		std::cout << "loading\n";
		for (int i = 0; i < 64; i++) {
			std::vector<RenderTile *> temp_vec;
			for (int j = 0; j < 64; j++) {
				//std::cout << i << ", " << j << ": " << heights[i * 64 + j] << "\n";
				glm::vec3 wind_normal = get_vec3_from_texture_array(i, j, winds);
				glm::vec3 tile_color = get_vec3_from_texture_array(i, j, grass_color);
			
				
				glm::vec4 tile_position = position;
				tile_position.x += j * tile_step;
				tile_position.y = position.y;
				tile_position.z += i * tile_step;
				std::vector<float> point_heights = {height_map[i * 65 + j],  height_map[(i * 65) + (j + 1)], height_map[(i + 1) * 65 + j], height_map[(i + 1) * 65 + (j + 1)] };
			

				RenderTile* temp_tile = new RenderTile(tile_position, point_heights, tile_step, heights[i * 64 + j], glm::normalize(wind_normal), tile_color, &compute_shader, grass_shader);
				temp_vec.push_back(temp_tile);

			}
			render_tiles.push_back(temp_vec);

		}

	}

	void mapVertices(float vertices[], float normals[], float tex_coord[]) {
		int vertex = 0;
		int normalsVertex = 0;
		int texVertex = 0;
		for (int i = 0; i <= NUM_DIVISION; i++) {
			for (int j = 0; j <= NUM_DIVISION; j++) {
				if ((j != NUM_DIVISION && i != NUM_DIVISION) || (j == NUM_DIVISION && i == NUM_DIVISION)) {
					glm::vec2 inputTex[] = {globalTexCoord[i][j] , globalTexCoord[i][j + 1], globalTexCoord[i + 1][j], globalTexCoord[i + 1][j + 1]};
					glm::vec3 inputPoints[] = { points[i][j], points[i][j + 1], points[i + 1][j], points[i + 1][j + 1] };
					mapSquare(vertices, normals, tex_coord, &vertex, &normalsVertex, &texVertex, inputPoints, inputTex, glm::vec3(1.0f, 1.0f, 1.0f));

				}

			}

		}
	}

	void setPoints() {

		float height_map[65 * 65];
		glBindTexture(GL_TEXTURE_2D, height_map_tex);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, height_map);
		

		float posX, posY, posZ;
		for (int i = 0; i <= NUM_DIVISION; i++) {
			for (int j = 0; j <= NUM_DIVISION; j++) {
				height_map[i * 65 + j] *= 1;
				//posX = (float)j / ((float)NUM_DIVISION - 1) * TERRAIN_SIZE;
				posX = j * 2;
				posY = height_map[i * 65 + j];
				this->height_map.push_back(height_map[i * 65 + j]);
				posZ = i * 2;
				points[j][i] = glm::vec3(posX, posY, posZ);

				glm::vec2 tempTexCoord;
				tempTexCoord.x = (float)j / ((float)NUM_DIVISION);
				tempTexCoord.y = (float)i / ((float)NUM_DIVISION);
				globalTexCoord[j][i] = tempTexCoord;
			}

		}

	}

	//todo create math class
	void mapSquare(float vertices[], float normals[], float tex[], int* vertex, int* normalsVertex, int* texVertex, glm::vec3 points[], glm::vec2 textureCoord[], glm::vec3 normalDir) {
		//first triangle
		vertexToElement(vertices, vertex, points[0]);
		vertexToElement(vertices, vertex, points[1]);
		vertexToElement(vertices, vertex, points[2]);

		glm::vec3 normalVec = calculateNormals(points[0], points[1], points[2]);
		normalVec *= normalDir;
		vertexToElement(normals, normalsVertex, normalVec);
		vertexToElement(normals, normalsVertex, normalVec);
		vertexToElement(normals, normalsVertex, normalVec);

		texToElement(tex, texVertex, textureCoord[0]);
		texToElement(tex, texVertex, textureCoord[1]);
		texToElement(tex, texVertex, textureCoord[2]);
		
		//second triangle
		vertexToElement(vertices, vertex, points[1]);
		vertexToElement(vertices, vertex, points[3]);
		vertexToElement(vertices, vertex, points[2]);

		normalVec = calculateNormals(points[1], points[3], points[2]);
		normalVec *= normalDir;
		vertexToElement(normals, normalsVertex, normalVec);
		vertexToElement(normals, normalsVertex, normalVec);
		vertexToElement(normals, normalsVertex, normalVec);
		
		texToElement(tex, texVertex, textureCoord[1]);
		texToElement(tex, texVertex, textureCoord[3]);
		texToElement(tex, texVertex, textureCoord[2]);
	}

	void vertexToElement(float vertices[], int* vertex, glm::vec3 vector) {
		vertices[*vertex] = vector.x;
		vertices[*vertex + 1] = vector.y;
		vertices[*vertex + 2] = vector.z;
		*vertex += 3;
	}
	
	void texToElement(float tex[], int* vertex, glm::vec2 vector) {
		tex[*vertex] = vector.x;
		tex[*vertex + 1] = vector.y;
		*vertex += 2;
	}

	glm::vec3 calculateNormals(glm::vec3 vectorA, glm::vec3 vectorB, glm::vec3 vectorC) {
		//(B-A)x(C-A)
		//
		return glm::cross(vectorB - vectorA, vectorC - vectorA);
	}

};

#endif