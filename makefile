test: main.c
	$(CC) -g -Werror -I. -DWLR_USE_UNSTABLE -o $@ $< -I/usr/local/include -I/usr/local/include/libdrm -I/usr/local/include -I/usr/include/pixman-1 -lwlroots -lwayland-server -lxkbcommon
