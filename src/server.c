#include <stdio.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_shell.h>
#include "server.h"
#include "output.h"
#include "xdg_shell.h"

void server_init(struct test_server *server) {

	server->wl_display = wl_display_create();

	server->backend = wlr_backend_autocreate(server->wl_display);

	server->renderer = wlr_renderer_autocreate(server->backend);
	wlr_renderer_init_wl_display(server->renderer, server->wl_display);

	server->allocator = wlr_allocator_autocreate(server->backend, server->renderer);

	wlr_compositor_create(server->wl_display, server->renderer);

	server->output_layout = wlr_output_layout_create();

	wl_list_init(&server->outputs);
	server->new_output.notify = server_new_output;
	wl_signal_add(&server->backend->events.new_output, &server->new_output);

	server->scene = wlr_scene_create();
	wlr_scene_attach_output_layout(server->scene, server->output_layout);
	
	wl_list_init(&server->views);
	server->xdg_shell = wlr_xdg_shell_create(server->wl_display);
	server->new_xdg_surface.notify = server_new_xdg_surface;
	wl_signal_add(&server->xdg_shell->events.new_surface, &server->new_xdg_surface);

	const char *socket = wl_display_add_socket_auto(server->wl_display);
	
	wlr_backend_start(server->backend);

	setenv("WAYLAND_DISPLAY", socket, true);
	printf("socket: %s\n", socket);
}

void server_run(struct test_server *server) {
	wl_display_run(server->wl_display);
}

void server_finish(struct test_server *server) {
	wl_display_destroy_clients(server->wl_display);
	wl_display_destroy(server->wl_display);
}
