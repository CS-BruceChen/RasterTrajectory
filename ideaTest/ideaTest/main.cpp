#include "shaderAssistance.hpp"
#include "windowAssistance.hpp"
#include <cstdlib>
#include <ctime>

int main() {
	initOpenGL();
	GLFWwindow* window = createWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
	initWindowAndGlad(window);
	Shader* lineShader = newShader("tri");
    srand((int)time(0));
    Line* lineArray[100];
    unsigned len = 3;
    for (int j = 0; j < 100; ++j) {
        float* vert = new float[2 * 3];
        for (int i = 0; i < 6; ++i) {
            vert[i] = 2 * ((rand() % 100) / 100.0 - 0.5);
        }
        lineArray[j] = new Line(vert, len);
    }

    
    FBO* fbo = new FBO(SCR_WIDTH, SCR_HEIGHT, FBO::Attachment::NoAttachment, GL_TEXTURE_2D, GL_RGB);
    unsigned framebuffer = fbo->getFBO();
    unsigned textureColorbuffer = fbo->texture();
	
    Shader* sampleFBO_shader = newShader("sampleFBO");
    Shader* testPoly_shader = newShader("polygon");
    Quad quad;
    TestPoly testPoly;
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);


        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        lineShader->use();
        for (int i = 0; i < 100; ++i) {
            glBindVertexArray(lineArray[i]->lineVAO);
            glDrawArrays(GL_LINE_STRIP, 0, len);
            glBindVertexArray(0); // no need to unbind it every time 
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //sampleFBO_shader->use();
        //glBindVertexArray(quad.quadVAO);
        testPoly_shader->use();
        glBindVertexArray(testPoly.polyVAO);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);	// use the color attachment texture as the texture of the quad plane
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    //glDeleteVertexArrays(1, &VAO);
    //glDeleteBuffers(1, &VBO);
    glDeleteProgram(lineShader->ID);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();


	return 0;
}