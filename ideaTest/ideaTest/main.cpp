#include "shaderAssistance.hpp"
#include "windowAssistance.hpp"
#include <cstdlib>
#include <ctime>

const unsigned trajN = 100;//trajectory number
const unsigned LEN_RANGE = 2;//trajectory lenth range: [2,2+LEN_RANGE-1]
const unsigned POINT_DIMENSION = 3;//x,y,id
const unsigned MAX_POLY_NUM = 1;//the number of polygons is 1 
const unsigned MAX_EDGE_RANGE = 10;//each polygon will have at most 3+MAX_EDGE_RANGE-1 edges;

int main() {
	initOpenGL();
	GLFWwindow* window = createWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
	initWindowAndGlad(window);

    double init_time = glfwGetTime();

	Shader* lineShader = newShader("tri");
    srand((int)time(0));
    Line* lineArray[trajN];
    unsigned lineLen[trajN];
    for (int j = 0; j < trajN; ++j) {
        unsigned len = 2 + rand() % LEN_RANGE;
        float* vert = new float[len * POINT_DIMENSION];
        for (int i = 0; i < len; ++i) {
            vert[i * POINT_DIMENSION + 0] = 2 * ((rand() % 100) / 100.0 - 0.5);//x
            vert[i * POINT_DIMENSION + 1] = 2 * ((rand() % 100) / 100.0 - 0.5);//y
            vert[i * POINT_DIMENSION + 2] = (float)j;//id
        }
        lineArray[j] = new Line(vert, len);
        lineLen[j] = len;
    }

    std::vector<TPolygon> polys;
    unsigned TOTAL_POLY_VERT_NUM = 0;
    for (int i = 0; i < MAX_POLY_NUM; ++i) {
        TPolygon temp_poly;
        unsigned edgeNum = 3 + (rand() % MAX_EDGE_RANGE);//随机的多边形边数目
        TOTAL_POLY_VERT_NUM += edgeNum;
        for (int j = 0; j < edgeNum; ++j) {
            c2t::Point p;//random polygon vertex
            p.x = 2 * ((rand() % 100) / 100.0 - 0.5);
            p.y = 2 * ((rand() % 100) / 100.0 - 0.5);
            temp_poly.push_back(p);
        }
        polys.push_back(temp_poly);
    }
    std::vector<float> verts,ids;
    triangulatePolygons(polys, verts, ids);
    
    FBO* fbo = new FBO(SCR_WIDTH, SCR_HEIGHT, FBO::Attachment::NoAttachment, GL_TEXTURE_2D, GL_RGB);
    unsigned framebuffer = fbo->getFBO();
    unsigned textureColorbuffer = fbo->texture();
	
    Shader* sampleFBO_shader = newShader("sampleFBO");
    Shader* testPoly_shader = newShader("polygon");
    Quad quad;
    TestPoly testPoly;
    Test_Poly_Tri tpt(verts.size() * sizeof(float), verts.data());

    GLTextureBuffer texBuf;
    std::vector<int> resultData;
    texBuf.create(trajN * sizeof(int), GL_R32I, resultData.data());
    glBindImageTexture(0, texBuf.texId, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32I);

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
        for (int i = 0; i < trajN; ++i) {
            glBindVertexArray(lineArray[i]->lineVAO);
            glDrawArrays(GL_LINE_STRIP, 0, lineLen[i]);
            glBindVertexArray(0); // no need to unbind it every time 
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //sampleFBO_shader->use();
        //glBindVertexArray(quad.quadVAO);
        testPoly_shader->use();
        //glBindVertexArray(testPoly.polyVAO);
        glBindVertexArray(tpt.tptVAO);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);	// use the color attachment texture as the texture of the quad plane
        //glDrawArrays(GL_TRIANGLES, 0, 6);
        glDrawArrays(GL_TRIANGLES, 0, TOTAL_POLY_VERT_NUM);
        glBindVertexArray(0);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    resultData = texBuf.getBuffer();
    texBuf.destroy();
    unsigned count = 0;
    for (int i = 0; i < trajN; ++i) {
        if (resultData[i] > 0) {
            ++count;
            std::cout << "Trajectory [" << i << "] overlap the polygon" << std::endl;
        }
    }
    std::cout << "There are a total of "<< trajN <<" trajectories, "<< count <<" of which are within the polygon" << std::endl;
    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    //glDeleteVertexArrays(1, &VAO);
    //glDeleteBuffers(1, &VBO);
    double dt = glfwGetTime() - init_time;
    std::cout << "total time is: "<< dt <<std::endl;
    glDeleteProgram(lineShader->ID);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();


	return 0;
}