#include <glut.h>
#include <queue>
#include "glsupport.h"
#include "geometrymaker.h"
#include "matrix4.h"
#include "quat.h"
#include "Geometry.h"
#include "Entity.h"
#include "Transform.h"
#include "Material.h"
#include "Camera.h"
#include "Scene.h"
#include "Util.h"
#include "Cube.h"
#include "Plane.h"
#include "Skybox.h"
#include "Model.h"
#include "Sphere.h"
#include "Light.h"
#include "Shader.h"
#include "ColorShader.h"
#include "ModelShader.h"
#include "TextureShader.h"
#include "ScreenShader.h"
#include "ShadowShader.h"
#include "Screen.h"
#include "FrameBufferObject.h"
int screenWidth = 600;
int screenHeight = 600;

ColorShader* colorShader;
ModelShader* modelShader;
TextureShader* textureShader;
TextureShader* cubemapShader;
TextureShader* refractShader;
TextureShader* reflectShader;
ScreenShader* screenShader;
ScreenShader* grayShader;
ScreenShader* colorInvertShader;
ScreenShader* fxaaShader;
ScreenShader* toneMappingShader;
ScreenShader* hdrFxaaShader;

ScreenShader* horizontalBlurShader;
ScreenShader* verticalBlurShader;

//ShadowShader* shadowShader;

ScreenShader* currentEffectShader = NULL;//record current effect for the whole scene (post-processing)


Camera camera(Cvec3(0, 0, 0), Quat::makeXRotation(0));

std::string currentModel = "model0";

Light* currentMovingLight;

Entity* screen = NULL;//for post-processing

FrameBufferObject* firstFBO;
FrameBufferObject* secondFBO;


class BtnEventListener : public ClickEventListener {

public:
    BtnEventListener() {}
    void onClick(Entity* button) {
        button->setPosition(button->initState.transform.getPosition() + Cvec3(0, 0, -0.2));
    }
    void onHover(Entity* button) {
        button->setPosition(button->initState.transform.getPosition());
        button->setScale(button->initState.transform.getScale() * 1.1);
        button->material->setColor(button->initState.color + Cvec3f(0.2, 0, 0));
    }
    void onIdle(Entity* button) {
        button->setPosition(button->initState.transform.getPosition());
        button->setScale(button->initState.transform.getScale());
        button->material->setColor(button->initState.color);
    }
};

class PostProcessingSwitchListener : public BtnEventListener {
private:
    int effectSelected = 0;
public:
    void onClick(Entity* button) {
        BtnEventListener::onClick(button);
        effectSelected = (effectSelected + 1) % 6;
        switch(effectSelected) {
            case 0:
            {
                screen->setProgram(screenShader->getProgramId());
                currentEffectShader = screenShader;
                break;
            }
            case 1:
            {
                screen->setProgram(grayShader->getProgramId());
                currentEffectShader = grayShader;
                break;
            }
            case 2:
            {
                screen->setProgram(colorInvertShader->getProgramId());
                currentEffectShader = colorInvertShader;
                break;
            }
            case 3:
            {
                screen->setProgram(fxaaShader->getProgramId());
                currentEffectShader = fxaaShader;
                break;
            }
            case 4:
            {
                screen->setProgram(toneMappingShader->getProgramId());
                currentEffectShader = toneMappingShader;
                break;
            }
            case 5:
            {
                screen->setProgram(hdrFxaaShader->getProgramId());
                currentEffectShader = hdrFxaaShader;
                break;
            }
            default:
                throw std::string("no matched model selected");
        }
    }
    void onHover(Entity* button) {
        BtnEventListener::onHover(button);
    }
    void onIdle(Entity* button) {
        BtnEventListener::onIdle(button);
    }
};

