#include <wayland-server-core.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xcursor_manager.h>

#include "server.h"
#include "keyboard.h"
#include "xdg_shell.h"
#include "cursor.h"

struct test_view *desktop_view_at(struct test_server *server, double lx, double ly, struct wlr_surface **surface, double *sx, double *sy) {
	struct wlr_scene_node *node = wlr_scene_node_at(
		&server->scene->node, lx, ly, sx, sy);
	if (node == NULL || node->type != WLR_SCENE_NODE_SURFACE) {
		return NULL;
	}
	*surface = wlr_scene_surface_from_node(node)->surface;
	while (node != NULL && node->data == NULL) {
		node = node->parent;
	}
	return node->data;
}

void process_cursor_move(struct test_server *server, uint32_t time) {
	struct test_view *view = server->grabbed_view;
	view->x = server->cursor->x - server->grab_x;
	view->y = server->cursor->y - server->grab_y;
	wlr_scene_node_set_position(view->scene_node, view->x, view->y);
}

void process_cursor_resize(struct test_server *server, uint32_t time) {
	struct test_view *view = server->grabbed_view;
	double border_x = server->cursor->x - server->grab_x;
	double border_y = server->cursor->y - server->grab_y;
	int new_left = server->grab_geobox.x;
	int new_right = server->grab_geobox.x + server->grab_geobox.width;
	int new_top = server->grab_geobox.y;
	int new_bottom = server->grab_geobox.y + server->grab_geobox.height;

	if (server->resize_edges & WLR_EDGE_TOP) {
		new_top = border_y;
		if (new_top >= new_bottom) {
			new_top = new_bottom - 1;
		}
	} else if (server->resize_edges & WLR_EDGE_BOTTOM) {
		new_bottom = border_y;
		if (new_bottom <= new_top) {
			new_bottom = new_top + 1;
		}
	}
	if (server->resize_edges & WLR_EDGE_LEFT) {
		new_left = border_x;
		if (new_left >= new_right) {
			new_left = new_right - 1;
		}
	} else if (server->resize_edges & WLR_EDGE_RIGHT) {
		new_right = border_x;
		if (new_right <= new_left) {
			new_right = new_left + 1;
		}
	}

	struct wlr_box geo_box;
	wlr_xdg_surface_get_geometry(view->xdg_surface, &geo_box);
	view->x = new_left - geo_box.x;
	view->y = new_top - geo_box.y;
	wlr_scene_node_set_position(view->scene_node, view->x, view->y);

	int new_width = new_right - new_left;
	int new_height = new_bottom - new_top;
	wlr_xdg_toplevel_set_size(view->xdg_surface, new_width, new_height);
}

void process_cursor_motion(struct test_server *server, uint32_t time) {
	if (server->cursor_mode == TEST_CURSOR_MOVE) {
		process_cursor_move(server, time);
		return;
	} else if (server->cursor_mode == TEST_CURSOR_RESIZE) {
		process_cursor_resize(server, time);
		return;
	}

	double sx, sy;
	struct wlr_seat *seat = server->seat;
	struct wlr_surface *surface = NULL;
	struct test_view *view = desktop_view_at(server, server->cursor->x, server->cursor->y, &surface, &sx, &sy);
	if (!view) {
		wlr_xcursor_manager_set_cursor_image(server->cursor_mgr, "left_ptr", server->cursor);
	}
	if (surface) {
		wlr_seat_pointer_notify_enter(seat, surface, sx, sy);
		wlr_seat_pointer_notify_motion(seat, time, sx, sy);
	} else {
		wlr_seat_pointer_clear_focus(seat);
	}
}

void server_cursor_motion(struct wl_listener *listener, void *data) {
	struct test_server *server = wl_container_of(listener, server, cursor_motion);
	struct wlr_event_pointer_motion *event = data;
	wlr_cursor_move(server->cursor, event->device, event->delta_x, event->delta_y);
	process_cursor_motion(server, event->time_msec);
}

void server_cursor_motion_absolute(struct wl_listener *listener, void *data) {
	struct test_server *server = wl_container_of(listener, server, cursor_motion_absolute);
	struct wlr_event_pointer_motion_absolute *event = data;
	wlr_cursor_warp_absolute(server->cursor, event->device, event->x, event->y);
	process_cursor_motion(server, event->time_msec);
}

void server_cursor_button(struct wl_listener *listener, void *data) {
	struct test_server *server = wl_container_of(listener, server, cursor_button);
	struct wlr_event_pointer_button *event = data;
	wlr_seat_pointer_notify_button(server->seat, event->time_msec, event->button, event->state);
	double sx, sy;
	struct wlr_surface *surface = NULL;
	struct test_view *view = desktop_view_at(server, server->cursor->x, server->cursor->y, &surface, &sx, &sy);
	if (event->state == WLR_BUTTON_RELEASED) {
		server->cursor_mode = TEST_CURSOR_PASSTHROUGH;
	} else {
		focus_view(view, surface);
	}
}

void server_cursor_axis(struct wl_listener *listener, void *data) {
	struct test_server *server = wl_container_of(listener, server, cursor_axis);
	struct wlr_event_pointer_axis *event = data;
	wlr_seat_pointer_notify_axis(server->seat, event->time_msec, event->orientation, event->delta, event->delta_discrete, event->source);
}


void server_cursor_frame(struct wl_listener *listener, void *data) {
	struct test_server *server = wl_container_of(listener, server, cursor_frame);
	wlr_seat_pointer_notify_frame(server->seat);
}

void begin_interactive(struct test_view *view, enum test_cursor_mode mode, uint32_t edges) {
	struct test_server *server = view->server;
	struct wlr_surface *focused_surface = server->seat->pointer_state.focused_surface;
	if (view->xdg_surface->surface !=
			wlr_surface_get_root_surface(focused_surface)) {
		return;
	}
	server->grabbed_view = view;
	server->cursor_mode = mode;

	if (mode == TEST_CURSOR_MOVE) {
		server->grab_x = server->cursor->x - view->x;
		server->grab_y = server->cursor->y - view->y;
	} else {
		struct wlr_box geo_box;
		wlr_xdg_surface_get_geometry(view->xdg_surface, &geo_box);

		double border_x = (view->x + geo_box.x) + ((edges & WLR_EDGE_RIGHT) ? geo_box.width : 0);
		double border_y = (view->y + geo_box.y) + ((edges & WLR_EDGE_BOTTOM) ? geo_box.height : 0);
		server->grab_x = server->cursor->x - border_x;
		server->grab_y = server->cursor->y - border_y;

		server->grab_geobox = geo_box;
		server->grab_geobox.x += view->x;
		server->grab_geobox.y += view->y;

		server->resize_edges = edges;
	}
}
