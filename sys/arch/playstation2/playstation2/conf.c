#include <sys/param.h>
#include <sys/conf.h>
#include <dev/cons.h>

#include "../dev/sio.h"

struct consdev constab[] = {
	&siocons,
	{ 0 },
};