class ModelSwitchBtnEventListener : public BtnEventListener {
private:
    int modelSelected = 0;
public:
    void onClick(Entity* button) {
        BtnEventListener::onClick(button);
        modelSelected = (modelSelected + 1) % 4;
        switch(modelSelected) {
            case 0:
            {
                Entity* monk = Scene::getEntity("model0");
                monk->setProgram(refractShader->getProgramId());
                break;
            }
            case 1:
            {
                Entity* monk = Scene::getEntity("model0");
                monk->setProgram(reflectShader->getProgramId());
                break;
            }
            case 2:
            {
                Entity* monk = Scene::getEntity("model0");
                monk->setProgram(modelShader->getProgramId());
                break;
            }
            case 3:
            {
                Entity* monk = Scene::getEntity("model0");
                monk->setProgram(colorShader->getProgramId());
                break;
            }

//            case 0:
//            {
//                Entity* monk = Scene::getEntity("model0");
//                Entity* spiderman = Scene::getEntity("model1");
//                monk->setVisible(true);
//                spiderman->setVisible(false);
//                currentModel = "model0";
//                break;
//            }
//            case 1:
//            {
//                Entity* monk = Scene::getEntity("model0");
//                Entity* spiderman = Scene::getEntity("model1");
//                monk->setVisible(false);
//                spiderman->setVisible(true);
//                currentModel = "model1";
//                break;
//            }
            default:
                throw std::string("no matched model selected");
        }
    }
    void onHover(Entity* button) {
        BtnEventListener::onHover(button);
    }
    void onIdle(Entity* button) {
        BtnEventListener::onIdle(button);
    }
};

class LightColorBtnEventListener : public BtnEventListener {
private:
    int lightColor0 = 0;
    int lightColor1 = 0;
public:
    void onClick(Entity* button) {
        BtnEventListener::onClick(button);
        
        if (button->getName() == "button1") {
            lightColor0 = (lightColor0 + 1) % 2;
            if (lightColor0 == 0) {
                currentMovingLight = Scene::getLight(0);
                Scene::light0->lightColor = Cvec3f(1, 1, 1);
                button->initState.color = Cvec3f(0.8, 0, 0);
            } else {
                currentMovingLight = NULL;
                Scene::light0->lightColor = Cvec3f(0, 0, 0);
                button->initState.color = Cvec3f(0.1, 0, 0);
            }
        } else {
            lightColor1 = (lightColor1 + 1) % 2;
            if (lightColor1 == 0) {
                currentMovingLight = Scene::getLight(1);
                Scene::light1->lightColor = Cvec3f(1, 1, 1);
                button->initState.color = Cvec3f(0.8, 0, 0);
            } else {
                currentMovingLight = NULL;
                Scene::light1->lightColor = Cvec3f(0, 0, 0);
                button->initState.color = Cvec3f(0.1, 0, 0);
            }
        }
        
    }
    void onHover(Entity* button) {
        BtnEventListener::onHover(button);
    }
    void onIdle(Entity* button) {
        BtnEventListener::onIdle(button);
    }
};

class SpecularLightColorBtnEventListener : public BtnEventListener {
private:
    bool spcularLightColorOn0 = false;
    bool spcularLightColorOn1 = false;
public:
    void onClick(Entity* button) {
        BtnEventListener::onClick(button);
        
        if (button->getName() == "button3") {
            spcularLightColorOn0 = !spcularLightColorOn0;
            if(spcularLightColorOn0) {
                Scene::light0->specularLightColor = Cvec3f(0, 0, 0);
                button->material->setColor(0.1, 0, 0);
                button->initState.color = Cvec3f(0.1, 0, 0);
            } else {
                Scene::light0->specularLightColor = Cvec3f(1, 1, 1);
                button->material->setColor(0.6, 0, 0);
                button->initState.color = Cvec3f(0.6, 0, 0);
            }
        } else {
            spcularLightColorOn1 = !spcularLightColorOn1;
            if(spcularLightColorOn1) {
                Scene::light1->specularLightColor = Cvec3f(0, 0, 0);
                button->material->setColor(0.1, 0, 0);
                button->initState.color = Cvec3f(0.1, 0, 0);
            } else {
                Scene::light1->specularLightColor = Cvec3f(1, 1, 1);
                button->material->setColor(0.6, 0, 0);
                button->initState.color = Cvec3f(0.6, 0, 0);
            }
        }
    }
    void onHover(Entity* button) {
        BtnEventListener::onHover(button);
    }
    void onIdle(Entity* button) {
        BtnEventListener::onIdle(button);
    }
};



