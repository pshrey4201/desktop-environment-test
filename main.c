#include <stdlib.h>
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>

struct test_server {
	struct wl_display *wl_display;
	struct wlr_backend *backend;
	struct wlr_renderer *renderer;
	struct wlr_allocator *allocator;
	struct wlr_scene *scene;

	struct wlr_output_layout *output_layout;
};

int main(int argc, char *argv[]) {

	struct test_server server;

	server.wl_display = wl_display_create();

	server.backend = wlr_backend_autocreate(server.wl_display);

	server.renderer = wlr_renderer_autocreate(server.backend);
	wlr_renderer_init_wl_display(server.renderer, server.wl_display);

	server.allocator = wlr_allocator_autocreate(server.backend, server.renderer);

	wlr_compositor_create(server.wl_display, server.renderer);

	server.output_layout = wlr_output_layout_create();

	server.scene = wlr_scene_create();
	wlr_scene_attach_output_layout(server.scene, server.output_layout);

	const char *socket = wl_display_add_socket_auto(server.wl_display);
	
	wlr_backend_start(server.backend);

	setenv("WAYLAND_DISPLAY", socket, true);

	wl_display_run(server.wl_display);

	wl_display_destroy_clients(server.wl_display);
	wl_display_destroy(server.wl_display);
	return 0;
}
