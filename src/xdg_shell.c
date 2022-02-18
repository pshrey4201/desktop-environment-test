#include <assert.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_shell.h>

#include "server.h"
#include "cursor.h"
#include "keyboard.h"
#include "xdg_shell.h"

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

void xdg_toplevel_request_move(struct wl_listener *listener, void *data) {
	struct test_view *view = wl_container_of(listener, view, request_move);
	begin_interactive(view, TEST_CURSOR_MOVE, 0);
}

void xdg_toplevel_request_resize(struct wl_listener *listener, void *data) {
	struct wlr_xdg_toplevel_resize_event *event = data;
	struct test_view *view = wl_container_of(listener, view, request_resize);
	begin_interactive(view, TEST_CURSOR_RESIZE, event->edges);
}

void server_new_xdg_surface(struct wl_listener *listener, void *data) {
	struct test_server *server = wl_container_of(listener, server, new_xdg_surface);
	struct wlr_xdg_surface *xdg_surface = data;
	
	if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP) {
		struct wlr_xdg_surface *parent = wlr_xdg_surface_from_wlr_surface(xdg_surface->popup->parent);
		struct wlr_scene_node *parent_node = parent->data;
		xdg_surface->data = wlr_scene_xdg_surface_create(parent_node, xdg_surface);
		return;
	}

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

	struct wlr_xdg_toplevel *toplevel = xdg_surface->toplevel;
	view->request_move.notify = xdg_toplevel_request_move;
	wl_signal_add(&toplevel->events.request_move, &view->request_move);
	view->request_resize.notify = xdg_toplevel_request_resize;
	wl_signal_add(&toplevel->events.request_resize, &view->request_resize);
	
}