void display(void) {
//    Scene::render();//render the scene directly to scren
    //render to texture and then to screen using one fbo


    //blur effect rendering using 2 fbos
    float time = (float) glutGet(GLUT_ELAPSED_TIME) / 1000.0f;//second passed
    float blurSize = 0.01f - 0.001f * time;
//    std::cout << blurSize << std::endl;
    if (blurSize < -1e-8) {
        screen->material->setDiffuseTextureId(firstFBO->getFrameBufferTexture());
        screen->setProgram(screenShader->getProgramId());
        Scene::renderToTexture();
        Scene::renderToScreen(currentEffectShader, screenWidth, screenHeight);
    } else {
        horizontalBlurShader->setBlurSize(blurSize);
        verticalBlurShader->setBlurSize(blurSize);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindFramebuffer(GL_FRAMEBUFFER, firstFBO->getFrameBuffer());
        glViewport(0, 0, 1024, 1024);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Scene::renderLoop();//render the scene to the firstFBO

        glBindFramebuffer(GL_FRAMEBUFFER, secondFBO->getFrameBuffer());
        glViewport(0, 0, 1024, 1024);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (screen != NULL) {
            screen->material->setDiffuseTextureId(firstFBO->getFrameBufferTexture());
            screen->setProgram(horizontalBlurShader->getProgramId());
            screen->draw(Scene::camera, horizontalBlurShader, Scene::light0, Scene::light1);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, screenWidth, screenHeight);
        if (screen != NULL) {
            screen->material->setDiffuseTextureId(secondFBO->getFrameBufferTexture());
            screen->setProgram(verticalBlurShader->getProgramId());
            screen->draw(Scene::camera, verticalBlurShader, Scene::light0, Scene::light1);
        }
        
        glutSwapBuffers();
    }

}

