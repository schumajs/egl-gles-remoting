#include "errno.h"
#include "error.h"
#include "heap_manager.h"
#include "janitor.h"
#include "sleep.h"

int main()
{
    TRY
    {
	int status;

	size_t offset0;
	size_t length0 = 3 * 4096;

	size_t offset1;
	size_t length1 = 3 * 4096;

	if (gvAlloc(&offset0, length0) == -1)
	{
	    THROW(e0, "gvAlloc");
	}

	printf("1. CLIENT -- OFFSET %zu LENGTH %zu\n", offset0, length0);

	if (gvAlloc(&offset1, length1) == -1)
	{
	    THROW(e0, "gvAlloc");
	}

	printf("2. CLIENT -- OFFSET %zu LENGTH %zu\n", offset1, length1);

	if ((status = gvBonjour(offset0, length0)) == -1)
	{
	    THROW(e0, "gvBonjour");
	}
	else
	{
	    printf("STATUS %i\n", status);
	}

	if ((status = gvBonjour(offset1, length1)) == -1)
	{
	    THROW(e0, "gvBonjour");
	}
	else
	{
	    printf("STATUS %i\n", status);
	}

	puts("PAUSE");
	gvSleep(10, 0);

	if ((status = gvAuRevoir(offset0)) == -1)
	{
	    THROW(e0, "gvAuRevoir");
	}
	else
	{
	    printf("STATUS %i\n", status);
	}

	if ((status = gvAuRevoir(offset1)) == -1)
	{
	    THROW(e0, "gvAuRevoir");
	}
	else
	{
	    printf("STATUS %i\n", status);
	}

	if ((status = gvFree(offset0)) == -1)
	{
	    THROW(e0, "gvFree");
	}
	else
	{
	    printf("STATUS %i\n", status);
	}

	if ((status = gvFree(offset1)) == -1)
	{
	    THROW(e0, "gvFree");
	}
	else
	{
	    printf("STATUS %i\n", status);
	}

	puts("WELL DONE");
    }
    CATCH (e0)
    {
	puts("DEMOCLIENT ERROR");
	return -1;
    }

    puts("DEMOCLIENT SUCCESS");
    return 0;
}
