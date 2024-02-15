#include <stdio.h>
#include "GLFW/glfw3.h"

int main(int argv, char** argc){
    GLFWwindow* window;

    if(!glfwInit()){
        printf("Couldn't init GLFW)");
    }

    window=glfwCreateWindow(640,480,"Hello", NULL, NULL);
    if(!window){
        printf("Couldn't open window\n");
        return 1;
    }

    unsigned char* data = new unsigned char[100*100*3];
    int row_stride = 100*3;
    int idx = 0;
    for(int y=0;y<100;++y){
        for(int x=0;x<100;++x){
            idx = y*row_stride+x*3;
            data[idx] = 0xff;
            data[idx+1] = 0x00;
            data[idx+1] = 0x00;
        }
    }

    glfwMakeContextCurrent(window);

    GLuint tex_handle;
    glGenTextures(1,&tex_handle);
    glBindTexture(GL_TEXTURE_2D, tex_handle);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,100,100,0,GL_RGB,GL_UNSIGNED_BYTE,data);

    
    while(!glfwWindowShouldClose(window)){
        
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        int window_width, window_height;
        glfwGetFramebufferSize(window, &window_width, &window_height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0,window_width,0,window_height,-1,1);
        glMatrixMode(GL_MODELVIEW);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex_handle);
        glBegin(GL_QUADS);
            glTexCoord2d(0,0); glVertex2i(0,0);
            glTexCoord2d(1,0); glVertex2i(100,0);
            glTexCoord2d(1,1); glVertex2i(100,100);
            glTexCoord2d(0,1); glVertex2i(0,100);
            glEnd();
            glDisable(GL_TEXTURE_2D);
    


        glfwSwapBuffers(window);
        glfwWaitEvents();
    }
    printf("Hello\n");
    return 0;
}