void init() {
    glClearColor(0.2, 0.2, 0.2, 1.0);
    glClearDepth(0.0);
    glCullFace(GL_BACK);
//    glEnable(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);
    glReadBuffer(GL_BACK);
    
    colorShader = new ColorShader();
    colorShader->createProgram("shaders/vertex_shader_simple.glsl", "shaders/fragment_shader_color.glsl");
    
    modelShader = new ModelShader();
    modelShader->createProgram("shaders/vertex_shader_model.glsl", "shaders/fragment_shader_model.glsl");
    
    textureShader = new TextureShader();
    textureShader->createProgram("shaders/vertex_shader_simple.glsl", "shaders/fragment_shader_texture.glsl");

    cubemapShader = new TextureShader();
    cubemapShader->createProgram("shaders/vertex_shader_cubemap.glsl", "shaders/fragment_shader_cubemap.glsl");
    
    reflectShader = new TextureShader();
    reflectShader->createProgram("shaders/vertex_shader_simple.glsl", "shaders/fragment_shader_environment_reflect.glsl");
    
    refractShader = new TextureShader();
    refractShader->createProgram("shaders/vertex_shader_simple.glsl", "shaders/fragment_shader_environment_refract.glsl");
    
    screenShader = new ScreenShader();
    screenShader->createProgram("shaders/vertex_shader_offscreen.glsl", "shaders/fragment_shader_offscreen.glsl");

    grayShader = new ScreenShader();
    grayShader->createProgram("shaders/vertex_shader_offscreen.glsl", "shaders/fragment_shader_postprocessing_blackwhite.glsl");

    colorInvertShader = new ScreenShader();
    colorInvertShader->createProgram("shaders/vertex_shader_offscreen.glsl", "shaders/fragment_shader_postprocessing_colorinvert.glsl");

    fxaaShader = new ScreenShader();
    fxaaShader->createProgram("shaders/vertex_shader_offscreen.glsl", "shaders/fragment_shader_postprocessing_fxaa.glsl");

    toneMappingShader = new ScreenShader();
    toneMappingShader->createProgram("shaders/vertex_shader_offscreen.glsl", "shaders/fragment_shader_postprocessing_tone_mapping.glsl");

    hdrFxaaShader = new ScreenShader();
    hdrFxaaShader->createProgram("shaders/vertex_shader_offscreen.glsl", "shaders/fragment_shader_postprocessing_tonemapping_fxaa.glsl");

    horizontalBlurShader = new ScreenShader();
    horizontalBlurShader->createProgram("shaders/vertex_shader_offscreen.glsl", "shaders/fragment_shader_postprocessing_horizontal_blur.glsl");

    verticalBlurShader = new ScreenShader();
    verticalBlurShader->createProgram("shaders/vertex_shader_offscreen.glsl", "shaders/fragment_shader_postprocessing_vertical_blur.glsl");

//    shadowShader = new ShadowShader();
//    shadowShader->createProgram("shaders/vertex_shader_shadow.glsl", "shaders/fragment_shader_shadow.glsl");

    currentEffectShader = screenShader;//default effect


    Scene::addShader(colorShader);
    Scene::addShader(modelShader);
    Scene::addShader(textureShader);
    Scene::addShader(cubemapShader);
    Scene::addShader(reflectShader);
    Scene::addShader(refractShader);

    Scene::setCamera(&camera);
    Light* light0 = new Light();
    light0->setPosition(1, 5, -5);//1,5,-5
    Light* light1 = new Light();
    light1->setPosition(-1, 0, -4);//0,0,-6  , 1,5,0
    Scene::setLight0(light0);
    Scene::setLight1(light1);

    currentMovingLight = Scene::getLight(0);

    
    Model* model0 = new Model("Monk_Giveaway_Fixed.obj", "model0");
    model0->setScale(Cvec3(0.5, 0.5, 0.5));
    model0->setPosition(Cvec3(0, -3.4, -9));
    model0->setRotation(Quat::makeYRotation(20));
    model0->material->setColor(0.0, 0.8, 0.8);
    model0->setProgram(refractShader->getProgramId());
    Scene::addChild(model0);


//    Model* model2 = new Model("Monk_Giveaway_Fixed.obj", "model0");
//    model2->setScale(Cvec3(0.5, 0.5, 0.5));
//    model2->setPosition(Cvec3(2, -6, -8));
//    model2->setRotation(Quat::makeYRotation(-20));
//    model2->material->setColor(0.0, 0.8, 0.8);
//    model2->setProgram(refractShader->getProgramId());
//    Scene::addChild(model2);
//
//    Model* model1 = new Model("Spiderman.obj", "model1");
//    model1->setScale(Cvec3(1.8, 1.8, 1.8));
//    model1->setPosition(Cvec3(-2, -6, -8));
//    model1->setRotation(Quat::makeYRotation(20));
//    model1->setProgram(refractShader->getProgramId());
//    Scene::addChild(model1);

//    Model* model1 = new Model("Spiderman.obj", "model1");
//    model1->setScale(Cvec3(1.3, 1.3, 1.3));
//    model1->setPosition(Cvec3(0, -2.3, -7));
//    model1->setRotation(Quat::makeYRotation(20));
//    model1->setVisible(false);
//    model1->setProgram(reflectShader->getProgramId());
//    Scene::addChild(model1);

    
    /************ model swtich button ************/
    Geometry* buttonG = new Sphere(2, 40, 40);
    Material* buttonM = new Material(Cvec3f(0.6, 0.0, 0.0));
    Entity* btn0 = new Entity("button0", buttonG, buttonM);
    btn0->setPosition(Cvec3(-1.8, 1.9, -5));
    btn0->setScale(Cvec3(0.05, 0.05, 0.05));
    btn0->registerClickEventListener(new ModelSwitchBtnEventListener());
    btn0->setProgram(colorShader->getProgramId());
    Scene::addChild(btn0);

    /************ post-processing effect swtich button ************/
    Entity* btn1 = new Entity("button1", buttonG, buttonM);
    btn1->setPosition(Cvec3(-1.5, 1.9, -5));
    btn1->setScale(Cvec3(0.05, 0.05, 0.05));
    btn1->registerClickEventListener(new PostProcessingSwitchListener());
    btn1->setProgram(colorShader->getProgramId());
    Scene::addChild(btn1);
    
    /************ light color buttons ************/
    for (int i = 0; i < 2; ++i) {
        Material* buttonM = new Material(Cvec3f(0.6, 0.0, 0.0));
        Entity* btn = new Entity("button" + std::to_string(i + 1), buttonG, buttonM);
        btn->setPosition(Cvec3(-1.8, 1.9 - (i + 1) / 3.0, -5));
        btn->setScale(Cvec3(0.05, 0.05, 0.05));
        btn->registerClickEventListener(new LightColorBtnEventListener());
        btn->setProgram(colorShader->getProgramId());
        Scene::addChild(btn);
    }
    
    /************ specular light color buttons ************/
    for (int i = 0; i < 2; ++i) {
        Material* buttonM = new Material(Cvec3f(0.6, 0.0, 0.0));
        Entity* btn = new Entity("button" + std::to_string(i + 3), buttonG, buttonM);
        btn->setPosition(Cvec3(-1.5, 1.9 - (i + 1) / 3.0, -5));
        btn->setScale(Cvec3(0.05, 0.05, 0.05));
        btn->registerClickEventListener(new SpecularLightColorBtnEventListener());
        btn->setProgram(colorShader->getProgramId());
        Scene::addChild(btn);
    }



    Cubemap cubemap;
//    cubemap.loadTextures("cubemap/snow2.jpeg", "cubemap/snow2.jpeg", "cubemap/snow2.jpeg", "cubemap/snow2.jpeg", "cubemap/snow2.jpeg", "cubemap/snow2.jpeg");
    cubemap.loadTextures("cubemap/snow.PNG", "cubemap/snow.PNG", "cubemap/snow.PNG", "cubemap/snow.PNG", "cubemap/snow.PNG", "cubemap/snow.PNG");
//        cubemap.loadTextures("cubemap/building.png", "cubemap/building.png", "cubemap/building.png", "cubemap/building.png", "cubemap/building.png", "cubemap/building.png");
//    cubemap.loadTextures("cubemap/posx.jpg", "cubemap/negx.jpg", "cubemap/posy.jpg", "cubemap/negy.jpg", "cubemap/posz.jpg", "cubemap/negz.jpg");
    Material* cubemapM = new Material();
    cubemapM->setCubemap(cubemap.getTexture());
    Cube* sb = new Cube(100);
    Entity* skybox = new Entity(sb, cubemapM);
    skybox->setProgram(cubemapShader->getProgramId());
    Scene::addChild(skybox);



    firstFBO = new FrameBufferObject(1024, 1024, true);
    Scene::setFrameBufferObject(firstFBO);


    secondFBO = new FrameBufferObject();


    Screen* plane = new Screen();
    Material* planeM = new Material();
    planeM->setDiffuseTextureId(firstFBO->getFrameBufferTexture());
    screen = new Entity(plane, planeM);
    screen->setProgram(screenShader->getProgramId());
    Scene::setScreen(screen);
//    screen->setPosition(Cvec3(0, 0, -10));
//    screen->setRotation(Quat::makeXRotation(90) * Quat::makeYRotation(180) * Quat::makeZRotation(30));
//    screen->rejectAllLights();





    Scene::createMeshes();//this call genereate vbo/ibo for the geometry of each Entity.
}
bool stereo = false;
int screenshot(void)
{

    int width = screenWidth;
    int height = screenHeight;
    
    int i,j;
    FILE *fptr;
    static int counter = 0; /* This supports animation sequences */
    char fname[32];
    unsigned char *image = (unsigned char*) malloc(3 * width * height * sizeof(unsigned char));
    /* Allocate our buffer for the image */
    if (image == NULL) {
        fprintf(stderr,"Failed to allocate memory for image\n");
        return 0;
    }

    glPixelStorei(GL_PACK_ALIGNMENT,1);

    /* Open the file */
    if (stereo)
        sprintf(fname,"L_%04d.jpg",counter);
    else
        sprintf(fname,"C_%04d.jpg",counter);
    if ((fptr = fopen(fname,"w")) == NULL) {
        fprintf(stderr,"Failed to open file for window dump\n");
        return 0;
    }

    /* Copy the image into our buffer */
    glReadBuffer(GL_BACK_LEFT);
    glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,image);

    /* Write the raw file */
    /* fprintf(fptr,"P6\n%d %d\n255\n",width,height); for ppm */
    for (j=height-1;j>=0;j--) {
        for (i=0;i<width;i++) {
            fputc(image[3*j*width+3*i+0],fptr);
            fputc(image[3*j*width+3*i+1],fptr);
            fputc(image[3*j*width+3*i+2],fptr);
        }
    }
    fclose(fptr);

    if (stereo) {
        /* Open the file */
        sprintf(fname,"R_%04d.jpg",counter);
        if ((fptr = fopen(fname,"w")) == NULL) {
            fprintf(stderr,"Failed to open file for window dump\n");
            return 0;
        }

        /* Copy the image into our buffer */
        glReadBuffer(GL_BACK_RIGHT);
        glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,image);

        /* Write the raw file */
        /* fprintf(fptr,"P6\n%d %d\n255\n",width,height); for ppm */
        for (j=height-1;j>=0;j--) {
            for (i=0;i<width;i++) {
                fputc(image[3*j*width+3*i+0],fptr);
                fputc(image[3*j*width+3*i+1],fptr);
                fputc(image[3*j*width+3*i+2],fptr);
            }
        }
        fclose(fptr);
    }
    
    /* Clean up */
    counter++;
    free(image);
    return 1;
}

