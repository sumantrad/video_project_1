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
    while(!glfwWindowShouldClose(window)){
        
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glDrawPixels(100,100,GL_RGB,GL_UNSIGNED_BYTE,data);

        glfwSwapBuffers(window);
        glfwWaitEvents();
    }
    printf("Hello\n");
    return 0;
}