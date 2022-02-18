#include <wayland-server-core.h>
#include "server.h"

int main(int argc, char *argv[]) {

	struct test_server server;
	server_init(&server);
	server_run(&server);
	server_finish(&server);
	
	return 0;
}