//control light position in x and z coordinates
void specialInput(int key, int x, int y) {
    if (currentMovingLight == NULL) {
        return;
    }
    switch (key) {
        case GLUT_KEY_RIGHT:
        {
            currentMovingLight->moveX(1);
            break;
        }
        case GLUT_KEY_DOWN:
        {
            currentMovingLight->moveZ(1);
            break;
        }
        case GLUT_KEY_LEFT:
        {
            currentMovingLight->moveX(-1);
            break;
        }
        case GLUT_KEY_UP:
        {
            currentMovingLight->moveZ(-1);
            break;
        }
        default:
            break;
    }
    std::cout << "light pos: " << currentMovingLight->getPosition()[0] << "," << currentMovingLight->getPosition()[1] << "," << currentMovingLight->getPosition()[2]<< std::endl;
}


void keyboard(unsigned char key, int x, int y) {

    switch(key) {
        case 'l':
        case 'L':
            camera.rotate(Quat::makeYRotation(10));
            break;
        case 'k':
        case 'K':
            camera.rotate(Quat::makeYRotation(-10));
            break;
        case 's':
        case 'S':
        {
            Entity* model = Scene::getEntity(currentModel);
            model->setScale(model->getScale() * 1.1);
            break;
        }
        case 'd':
        case 'D':
        {
            Entity* model = Scene::getEntity(currentModel);
            model->setScale(model->getScale() / 1.1);
            break;
        }
        case 'G':
        case 'g':
        {
            if (currentMovingLight != NULL) {
                currentMovingLight->moveY(1);
            }
            break;
        }
        case 'b':
        case 'B':
        {
            if (currentMovingLight != NULL) {
                currentMovingLight->moveY(-1);
            }
            break;
        }
        case 'p':
        case'P':
            screenshot();
            break;
        default:
            break;
    }
}


