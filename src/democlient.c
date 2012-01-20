#include <stdlib.h>
#include <EGL/egl.h>

#include "client_state_tracker.0.h"
#include "errno.h"
#include "error.h"

int main()
{
    void              *display1  = malloc(sizeof(int));
    void              *context1  = malloc(sizeof(int));
    GVcontextstateptr  state1    = malloc(sizeof(struct GVcontextstate));

    void              *display2  = malloc(sizeof(int));
    void              *context2  = malloc(sizeof(int));
    GVcontextstateptr  state2    = malloc(sizeof(struct GVcontextstate));

    GVcontextstateptr  state3;

    TRY
    {
	state1->markedCurrent   = 0;
	state1->markedDestroyed = 0;

	state2->markedCurrent   = 1;
	state2->markedDestroyed = 1;

	gvSetEglContextState(display1, context1, state1);
	gvSetEglContextState(display2, context2, state2);

	state1 = gvGetEglContextState(display1, context1);
	state2 = gvGetEglContextState(display2, context2);
	    
	printf("1. %i %i\n", state1->markedCurrent, state1->markedDestroyed);
	printf("2. %i %i\n", state2->markedCurrent, state2->markedDestroyed);

	printf("%p\n", state2);

	gvSetEglContextState(NULL, NULL, state2);
	state3 = gvGetEglContextState(NULL, NULL);

	printf("2. %i %i\n", state3->markedCurrent, state3->markedDestroyed);

	printf("%p\n", state3);
    }
    CATCH (e0)
    {
	puts("DEMOCLIENT ERROR");
	return -1;
    }

    puts("DEMOCLIENT SUCCESS");
    return 0;
}
