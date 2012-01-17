#include "error.h"
#include "janitor.h"
#include "sleep.h"

int main()
{
    TRY
    {
	int status = 1;

	if ((status = gvBonjour(10 * 4096, 3 * 4096)) == -1)
	{
	    THROW(e0, "gvBonjour");
	}

	printf("STATUS1 %i\n", status);

	gvSleep(2, 0);

	if ((status = gvAuRevoir(10 * 4096)) == -1)
	{
	    THROW(e0, "gvAuRevoir");
	}

	printf("STATUS2 %i\n", status);
	
    }
    CATCH (e0)
    {
	puts("ERROR");
    }

    puts("SUCCESS");

    return 0;
}
