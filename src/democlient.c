

#include <errno.h>
#include <stdlib.h>
#include <EGL/egl.h>

#include "error.h"

int main()
{
    EGLint error;

    TRY
    {
	printf("%p\n", malloc(0));

	error = eglGetError();

	printf("ERROR %i\n", error);
    }
    CATCH (e0)
    {
	puts("DEMOCLIENT ERROR");
	return -1;
    }

    puts("DEMOCLIENT SUCCESS");
    return 0;
}
