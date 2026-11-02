#include <iostream>
#include "object.h"

void Object::update()
{
	if (!matrixDirty)
		return;

	modelMatrix = glm::mat4(1.0f);

	modelMatrix = glm::translate(modelMatrix, position);
	modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	modelMatrix = glm::scale(modelMatrix, scale);

	matrixDirty = false;
}

Object::Object(ObjData objData) : objData(objData), position(0.0f), rotation(0.0f), scale(1.0f)
{
}

void Mesh::createBuffers(std::vector<glm::vec3> vertices, std::vector<glm::vec3> uvs, std::vector<glm::vec3> normals, std::vector<glm::vec3> tangents, std::vector<glm::vec3> bitangents)
{
	vertexCount = static_cast<GLsizei>(vertices.size());

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &vertexVBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(glm::vec3),
        vertices.data(),
        GL_STATIC_DRAW
    );

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 0, nullptr
    );

    if (!uvs.empty())
    {
        glGenBuffers(1, &uvVBO);
        glBindBuffer(GL_ARRAY_BUFFER, uvVBO);

        glBufferData(
            GL_ARRAY_BUFFER,
            uvs.size() * sizeof(glm::vec2),
            uvs.data(),
            GL_STATIC_DRAW
        );

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1, 2, GL_FLOAT, GL_FALSE, 0, nullptr
        );
    }

    if (!normals.empty())
    {
        glGenBuffers(1, &normalVBO);
        glBindBuffer(GL_ARRAY_BUFFER, normalVBO);

        glBufferData(
            GL_ARRAY_BUFFER,
            normals.size() * sizeof(glm::vec3),
            normals.data(),
            GL_STATIC_DRAW
        );

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(
            2, 3, GL_FLOAT, GL_FALSE, 0, nullptr
        );
    }

    if (!tangents.empty())
    {
        glGenBuffers(1, &tangentVBO);
        glBindBuffer(GL_ARRAY_BUFFER, tangentVBO);

        glBufferData(
            GL_ARRAY_BUFFER,
            tangents.size() * sizeof(glm::vec3),
            tangents.data(),
            GL_STATIC_DRAW
        );

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(
            3, 3, GL_FLOAT, GL_FALSE, 0, nullptr
        );
    }

    if (!bitangents.empty())
    {
        glGenBuffers(1, &bitangentVBO);
        glBindBuffer(GL_ARRAY_BUFFER, bitangentVBO);

        glBufferData(
            GL_ARRAY_BUFFER,
            bitangents.size() * sizeof(glm::vec3),
            bitangents.data(),
            GL_STATIC_DRAW
        );

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(
            4, 3, GL_FLOAT, GL_FALSE, 0, nullptr
        );
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Mesh::draw() const
{
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, vertexCount);
	glBindVertexArray(0);
}

