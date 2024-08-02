#ifndef RAW_MODEL_H
#define RAW_MODEL_H


class RawModel {

public:

	unsigned int vaoID;
	unsigned int vertexCount;

	RawModel(int vaoID, int vertexCount) {
		this->vaoID = vaoID;
		this->vertexCount = vertexCount;
	}
};

class Loader {

public:

	RawModel loadToVAO(float vertices[], float normals[], int numVertices) {
		unsigned int VAO = createVAO();
		storeDataInAttributeList(0, numVertices, 3, vertices);
		storeDataInAttributeList(1, numVertices, 3, normals);
		return RawModel(VAO, numVertices / 3 * sizeof(float));

	}
	
	RawModel loadToVAOTexture(float vertices[], float normals[], float texture_coord[], int numVertices, int numTex) {
		unsigned int VAO = createVAO();
		storeDataInAttributeList(0, numVertices, 3, vertices);
		storeDataInAttributeList(1, numVertices, 3, normals);
		storeDataInTexCoordList(2, numTex, 2, texture_coord);
		return RawModel(VAO, numVertices / 3 * sizeof(float));

	}

	//RawModel loadToEBO(float vertices[], float normals[], float texture_coord[], int numVertices, int numTex)

	unsigned int load_texture(const char* texture_name) {

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


	int createVAO() {
		unsigned int VAO;
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		return VAO;
	}

	void storeDataInAttributeList(int attributeNumber, int count, int verticesInAttribute, float vertices[]) {
		unsigned int VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, count, vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(attributeNumber, verticesInAttribute, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(attributeNumber);
	}
	
	void storeDataInTexCoordList(int attributeNumber, int count, int verticesInAttribute, float vertices[]) {
		unsigned int VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, count, vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(attributeNumber, verticesInAttribute, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(attributeNumber);
	}
	
	void bindIndicesBuffer(int indices[], int num_indices) {
		unsigned int EBO;
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices, indices, GL_STATIC_DRAW);
	}

};


class Renderer {

public:

	void prepare() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void render(RawModel model, GLenum type) {
		glBindVertexArray(model.vaoID);
		glDrawArrays(type, 0, model.vertexCount);
		//glDrawArrays(type, 0, 9*6*3);
		glBindVertexArray(model.vaoID);
	}
};

#endif
