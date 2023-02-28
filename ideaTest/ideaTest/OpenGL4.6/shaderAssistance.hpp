#pragma once
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <omp.h>
#include <clipper/clipper.hpp>
#include <poly2tri/poly2tri.h>
#include <clip2tri/clip2tri.h>


const unsigned SCR_WIDTH = 800;
const unsigned SCR_HEIGHT = 600;

//declaration
class ImageTexture {
public:
    GLuint textureID;
    unsigned width;
    unsigned height;
public:
    ImageTexture();
    ImageTexture(unsigned w, unsigned h);
public:
    void generateTexture();
    void generateImageTexture();
    void transfer2Texture(float* data);
public:
    static float* getTextureData(GLuint width, GLuint height, GLuint channels, GLuint texID);
};

//考虑写成基类，这样可以针对不同的着色器定制特定的初始化方法和数据操作方法。
//当然，基类的构造方法不能被继承
class Shader
{
public:
    GLuint ID;//shader program ID
    Shader(std::vector<std::string> filePaths);
    void use();
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;   
    void setVec2(const std::string& name, float x, float y) const;
    void setVec3(const std::string& name, float x, float y, float z) const;    
    void setVec4(const std::string& name, float x, float y, float z, float w) const;
    void setVec4(const std::string& name, int x, int y, int z, int w) const;

private:
    std::string getShaderSourceCode(const std::string& filePath);
    void compileAndLinkShader(std::string& shaderString, char shaderType);
    void checkErrors(GLuint id, std::string type);//id means shader id or program id
};

struct DATA_LINE {//part of the whole line we read, containing only the required attribute data 
    std::string GlobalID;
    unsigned N_Point;
    unsigned IdexPoint;
    unsigned Timestamp;
    float PositionX;
    float PositionY;
};

const unsigned POSITION_DIMENSION = 2;
const unsigned TIME_DIMENSION = 1;
const unsigned ATTRIBUTE_NUM = 6;
const std::string REQUIRED_ATTRIBUTE[] = { "GlobalID","N_Point","IdxPoint","Timestamp","PositionX","PositionY" };
//构造函数测试通过，剩余需要测试着色器传输函数。
class Trajectories
{
public:
    float MAXX;
    float MAXY;
    float MINX;
    float MINY;

public:
    Trajectories(const char* filePath);
    ~Trajectories();
    void dataTransferToShader(unsigned posVarLocation);
    void dataTransferToShader(unsigned posVarLocation, unsigned timeVarLocation);
    void drawTrajectories(Shader* shader,GLenum drawMode);//drawmode可以改成内置的枚举类型以进一步限定
    void showResult() {
        std::cout << "MAXX: " << MAXX << std::endl;
        std::cout << "MINX: " << MINX << std::endl;
        std::cout << "MAXY: " << MAXY << std::endl;
        std::cout << "MINY: " << MINY << std::endl;
        std::cout << "N: " << N << std::endl;
        std::cout << "pointNum:" << std::endl;
        for (size_t i = 0; i < N; ++i) {
            std::cout << pointNum[i] << " ";
        }
        std::cout << "Trajectory:" << std::endl;
        for (size_t i = 0; i < N; ++i) {
            std::cout << "point of trajectory " << i << " :" << std::endl;
            for (size_t j = 0; j < pointNum[i]; ++j) {
                std::cout << "(" << position[i][POSITION_DIMENSION * j + 0] << "," << position[i][POSITION_DIMENSION * j + 1] << "," << time[i][TIME_DIMENSION * j + 0] << ")" << std::endl;
            }
        }
    }

public:
    unsigned getN() const { return N; }
    std::vector<unsigned> getPointNum() const { return pointNum; }
    std::vector<float*> getPosition() const { return position; };
    std::vector<unsigned*> getTime() const { return time; }
private:
    unsigned N;//number of trajectory
    unsigned* VAOs;//VAO for each trajectory
    std::vector<unsigned> pointNum;//point number of each trajectory
    std::vector<float*> position;
    std::vector<unsigned*> time;

private:
    std::vector<std::string> getColumnNameArray(std::ifstream& readFile);
    void getDataLine(std::string readLine, DATA_LINE& data_line, std::vector<std::string>& columnNames);
    void initTrajectoriesVariable(std::ifstream& readFile, std::vector<std::string>& columnNames);//init the variable that descript these trajectories. (N, pointNum, position, time)
};

