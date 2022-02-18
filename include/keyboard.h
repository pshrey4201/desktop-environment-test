
struct test_keyboard {
	struct wl_list link;
	struct test_server *server;
	struct wlr_input_device *device;

	struct wl_listener modifiers;
	struct wl_listener key;
};

void focus_view(struct test_view *view, struct wlr_surface *surface);
void keyboard_handle_modifiers(struct wl_listener *listener, void *data);
bool handle_keybinding(struct test_server *server, xkb_keysym_t sym);
void keyboard_handle_key(struct wl_listener *listener, void *data);
