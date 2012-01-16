#include "error.h"
#include "janitor.h"

int main()
{
    TRY ()
    {
	int status = 1;

	if ((status = gvBonjour(4096, 3 * 4096)) == -1)
	{
	    THROW(e0, "gvBonjour");
	}

	printf("STATUS %i\n", status);
    }
    CATCH (e0)
    {
	puts("ERROR");
    }

    puts("SUCCESS");

    return 0;
}