Cvec2 getScreenSpaceCoordFromEyeSpace(const Cvec3& eyeSpaceCoord, const Matrix4& projection, int screenW, int screenH) {
    if (eyeSpaceCoord[2] > -1e-8) {
        throw string("z > 0");
    }
    Cvec4 p = projection * Cvec4(eyeSpaceCoord, 1);
    Cvec3 clippedCoord = Cvec3(p[0], p[1], p[2]) / p[3];
    return Cvec2(clippedCoord[0] * screenW / 2.0 + (screenW - 1) / 2.0, clippedCoord[1] * screenH / 2.0 + (screenH - 1) / 2.0);
}

Cvec2 getScreenSpaceCoordFromWorldSpace(const Matrix4& modelMatrix, const Matrix4& projection, int screenW, int screenH) {
    Cvec4 eyeSpaceCoord = inv(camera.getViewMatrix()) * modelMatrix * Cvec4(0, 0, 0, 1);
    return getScreenSpaceCoordFromEyeSpace(Cvec3(eyeSpaceCoord), projection, screenW, screenH);
}

double getScreenToEyeScale(double z, double fovy, int screenHeight) {
    if (z > -1e-8) {
        throw string("z > 0");
    }
    return -(z * tan(fovy * CS175_PI / 360.0)) * 2 / screenHeight;
}

