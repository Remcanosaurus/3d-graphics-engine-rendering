// 3D graphics engine OpenGL variant

#define GLEW_STATIC
#include <string>
#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_image.h>
#include <algorithm>
#include <iostream>
#include <filesystem>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "shader_loader.cpp"
#include "obj_loader.h"
#include "controls.h"
#include "light.h"
#include "frustum_culling.h"
#include "object.h"

constexpr int kScreenWidth{ 640 };
constexpr int kScreenHeight{ 480 };

//Window
SDL_Window* gWindow{ nullptr };

SDL_GLContext glContext;

// Lichtbron
Light light{
    glm::vec3{ 3.0f, 1.0f, 3.0f }, // positie
    glm::vec3{ 0.5f, 0.5f, 0.5f }, // la
    glm::vec3{ 0.9f, 0.9f, 0.9f }, // ld
    glm::vec3{ 1.0f, 1.0f, 1.0f }  // ls
};

Frustum cameraFrustum;
float cameraNearPlane{ 0.1f };
float cameraFarPlane{ 100.0f };

std::vector<Object> objects;

bool init()
{
    bool success{ true };

    //SDL initialiseren
    if (SDL_Init(SDL_INIT_VIDEO) == false)
    {
        SDL_Log("SDL error: %s\n", SDL_GetError());
        success = false;
    }
    else
    {
        // OpenGL attributen definiëren
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3); // OpenGL versie
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); // Enkele buffer
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24); // Pixel buffer grootte

        //Window maken
        if (gWindow = SDL_CreateWindow("OpenGL", kScreenWidth, kScreenHeight, SDL_WINDOW_OPENGL); gWindow == nullptr)
        {
            SDL_Log("SDL error: %s\n", SDL_GetError());
            success = false;
        }
        else {
            // OpenGL context
            glContext = SDL_GL_CreateContext(gWindow);
            if (!glContext) {
                std::cerr << "OpenGL Context Error: " << SDL_GetError() << "\n";
                SDL_DestroyWindow(gWindow);
                SDL_Quit();
                return false;
            }

            // GLEW initialiseren
            glewExperimental = GL_TRUE;
            GLenum glewErrorCode = glewInit();
            if (GLEW_OK != glewErrorCode) {
                std::cerr << "GLEW init error: " << glewGetErrorString(glewErrorCode) << "\n";
            }

            // Z-buffer
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            // Culling
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        }
    }

    return success;
}

void close()
{
    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(gWindow);
    SDL_Quit();
}