void generateMeshesFromObject(Object& object)
{
    ObjData& objData = object.objData;
    const std::vector<Material>& materials = objData.materials;

    size_t materialCount = std::max<size_t>(1, materials.size());

    std::vector<std::vector<glm::vec3>> flatVerticesPerMaterial(materialCount);
    std::vector<std::vector<glm::vec2>> flatUVsPerMaterial(materialCount);
    std::vector<std::vector<glm::vec3>> flatNormalsPerMaterial(materialCount);
    std::vector<std::vector<glm::vec3>> flatTangentsPerMaterial(materialCount);
    std::vector<std::vector<glm::vec3>> flatBitangentsPerMaterial(materialCount);

    // Per materiaal de vertices verzamelen
    for (const auto& face : objData.faces)
    {
        int materialIndex = face.materialIndex >= 0 ? face.materialIndex : 0;

        if (materialIndex >= static_cast<int>(materialCount))
            materialIndex = 0;

        for (size_t i = 0; i < face.vertexIndices.size(); ++i)
        {
            flatVerticesPerMaterial[materialIndex].push_back(objData.vertices[face.vertexIndices[i]]);

            if (!objData.textureCoords.empty() && !face.textureCoordIndices.empty())
                flatUVsPerMaterial[materialIndex].push_back(objData.textureCoords[face.textureCoordIndices[i]]);
            else
                flatUVsPerMaterial[materialIndex].push_back(glm::vec2{ 0.0f, 0.0f });

            if (!objData.normals.empty() && !face.normalIndices.empty())
                flatNormalsPerMaterial[materialIndex].push_back(objData.normals[face.normalIndices[i]]);
            else
                flatNormalsPerMaterial[materialIndex].push_back(glm::vec3{ 0.0f });

            flatTangentsPerMaterial[materialIndex].push_back(face.tangents[i]);
            flatBitangentsPerMaterial[materialIndex].push_back(face.bitangents[i]);
        }
    }

    // Per materiaal een Mesh aanmaken
    for (size_t m = 0; m < materialCount; ++m)
    {
        if (flatVerticesPerMaterial[m].empty())
            continue;

        Mesh mesh;

        mesh.vertexCount = static_cast<GLsizei>(flatVerticesPerMaterial[m].size());

        // VAO aanmaken
        glGenVertexArrays(1, &mesh.VAO);
        glBindVertexArray(mesh.VAO);

        // Vertex positions
        glGenBuffers(1, &mesh.vertexVBO);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexVBO);
        glBufferData(
            GL_ARRAY_BUFFER,
            flatVerticesPerMaterial[m].size() * sizeof(glm::vec3),
            flatVerticesPerMaterial[m].data(),
            GL_STATIC_DRAW
        );
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // UVs
        if (!flatUVsPerMaterial[m].empty())
        {
            glGenBuffers(1, &mesh.uvVBO);
            glBindBuffer(GL_ARRAY_BUFFER, mesh.uvVBO);
            glBufferData(
                GL_ARRAY_BUFFER,
                flatUVsPerMaterial[m].size() * sizeof(glm::vec2),
                flatUVsPerMaterial[m].data(),
                GL_STATIC_DRAW
            );
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        }

        // Normals
        if (!flatNormalsPerMaterial[m].empty())
        {
            glGenBuffers(1, &mesh.normalVBO);
            glBindBuffer(GL_ARRAY_BUFFER, mesh.normalVBO);
            glBufferData(
                GL_ARRAY_BUFFER,
                flatNormalsPerMaterial[m].size() * sizeof(glm::vec3),
                flatNormalsPerMaterial[m].data(),
                GL_STATIC_DRAW
            );
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        }

        // Tangents
        if (!flatTangentsPerMaterial[m].empty())
        {
            glGenBuffers(1, &mesh.tangentVBO);
            glBindBuffer(GL_ARRAY_BUFFER, mesh.tangentVBO);
            glBufferData(
                GL_ARRAY_BUFFER,
                flatTangentsPerMaterial[m].size() * sizeof(glm::vec3),
                flatTangentsPerMaterial[m].data(),
                GL_STATIC_DRAW
            );
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        }

        // Bitangents
        if (!flatBitangentsPerMaterial[m].empty())
        {
            glGenBuffers(1, &mesh.bitangentVBO);
            glBindBuffer(GL_ARRAY_BUFFER, mesh.bitangentVBO);
            glBufferData(
                GL_ARRAY_BUFFER,
                flatBitangentsPerMaterial[m].size() * sizeof(glm::vec3),
                flatBitangentsPerMaterial[m].data(),
                GL_STATIC_DRAW
            );
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        }

        // Material properties
        if (m < materials.size())
        {
            mesh.ka = materials[m].ambientColor;
            mesh.kd = materials[m].diffuseColor;
            mesh.ks = materials[m].specularColor;
            mesh.ns = materials[m].shininess > 0.0f ? materials[m].shininess : 32.0f;
        }
        else
        {
            mesh.ka = glm::vec3{ 1.0f };
            mesh.kd = glm::vec3{ 1.0f };
            mesh.ks = glm::vec3{ 1.0f };
            mesh.ns = 48.0f;
        }

        // Diffuse texture aanmaken
        glGenTextures(1, &mesh.diffuseTexture);
        glBindTexture(GL_TEXTURE_2D, mesh.diffuseTexture);

        if (m < materials.size() && !materials[m].diffuseTexture.isEmpty())
        {
            const Texture& tex = materials[m].diffuseTexture;

            glTexImage2D(
                GL_TEXTURE_2D, 0, GL_RGBA,
                tex.width, tex.height, 0,
                GL_RGBA, GL_UNSIGNED_BYTE,
                tex.pixels.data()
            );
        }
        else
        {
            unsigned char white[4] = { 255, 255, 255, 255 };

            glTexImage2D(
                GL_TEXTURE_2D, 0, GL_RGBA,
                1, 1, 0,
                GL_RGBA, GL_UNSIGNED_BYTE,
                white
            );
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // Heightmap texture aanmaken
        glGenTextures(1, &mesh.heightMapTexture);
        glBindTexture(GL_TEXTURE_2D, mesh.heightMapTexture);

        if (m < materials.size() && !materials[m].bumpTexture.isEmpty())
        {
            const Texture& tex = materials[m].bumpTexture;

            glTexImage2D(
                GL_TEXTURE_2D, 0, GL_RGBA,
                tex.width, tex.height, 0,
                GL_RGBA, GL_UNSIGNED_BYTE,
                tex.pixels.data()
            );
        }
        else
        {
            unsigned char white[4] = { 255, 255, 255, 255 };

            glTexImage2D(
                GL_TEXTURE_2D, 0, GL_RGBA,
                1, 1, 0,
                GL_RGBA, GL_UNSIGNED_BYTE,
                white
            );
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        std::cout << "Mesh aangemaakt met VAO id: "
            << mesh.VAO << "\n";

        object.meshes.push_back(std::move(mesh));
    }

    // VAO loskoppelen
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}