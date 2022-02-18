test_sources = files(
    'main.c',
    'server.c',
    'output.c',
    'xdg_shell.c',
)

test_deps = [
    drm,
    pixman,
    server_protos,
    wayland_server,
    wlroots,
    xkbcommon,
]

executable(
    'desktop-environment-test',
    test_sources,
    include_directories: [test_inc],
    dependencies: test_deps,
)
