#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>
#include "video_reader.hpp"

int main(int argc, const char** argv) {
    GLFWwindow* window;

    if (!glfwInit()) {
        printf("Couldn't init GLFW\n");
        return 1;
    }

    //VideoReaderState vr_state;
    VideoReaderState vr_state;
    vr_state.sws_scaler_ctx = NULL;
    if (!video_reader_open(&vr_state, "/home/sumantra/Videos/Webcam/test.webm")){
        printf("Couldn't open video file (make sure you set a video file that exists)\n");
        return 1;
    }

    window = glfwCreateWindow(vr_state.width, vr_state.height, "Hello World", NULL, NULL);
    if (!window) {
        printf("Couldn't open window\n");
        return 1;
    }

    glfwMakeContextCurrent(window);

    // Generate texture
    GLuint tex_handle;
    glGenTextures(1, &tex_handle);
    glBindTexture(GL_TEXTURE_2D, tex_handle);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // Allocate frame buffer
    constexpr int ALIGNMENT = 128;
    const int frame_width = vr_state.width;
    const int frame_height = vr_state.height;
    uint8_t* frame_data;
    if (posix_memalign((void**)&frame_data, ALIGNMENT, frame_width * frame_height * 4) != 0) {
        printf("Couldn't allocate frame buffer\n");
        return 1;
    }
    
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set up orphographic projection
        int window_width, window_height;
        glfwGetWindowSize(window, &window_width, &window_height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, window_width, window_height, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);

        // Read a new frame and load it into texture
        int64_t pts;
        if (!video_reader_read_frame(&vr_state, frame_data, &pts)) {
            printf("Couldn't load video frame\n");
            return 1;
        }

        static bool first_frame = true;
        if (first_frame) {
            glfwSetTime(0.0);
            first_frame = false;
        }

        double pt_in_seconds = pts * (double)vr_state.time_base.num / (double)vr_state.time_base.den;
        while (pt_in_seconds > glfwGetTime()) {
            glfwWaitEventsTimeout(pt_in_seconds - glfwGetTime());
        }

        glBindTexture(GL_TEXTURE_2D, tex_handle);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_width, frame_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, frame_data);

        // Render whatever you want
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex_handle);
        glBegin(GL_QUADS);
            glTexCoord2d(0,0); glVertex2i(0, 0);
            glTexCoord2d(1,0); glVertex2i(frame_width, 0);
            glTexCoord2d(1,1); glVertex2i(frame_width, frame_height);
            glTexCoord2d(0,1); glVertex2i(0, frame_height);
        glEnd();
        glDisable(GL_TEXTURE_2D);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    video_reader_close(&vr_state);

    return 0;
}