int main(int argc, char* args[])
{
    int exitCode{ 0 };

    // Fix pad errors door absolute paths te gebruiken
    std::filesystem::path exePath = std::filesystem::absolute(args[0]);
    std::filesystem::path baseDir = exePath.parent_path();

    std::filesystem::path vertexPath = baseDir / "vertex_shader.vert";
    std::filesystem::path fragmentPath = baseDir / "fragment_shader.frag";

    std::string filePath; // Pad naar OBJ bestand uit command line argument halen
    if (argc > 1) filePath = args[1];
    else {
        filePath = baseDir.string() + "/cube.obj";
    }

    std::string fileName = std::filesystem::path(filePath).filename().string();
    std::cout << "OBJ bestand: " << fileName << "\n";
    //Initialiseren
    if (init() == false)
    {
        SDL_Log("Fout bij initialiseren.\n");
        exitCode = 1;
    }
    else
    {
        // Laad OBJ file
        ObjData objData = loadObjFile(filePath);
        calculateTangents(objData);

        for (int x = 0; x < 3; ++x)
		{
			for (int z = 0; z < 3; ++z)
			{
				Object object{ objData };
				generateMeshesFromObject(object);
				object.setPosition(glm::vec3{ static_cast<float>(x) * 4.0f, 0.0f, static_cast<float>(z) * 4.0f });
				objects.push_back(object);
			}
		}

        bool quit{ false };

        SDL_Event e;
        SDL_zero(e);

        glClearColor(0.5f, 0.5f, 0.9f, 1.0f);

        // Definieer de contanten uit de material
        std::vector<glm::vec3> kaVector;
        std::vector<glm::vec3> kdVector;
        std::vector<glm::vec3> ksVector;
        std::vector<float> nsVector; // shininess per material

        // Laad de shader in een shader program en sla de ID op.
        GLuint programID = LoadShaders(vertexPath.string().c_str(), fragmentPath.string().c_str());

        // Uniform locaties voor shader
        GLint modelLoc = glGetUniformLocation(programID, "model");
        GLint viewLoc = glGetUniformLocation(programID, "view");
        GLint projLoc = glGetUniformLocation(programID, "projection");
        GLint camPosLoc = glGetUniformLocation(programID, "camPos");
        GLint samplerLoc = glGetUniformLocation(programID, "textureSampler");
		GLint heightMapSamplerLoc = glGetUniformLocation(programID, "heightMapSampler");

        GLint kaLoc = glGetUniformLocation(programID, "ka");
        GLint kdLoc = glGetUniformLocation(programID, "kd");
        GLint ksLoc = glGetUniformLocation(programID, "ks");
        GLint nsLoc = glGetUniformLocation(programID, "ns");

        GLint lposLoc = glGetUniformLocation(programID, "lpos");
        GLint laLoc = glGetUniformLocation(programID, "la");
        GLint ldLoc = glGetUniformLocation(programID, "ld");
        GLint lsLoc = glGetUniformLocation(programID, "ls");

        //Main loop
        while (quit == false)
        {
            while (SDL_PollEvent(&e) == true)
            {
                if (e.type == SDL_EVENT_QUIT)
                {
                    //Quit als quit event
                    quit = true;
                }

                if (e.type == SDL_EVENT_KEY_DOWN)
                {
                    //Quit als escape
                    if (e.key.key == SDLK_ESCAPE)
                    {
                        quit = true;
                    }
                }
            }

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Draw calls hier
            glUseProgram(programID);

            // Bereken matrices van input
            computeMatricesFromInputs((float)kScreenWidth / kScreenHeight, cameraNearPlane, cameraFarPlane);
            glm::mat4 ProjectionMatrix = getProjectionMatrix();
            glm::mat4 ViewMatrix = getViewMatrix();

            // Krijg cam pos
            glm::vec3 camPos = getCameraPosition();

            cameraFrustum = calculateCameraFrustum(camPos, getCameraFront(), getCameraRight(), getCameraUp(), kScreenWidth / kScreenHeight, getFOV(), cameraNearPlane, cameraFarPlane);

            // Upload camera position
            if (camPosLoc != -1) glUniform3fv(camPosLoc, 1, glm::value_ptr(camPos));

            // Upload licht uniforms
            if (lposLoc != -1) glUniform3fv(lposLoc, 1, glm::value_ptr(light.light_position));
            if (laLoc != -1) glUniform3fv(laLoc, 1, glm::value_ptr(light.la));
            if (ldLoc != -1) glUniform3fv(ldLoc, 1, glm::value_ptr(light.ld));
            if (lsLoc != -1) glUniform3fv(lsLoc, 1, glm::value_ptr(light.ls));

            // Upload matrices naar shader
            if (projLoc != -1) glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(ProjectionMatrix));
            if (viewLoc != -1) glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(ViewMatrix));

            for (Object& object : objects)
            {
                object.update();
                glm::mat4 ModelMatrix = object.getModelMatrix();
                AABB objectBounds = calculateWorldAABB(object.objData.vertices, ModelMatrix);

                if (!cameraFrustum.isInsideFrustum(objectBounds))
                    continue;

                // Upload model matrix naar shader
                if (modelLoc != -1) glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(ModelMatrix));

                for (const Mesh& mesh : object.meshes)
                {
					glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, mesh.diffuseTexture);

					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, mesh.heightMapTexture);

                    if (kaLoc != -1)
                        glUniform3fv(kaLoc, 1, glm::value_ptr(mesh.ka));

                    if (kdLoc != -1)
                        glUniform3fv(kdLoc, 1, glm::value_ptr(mesh.kd));

                    if (ksLoc != -1)
                        glUniform3fv(ksLoc, 1, glm::value_ptr(mesh.ks));

                    if (nsLoc != -1)
                        glUniform1f(nsLoc, mesh.ns);
					
                    mesh.draw();
                }
            }

            // Swap buffer naar scherm
            SDL_GL_SwapWindow(gWindow);

            light.light_position = camPos;
        }
    }

    //Clean up
    close();

    std::cout << "Druk op een toets om af te sluiten.\n";
    system("pause");
    return exitCode;
}