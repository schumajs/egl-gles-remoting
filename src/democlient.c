#include <stdio.h>
#include <EGL/egl.h>

int main()
{
    printf("ERROR: %i\n", eglGetError());

    return 0;
}
