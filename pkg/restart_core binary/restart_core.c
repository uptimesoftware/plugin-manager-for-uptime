// http://www.tuxation.com/setuid-on-shell-scripts.html

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <wait.h>
#include <unistd.h>


int main(int argc, char *argv[])
{
	setuid( 0 );
	int ret = 0;
	char cmdarg[6];
	
	// get the first argument (1) if it exists and 
	if ( argc >= 2 ) {
		strncpy(cmdarg, argv[1], sizeof(cmdarg));

		if (strcmp(cmdarg, "stop") == 0 ) {
			ret = WEXITSTATUS( system( "/etc/init.d/uptime_core stop" ) );
			printf("stopped\n");
		}
		else if ( argc = 2 && strcmp(cmdarg, "start") == 0 ) {
			ret = WEXITSTATUS( system( "/etc/init.d/uptime_core start" ) );
			printf("started\n");
		}
		else {
			printf("error\n");
			return 1;
		}
	
		if (ret != 0) {
			printf("error\n");
			return ret;
		}
	}
	else {
		printf("error\n");
		return 2;
	}

	return 0;
}
