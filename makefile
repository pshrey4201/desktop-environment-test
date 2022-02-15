WAYLAND_PROTOCOLS=$(shell pkg-config --variable=pkgdatadir wayland-protocols)
WAYLAND_SCANNER=$(shell pkg-config --variable=wayland_scanner wayland-scanner)

xdg-shell-protocol.h:
	$(WAYLAND_SCANNER) server-header \
		$(WAYLAND_PROTOCOLS)/stable/xdg-shell/xdg-shell.xml $@

xdg-shell-protocol.c: xdg-shell-protocol.h
	$(WAYLAND_SCANNER) private-code \
		$(WAYLAND_PROTOCOLS)/stable/xdg-shell/xdg-shell.xml $@

menubar: menubar.c menubar.h
	$(CC) -g -Werror -I. -o $@ $< `pkg-config --cflags --libs gtk4`
test: main.c xdg-shell-protocol.h xdg-shell-protocol.c
	$(CC) -g -Werror -I. -DWLR_USE_UNSTABLE -o $@ $< -I/usr/local/include -I/usr/local/include/libdrm -I/usr/local/include -I/usr/include/pixman-1 -lwlroots -lwayland-server -lxkbcommon `pkg-config --cflags --libs gtk4`