class FBO
{
public:
    enum Attachment {
        NoAttachment,
        CombinedDepthStencil,
        Depth
    };
    FBO(int width, int height, Attachment attachment, GLenum target, GLenum internal_format) {
        tex = 0; fbo = 0; depth = 0;
        this->wd = width;
        this->ht = height;
        this->target = target;
        if (attachment == CombinedDepthStencil) {
            std::cout << "attachment not yet implemented" <<std::endl;
            return;
        }
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glGenTextures(1, &tex);
        glBindTexture(target, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(target, 0, internal_format, width, height, 0, internal_format, GL_FLOAT, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

        if (attachment == Depth) {
            glGenRenderbuffers(1, &depth);
            glBindRenderbuffer(GL_RENDERBUFFER, depth);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        }

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "Error in FBO creation!!!!!!!!!!!!!!!!!!!!!!" <<std::endl;
        }
        bindDefault();
    }
    ~FBO() {
        if (fbo != 0) {
            glDeleteFramebuffers(1, &fbo);
            fbo = 0;
        }
        if (tex != 0) {
            glDeleteTextures(1, &tex);
            tex = 0;
        }
        if (depth != 0) {
            glDeleteRenderbuffers(1, &depth);
            depth = 0;
        }
    }

    static void bindDefault() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

public:
    int getFBO() const { return fbo; }
    int width() const { return wd; }
    int height() const { return ht; }
    GLuint texture() const { return tex; }
    void bind() { glBindFramebuffer(GL_FRAMEBUFFER, this->fbo); }

private:
    int wd, ht;
    GLuint tex;
    GLuint fbo;
    GLuint depth;
    GLenum target;
};

const unsigned VERTEX_DIMENSION = 2;
class Polygon {
public:
    Polygon(std::vector<std::vector<float>> vertices) {
        vertexNum = vertices.size();
        vertexPosition = new float[vertexNum * VERTEX_DIMENSION];
        for (size_t i = 0; i < vertices.size(); ++i) {
            vertexPosition[i * VERTEX_DIMENSION + 0] = vertices[i][0];
            vertexPosition[i * VERTEX_DIMENSION + 1] = vertices[i][1];
        }
    }

    void drawPolygon(Shader* shader,unsigned posVarLocation, GLenum drawMode) {
        unsigned VAO;
        glGenVertexArrays(1, &VAO);
        unsigned VBO;
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, VERTEX_DIMENSION * vertexNum * sizeof(float), vertexPosition, GL_STATIC_DRAW);
        glVertexAttribPointer(posVarLocation, VERTEX_DIMENSION, GL_FLOAT, GL_FALSE, VERTEX_DIMENSION * sizeof(float), (void*)0);//GL_FALSE,for that we don't normalize data here. We do it ourselves.
        glEnableVertexAttribArray(0);//attri pointer 0

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        shader->use();
        glDrawArrays(drawMode, 0, vertexNum);

        glBindVertexArray(0);
    }

    ~Polygon() {
        delete[] vertexPosition;
    }
private:
    float* vertexPosition;
    unsigned vertexNum;
};

