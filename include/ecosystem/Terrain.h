#ifndef TERRAIN_H
#define TERRAIN_H

#include <ecosystem/RenderTileTree.h>
#include <ecosystem/render_tile.h>

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
	unsigned int height_map_tex;


	RenderTileTree *render_tile_tree;

	Terrain(Shader* shader, Shader* grass_shader, ComputeShader *clipping_shader, glm::vec4 position, const char* grass_heights_name, const char* terrain_height_name, bool has_grass = true) {
		this->shader = shader;
		this->grass_shader = grass_shader;
		this->position = position;
		this->has_grass = has_grass;
		this->model = glm::transpose(glm::translate(glm::mat4(1.0f), position.xyz()));
		
		load_textures(grass_heights_name, terrain_height_name);
		createTerrainMesh();
		if (has_grass) render_tile_tree = new RenderTileTree(grass_shader, clipping_shader, model, 128, 64, position, height_map);

	}

	~Terrain() {
		for (int i = 0; i < render_tiles.size(); i++) {
			for (RenderTile* renderTile : render_tiles[i]) {
				delete renderTile;
			}
		}
		delete render_tile_tree;
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

		if (has_grass) render_tile_tree->draw(time, proj, view, view_pos, frustum);
	}



	
private:
	const int TERRAIN_SIZE = 128;
	const int SQUARE_SIZE = 18;
	const int NUM_DIVISION = 64;
	glm::vec3 points[65][65];
	glm::vec2 globalTexCoord[65][65];
	Loader loader;

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
		terrain_mesh.push_back(temp_rawModel);

	}

	void load_textures(const char* grass_heights_name, const char* terrain_heights_name) {
		texture = loader.load_texture("Textures/grass.jpg");
		height_map_tex = loader.load_texture(terrain_heights_name);
	}
		
	glm::vec3 get_vec3_from_texture_array(int i, int j, float tex_array[]) {
		float winds[65 * 65*3];
		glm::vec3 rtn_vec;
		rtn_vec.x = tex_array[(i * 64 + j) * 3];
		rtn_vec.y = tex_array[(i * 64 + j) * 3 + 1];
		rtn_vec.z = tex_array[(i * 64 + j) * 3 + 2];

		return rtn_vec;
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
		return glm::cross(vectorB - vectorA, vectorC - vectorA);
	}

};

#endif