Cvec3 getPickedPointOnSphereInScreenSpace(Entity* entity, int x, int y) {
    Cvec2 centerInScreenSpace = getScreenSpaceCoordFromWorldSpace(entity->getModelMatrix(), camera.getProjectionMatrix(), screenWidth, screenHeight);
    
    double r = screenWidth / 2;
    double cr = r / getScreenToEyeScale(entity->getPosition()[2], camera.getFov(), screenHeight);//here assume RBT in eye space.

    double x_cx = x - centerInScreenSpace[0];
    double y_cy = y - centerInScreenSpace[1];
    
    double pickedZ2 = cr * cr - x_cx * x_cx - y_cy * y_cy;
    pickedZ2 = pickedZ2 < 0 ? 0 : pickedZ2;
    double pickedZ = sqrt(pickedZ2);

    return Cvec3(x, y, pickedZ);
}

Cvec3 prev_pos;
Cvec3 cur_pos;
bool rotating = false;
void mouse(int button, int state, int x, int y) {
    Scene::updateMouseEvent(button, state, x, y, screenWidth, screenHeight);
    
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        Entity* model = Scene::getEntity(currentModel);
        if (model == NULL) {
            std::cerr << "model not found in the scene" << std::endl;
            return;
        }
        
        cur_pos = getPickedPointOnSphereInScreenSpace(model, x, screenHeight - y);
        prev_pos = cur_pos;
        rotating = true;
    } else {
        rotating = false;
    }
    
}

void motion(int x, int y) {
    if (!rotating) {
        return;
    }

    Entity* model = Scene::getEntity(currentModel);
    if (model == NULL) {
        std::cerr << "model not found in the scene" << std::endl;
        return;
    }

    cur_pos = getPickedPointOnSphereInScreenSpace(model, x, screenHeight - y);
    
    Cvec2 center = getScreenSpaceCoordFromWorldSpace(model->getModelMatrix(), camera.getProjectionMatrix(), screenWidth, screenHeight);
    Cvec3 c(center, 0);
    
    Cvec3 v1 = normalize(prev_pos - c);
    Cvec3 v2 = normalize(cur_pos - c);
    
    Quat q1(0, v2);
    Quat q2(0, -v1);
    
    Quat q = q1 * q2;
//    Quat q(dot(v1, v2), cross(v1, v2));
    model->rotate(q);
}

bool insideWindow(int x, int y) {
    return (x >= 0 && y >= 0 && x <= screenWidth && y <= screenHeight);
}
void passiveMotion(int x, int y) {
    if (insideWindow(x, y)) {
        Scene::updateMousePassiveMotion(x, y, screenWidth, screenHeight);
    }
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    screenWidth = w;
    screenHeight = h;
}

void idle(void) {
    glutPostRedisplay();
}


int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);// | GLUT_MULTISAMPLE);
    glutInitWindowSize(screenWidth, screenHeight);
    glutCreateWindow("CS-6533");
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);

    glutSpecialFunc(specialInput);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutPassiveMotionFunc(passiveMotion);
    
//    glEnable(GLUT_MULTISAMPLE);
    
    init();
    glutMainLoop();
    return 0;
}