struct Quad {
    unsigned quadVAO;
    unsigned quadVBO;
    Quad() {
        float quadVertices[] = {
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

};

struct TestPoly {
    unsigned polyVAO;
    unsigned polyVBO;
    TestPoly() {
        float polyVertices[] = {
            -0.5,-0.5,
            0,0.5,
            0.5,-0.5
        };
        glGenVertexArrays(1, &polyVAO);
        glGenBuffers(1, &polyVBO);
        glBindVertexArray(polyVAO);
        glBindBuffer(GL_ARRAY_BUFFER, polyVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(polyVertices), &polyVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
};

struct Test_Poly_Tri {
    unsigned tptVAO;
    unsigned tptVBO;
    Test_Poly_Tri(unsigned size, void* data) {
        glGenVertexArrays(1, &tptVAO);
        glGenBuffers(1, &tptVBO);
        glBindVertexArray(tptVAO);
        glBindBuffer(GL_ARRAY_BUFFER, tptVBO);
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
};


struct Line {
    unsigned lineVAO;
    unsigned lineVBO;
    Line(float* lineVert,unsigned len) {
        glGenVertexArrays(1, &lineVAO);
        glGenBuffers(1, &lineVBO);
        glBindVertexArray(lineVAO);
        glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
        
        glBufferData(GL_ARRAY_BUFFER, 3 * len * sizeof(float), lineVert, GL_STATIC_DRAW);//x, y, id
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(2 * sizeof(float)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
};

struct GLTextureBuffer {
    GLTextureBuffer() : texId(0), bufId(0) {}
    void create(int size, GLenum format, void* data) {
        this->size = size;
        GLenum err;

        if (bufId > 0)
            glDeleteBuffers(1, &bufId);  //delete previously created tbo

        glGenBuffers(1, &bufId);

        glBindBuffer(GL_TEXTURE_BUFFER, bufId);
        glBufferData(GL_TEXTURE_BUFFER, size, data, GL_DYNAMIC_DRAW);

        err = glGetError();
        if (err > 0) {
            std::cout << "createTextureBuffer error 1: " << err << std::endl;
        }

        if (texId > 0)
            glDeleteTextures(1, &texId); //delete previously created texture

        glGenTextures(1, &texId);
        glBindTexture(GL_TEXTURE_BUFFER, texId);
        glTexBuffer(GL_TEXTURE_BUFFER, format, bufId);
        glBindBuffer(GL_TEXTURE_BUFFER, 0);

        err = glGetError();
        if (err > 0) {
            std::cout << "createTextureBuffer error 2: " << err << std::endl;
        }
    }
    void setData(int size, GLenum format, void* data) {
        this->size = size;
        GLenum err;

        if (bufId <= 0) {
            std::cout << "buffer not created!!";
            return;
        }

        glBindBuffer(GL_TEXTURE_BUFFER, bufId);
        glBufferData(GL_TEXTURE_BUFFER, size, data, GL_DYNAMIC_DRAW);

        err = glGetError();
        if (err > 0) {
            std::cout << "set data TextureBuffer error 1: "  << err <<std::endl;
        }

        if (texId <= 0) {
            std::cout << "texture buffer not created!!";
            return;
        }

        glBindTexture(GL_TEXTURE_BUFFER, texId);
        glTexBuffer(GL_TEXTURE_BUFFER, format, bufId);
        glBindBuffer(GL_TEXTURE_BUFFER, 0);
        glBindTexture(GL_TEXTURE_BUFFER, 0);

        err = glGetError();
        if (err > 0) {
            std::cout << "set data TextureBuffer error 2: " << err << std::endl;
        }
    }
    std::vector<int> getBuffer() {
        std::vector<int> data(size / sizeof(int));
        glBindBuffer(GL_TEXTURE_BUFFER, bufId);
        GLenum err = glGetError();
        glGetBufferSubData(GL_TEXTURE_BUFFER, 0, size, data.data());
        err = glGetError();
        glBindBuffer(GL_TEXTURE_BUFFER, 0);

        err = glGetError();
        if (err > 0) {
            std::cout << "getBuffer error: " << err << std::endl;
        }
        return data;
    }
    std::vector<float> getBufferF() {
        std::vector<float> data(size / sizeof(float));
        glBindBuffer(GL_TEXTURE_BUFFER, bufId);
        GLenum err = glGetError();
        glGetBufferSubData(GL_TEXTURE_BUFFER, 0, size, data.data());
        err = glGetError();
        glBindBuffer(GL_TEXTURE_BUFFER, 0);

        err = glGetError();
        if (err > 0) {
            std::cout << "getBuffer error: " << err << std::endl;
        }
        return data;
    }
    void destroy() {
        if (bufId > 0)
            glDeleteBuffers(1, &bufId);  //delete previously created tbo

        if (texId > 0)
            glDeleteTextures(1, &texId); //delete previously created texture

        bufId = 0;
        texId = 0;
    };

    int size;
    GLuint texId, bufId;
};


Shader* newShader(std::string name1) {
    std::vector<std::string> shader;
    std::string vs = "./" + name1 + ".vs.glsl";
    std::string fs = "./" + name1 + ".fs.glsl";
    shader.push_back(vs);
    shader.push_back(fs);
    return new Shader(shader);
}

//implemetation

/*
* we suppose that the csv file is ordered by GLobalID, then IdxPoint;
*/
Trajectories::Trajectories(const char* filePath) {
    //TODO: 
    //1.read data from filePath
    //2.initalize variable 'pointNum', 'position', 'time' and 'N' 
    std::ifstream trajectoriesFile(filePath);
    std::vector<std::string> columnNames = getColumnNameArray(trajectoriesFile);
    initTrajectoriesVariable(trajectoriesFile, columnNames);
}

Trajectories::~Trajectories() {
    //delete every heap mem in position array and time array;
    for (unsigned i = 0; i < N; ++i) {
        delete[] position[i];
        delete[] time[i];
    }
    delete[] VAOs;

}

//varlocation can be obtained from outside shader program
void Trajectories::dataTransferToShader(unsigned posVarLocation) {
    VAOs = new unsigned[N];
    glGenVertexArrays(N, VAOs);
    for (unsigned i = 0; i < N; ++i) {
        unsigned posVBO;
        glGenBuffers(1, &posVBO);

        glBindVertexArray(VAOs[i]);

        glBindBuffer(GL_ARRAY_BUFFER, posVBO);
        glBufferData(GL_ARRAY_BUFFER, POSITION_DIMENSION * pointNum[i] * sizeof(float), position[i], GL_STATIC_DRAW);
        glVertexAttribPointer(posVarLocation, POSITION_DIMENSION, GL_FLOAT, GL_FALSE, POSITION_DIMENSION * sizeof(float), (void*)0);//GL_FALSE,for that we don't normalize data here. We do it ourselves.
        glEnableVertexAttribArray(0);//attri pointer 0

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}

void Trajectories::dataTransferToShader(unsigned posVarLocation,unsigned timeVarLocation) {
    VAOs = new unsigned[N];
    glGenVertexArrays(N, VAOs);
    for (unsigned i = 0; i < N; ++i) {
        unsigned posVBO, timeVBO;
        glGenBuffers(1, &posVBO);
        glGenBuffers(1, &timeVBO);
        
        glBindVertexArray(VAOs[i]);
        
        glBindBuffer(GL_ARRAY_BUFFER, posVBO);
        glBufferData(GL_ARRAY_BUFFER, POSITION_DIMENSION * pointNum[i] * sizeof(float), position[i], GL_STATIC_DRAW);
        glVertexAttribPointer(posVarLocation, POSITION_DIMENSION, GL_FLOAT, GL_FALSE, POSITION_DIMENSION * sizeof(float), (void*)0);//GL_FALSE,for that we don't normalize data here. We do it ourselves.
        glEnableVertexAttribArray(0);//attri pointer 0

        glBindBuffer(GL_ARRAY_BUFFER, timeVBO);
        glBufferData(GL_ARRAY_BUFFER, TIME_DIMENSION * pointNum[i] * sizeof(unsigned), time[i], GL_STATIC_DRAW);
        glVertexAttribPointer(timeVarLocation, TIME_DIMENSION, GL_UNSIGNED_INT, GL_FALSE, TIME_DIMENSION * sizeof(unsigned), (void*)0);//GL_FALSE,for that we don't normalize data here. We do it ourselves.
        glEnableVertexAttribArray(1);//attri pointer 1

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}

void Trajectories::drawTrajectories(Shader* shader,GLenum drawMode) {
    //transfer normalization fractor
    shader->setFloat("MAXX", MAXX);
    shader->setFloat("MINX", MINX);
    shader->setFloat("MAXY", MAXY);
    shader->setFloat("MINY", MINY);
    shader->use();
    glClearColor(0.5f, 0.2f, 0.0f, 0.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
    glClear(GL_COLOR_BUFFER_BIT);
    for (unsigned i = 0; i < N; ++i) {
        glBindVertexArray(VAOs[i]);
        glDrawArrays(drawMode, 0, pointNum[i]);
        glBindVertexArray(0);
    }
    
}

std::vector<std::string> Trajectories::getColumnNameArray(std::ifstream& readFile) {
    std::string line;
    getline(readFile, line);
    std::vector<std::string> columnNames;
    std::string name;
    std::istringstream readNameStr(line);
    while (getline(readNameStr, name, ',')) {
        columnNames.push_back(name);
    }
    return columnNames;
}

void Trajectories::getDataLine(std::string readLine, DATA_LINE& data_line, std::vector<std::string>& columnNames) {
    std::string attributeValue;
    std::istringstream readstr(readLine);
    for (size_t i = 0; i < columnNames.size(); ++i) {
        getline(readstr, attributeValue, ',');
        for (size_t j = 0; j < ATTRIBUTE_NUM; ++j) {
            if (columnNames[i] == REQUIRED_ATTRIBUTE[j]) {//if the columnName is just the attribute we required 
                if (columnNames[i] == "GlobalID") {
                    data_line.GlobalID = attributeValue;
                }
                else if (columnNames[i] == "N_Point") {
                    data_line.N_Point = (unsigned)std::stoi(attributeValue);
                }
                else if (columnNames[i] == "IdxPoint") {
                    data_line.IdexPoint = (unsigned)std::stoi(attributeValue);
                }
                else if (columnNames[i] == "Timestamp") {
                    long long a = std::stoll(attributeValue);
                    data_line.Timestamp = (unsigned)(a / 1000);
                    
                }
                else if (columnNames[i] == "PositionX") {
                    data_line.PositionX = std::stof(attributeValue);
                }
                else if (columnNames[i] == "PositionY") {
                    data_line.PositionY = std::stof(attributeValue);
                }
            }
        }
    }
}

void Trajectories::initTrajectoriesVariable(std::ifstream& readFile, std::vector<std::string>& columnNames) {
    /*const unsigned TOTAL_ROW = 10575242;
    unsigned rowCount = 0;*/
    
    float maxX = FLT_MIN, maxY = FLT_MIN, minX = FLT_MAX, minY = FLT_MAX;
    
    std::string previous_GLobalID = "";
    unsigned previous_N_Point = 0;
    bool isFromTheSameTrajectory = true;
    unsigned trajectoryCount = 0;
    float* positionArray = NULL;
    unsigned* timeArray = NULL;

    std::string readLine;
    while (getline(readFile, readLine)) {
        /*rowCount++;
        float processPercent = rowCount * 100.0f / TOTAL_ROW;
        printf("文件读取总进度：%.2lf%%\r", processPercent);*/
        
        DATA_LINE data_line;
        
        //put data from readLine into data_line struct
        getDataLine(readLine, data_line, columnNames);

        //get the range of trajectory
        if (data_line.PositionX > maxX) maxX = data_line.PositionX;
        if (data_line.PositionX < minX) minX = data_line.PositionX;
        if (data_line.PositionY > maxY) maxY = data_line.PositionY;
        if (data_line.PositionY < minY) minY = data_line.PositionY;

        //process the data_line and put data from data_line into every variable
        if (data_line.GlobalID != previous_GLobalID) {//the first time meet a different trajectory
            isFromTheSameTrajectory = false;
            ++trajectoryCount;
        }
        else {
            isFromTheSameTrajectory = true;
        }

        if (!isFromTheSameTrajectory) {//the first time meet a different trajectory
            positionArray = new float[POSITION_DIMENSION * data_line.N_Point];
            timeArray = new unsigned[data_line.N_Point];
            pointNum.push_back(data_line.N_Point);
            position.push_back(positionArray);
            time.push_back(timeArray);
        }
        positionArray[POSITION_DIMENSION * data_line.IdexPoint + 0] = data_line.PositionX;
        positionArray[POSITION_DIMENSION * data_line.IdexPoint + 1] = data_line.PositionY;
        timeArray[TIME_DIMENSION * data_line.IdexPoint + 0] = data_line.Timestamp;
        //update the previous variable
        previous_GLobalID = data_line.GlobalID;
        previous_N_Point = data_line.N_Point;
    }

    N = trajectoryCount;
    MAXX = maxX;
    MINX = minX;
    MAXY = maxY;
    MINY = minY;
}

Shader::Shader(std::vector<std::string> filePaths) {
    ID = glCreateProgram();
    for (int i = 0; i < (int)filePaths.size(); ++i) {
        int len = filePaths[i].size();
        char shaderType = filePaths[i][len - 7];//[name].[type char]s.glsl,
        //std::cout << filePaths[i] << std::endl;
        //std::cout << shaderType << std::endl;
        std::string shaderString = getShaderSourceCode(filePaths[i]);
        compileAndLinkShader(shaderString, shaderType);

    }
}

void Shader::use() {
    glUseProgram(ID);
}

void Shader::setBool(const std::string& name, bool value) const{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
// ------------------------------------------------------------------------
void Shader::setInt(const std::string& name, int value) const{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
// ------------------------------------------------------------------------
void Shader::setFloat(const std::string& name, float value) const{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
// ------------------------------------------------------------------------

void Shader::setVec2(const std::string& name, float x, float y) const{
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}
// ------------------------------------------------------------------------

void Shader::setVec3(const std::string& name, float x, float y, float z) const{
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}
// ------------------------------------------------------------------------

void Shader::setVec4(const std::string& name, float x, float y, float z, float w) const{
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}

void Shader::setVec4(const std::string& name, int x, int y, int z, int w) const {
    glUniform4i(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}

std::string Shader::getShaderSourceCode(const std::string& filePath) {
    std::string computeShaderString;
    std::ifstream computeShaderFile;
    computeShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        computeShaderFile.open(filePath);
        std::stringstream computeShaderStream;
        computeShaderStream << computeShaderFile.rdbuf();
        computeShaderFile.close();
        computeShaderString = computeShaderStream.str();
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
    }

    return computeShaderString;
}

void Shader::compileAndLinkShader(std::string& shaderString ,char shaderType) {
    const char* shaderCode = shaderString.c_str();
    GLuint shaderID = 0;
    std::string checkErroType = "";


    if (shaderType == 'v') {//vertex shader
        shaderID = glCreateShader(GL_VERTEX_SHADER);
        checkErroType = "VERTEX";
    }
    else if (shaderType == 'f') {//fragment shader
        shaderID = glCreateShader(GL_FRAGMENT_SHADER);
        checkErroType = "FRAGMENT";
    }
    else if (shaderType == 'c') {//compute shader
        shaderID = glCreateShader(GL_COMPUTE_SHADER);
        checkErroType = "COMPUTE";
    }
    else if (shaderType == 'g') {//geometery shader
        shaderID = glCreateShader(GL_GEOMETRY_SHADER);
        checkErroType = "GEOMETRY";
    }

    //compile shader
    glShaderSource(shaderID, 1, &shaderCode, NULL);
    glCompileShader(shaderID);
    checkErrors(shaderID, checkErroType);
    
    //link shader
    glAttachShader(ID, shaderID);
    glLinkProgram(ID);
    checkErrors(ID, "PROGRAM");

    //delete shader
    glDeleteShader(shaderID);
}

void Shader::checkErrors(GLuint id, std::string type) {
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(id, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(id, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}



ImageTexture::ImageTexture() {
    this->width = SCR_WIDTH;
    this->height = SCR_HEIGHT;
}

ImageTexture::ImageTexture(unsigned w, unsigned h) {
	this->width = w;
	this->height = h;
}

void ImageTexture::generateTexture() {
    // generate texture
    glGenTextures(1, &this->textureID);
    glBindTexture(GL_TEXTURE_2D, this->textureID);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, this->width, this->height);
    // turn off filtering and wrap modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ImageTexture::generateImageTexture() {
    glBindImageTexture(0, this->textureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
}

void ImageTexture::transfer2Texture(float* data) {
    glBindTexture(GL_TEXTURE_2D, this->textureID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->width, this->height, GL_RGBA, GL_FLOAT, data);
}

float* ImageTexture::getTextureData(GLuint width, GLuint height, GLuint channels, GLuint texID) {
    float* data = new float[width * height * channels];
    glBindTexture(GL_TEXTURE_2D, texID);
    if (channels == 1)    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, data);
    if (channels == 3) glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, data);
    if (channels == 4) glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return data;
}


typedef std::vector<c2t::Point> TPolygon;

void triangulatePolygons(std::vector<TPolygon>& polys, std::vector<float>& verts, std::vector<float>& ids) {
    verts.clear();
    ids.clear();
    int mts = omp_get_max_threads();
    std::cout << "max threads:" << mts <<std::endl;
    std::vector<std::vector<float> > tverts(mts), tids(mts);

#pragma omp parallel for
    for (int i = 0; i < polys.size(); i++) {
        int id = omp_get_thread_num();
        vector<TPolygon > inputPolygons;
        TPolygon outputTriangles;  // Every 3 points is a triangle
        TPolygon boundingPolygon;
        inputPolygons.push_back(polys[i]);
        c2t::clip2tri clip2tri;
        clip2tri.triangulate(inputPolygons, outputTriangles, boundingPolygon);
        for (int j = 0; j < outputTriangles.size(); j++) {
            float x = float(outputTriangles[j].x);
            float y = float(outputTriangles[j].y);
            tverts[id].push_back(x);
            tverts[id].push_back(y);
            tids[id].push_back(i);
        }
    }
    for (int i = 0; i < mts; i++) {
        verts.insert(verts.end(), tverts[i].begin(), tverts[i].end());
        ids.insert(ids.end(), tids[i].begin(), tids[i].end());
    }
}

