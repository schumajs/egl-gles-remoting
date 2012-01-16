
#include <stdio.h>
#include <EGL/egl.h>

#define TRY(lbl)
#define THROW(to, msg) perror(msg); goto to
#define CATCH(to) while (0) to:

int main()
{
    TRY ()
    {
	if (0)
	{
	    THROW(c0, "ERROR1");
	}

	if (0)
	{
	    THROW(c0, "ERROR2");
	}

	if (1)
	{
	    THROW(c1, "ERROR3");
	}
    }
    CATCH (c0)
    {
	puts("Drinnen 1");
    }
    CATCH (c1)
    {
	puts("Drinnen 2");
    }

    puts("Und draussen");

    return 0;
}
