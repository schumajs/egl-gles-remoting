#include "errno.h"
#include "error.h"
#include "janitor.h"
#include "sleep.h"

int main()
{
    TRY
    {
	int    status  = 1;
	size_t offset0 = 4096;
	size_t offset1 = 4 * 4096;

	puts("\nBonjour 1\n");

	if ((status = gvBonjour(offset0, 3 * 4096)) == -1)
	{
	    errno = -1;
	    THROW(e0, "gvBonjour");
	}

	printf("STATUS0 %i\n", status);

	puts("\nBonjour 2\n");

	if ((status = gvBonjour(offset1, 3 * 4096)) == -1)
	{
	    errno = -1;
	    THROW(e0, "gvBonjour");
	}

	printf("STATUS1 %i\n", status);

	gvSleep(10, 0);

	puts("\nAu Revoir 1\n");

	if ((status = gvAuRevoir(offset0)) == -1)
	{
	    errno = -1;
	    THROW(e0, "gvAuRevoir");
	}

	printf("STATUS2 %i\n", status);

	puts("\nAu Revoir 2\n");

	if ((status = gvAuRevoir(offset1)) == -1)
	{
	    errno = -1;
	    THROW(e0, "gvAuRevoir");
	}

	printf("STATUS3 %i\n", status);
	
    }
    CATCH (e0)
    {
	puts("DEMOCLIENT ERROR");
	return -1;
    }

    puts("DEMOCLIENT SUCCESS");

    return 0;
}
