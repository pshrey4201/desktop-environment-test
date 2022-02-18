#include <assert.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_shell.h>
#include "server.h"
#include "xdg_shell.h"

void focus_view(struct test_view *view, struct wlr_surface *surface) {
	
	struct test_server *server = view->server;

	wlr_scene_node_raise_to_top(view->scene_node);
	wl_list_remove(&view->link);
	wl_list_insert(&server->views, &view->link);

	wlr_xdg_toplevel_set_activated(view->xdg_surface, true);

}

void xdg_toplevel_map(struct wl_listener *listener, void *data) {
	struct test_view *view = wl_container_of(listener, view, map);
	wl_list_insert(&view->server->views, &view->link);
	focus_view(view, view->xdg_surface->surface);
}

void xdg_toplevel_unmap(struct wl_listener *listener, void *data) {
	struct test_view *view = wl_container_of(listener, view, unmap);
	wl_list_remove(&view->link);
}

void xdg_toplevel_destroy(struct wl_listener *listener, void *data) {
	struct test_view *view = wl_container_of(listener, view, destroy);

	wl_list_remove(&view->map.link);
	wl_list_remove(&view->unmap.link);
	wl_list_remove(&view->destroy.link);
	wl_list_remove(&view->request_move.link);
	wl_list_remove(&view->request_resize.link);

	free(view);
}

void server_new_xdg_surface(struct wl_listener *listener, void *data) {
	struct test_server *server = wl_container_of(listener, server, new_xdg_surface);
	struct wlr_xdg_surface *xdg_surface = data;

	assert(xdg_surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL);

	struct test_view *view = calloc(1, sizeof(struct test_view));
	view->server = server;
	view->xdg_surface = xdg_surface;
	view->scene_node = wlr_scene_xdg_surface_create(&view->server->scene->node, view->xdg_surface);
	view->scene_node->data = view;
	xdg_surface->data = view->scene_node;

	view->map.notify = xdg_toplevel_map;
	wl_signal_add(&xdg_surface->events.map, &view->map);
	view->unmap.notify = xdg_toplevel_unmap;
	wl_signal_add(&xdg_surface->events.unmap, &view->unmap);
	view->destroy.notify = xdg_toplevel_destroy;
	wl_signal_add(&xdg_surface->events.destroy, &view->destroy);
	
}
