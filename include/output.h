#include <wayland-server-core.h>
#include <wlr/types/wlr_output.h>

struct test_output {
	struct wl_list link;
	struct test_server *server;
	struct wlr_output *wlr_output;
	struct wl_listener frame;	
};
void output_frame(struct wl_listener *listener, void *data);
void server_new_output(struct wl_listener *listener, void *data);
