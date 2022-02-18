project(
    'desktop-environment-test',
    'c',
    default_options: [
      'werror=true',
    ],
)

add_project_arguments(
    [
	'-DWLR_USE_UNSTABLE',
    ],
    language: 'c',
)

wayland_server = dependency('wayland-server', version: '>=1.20.0')
wayland_client = dependency('wayland-client')
wayland_cursor = dependency('wayland-cursor')
wayland_egl = dependency('wayland-egl')
wayland_protos = dependency('wayland-protocols', version: '>=1.24')
wlroots = dependency('wlroots')
xkbcommon = dependency('xkbcommon')
pixman = dependency('pixman-1')
drm_full = dependency('libdrm') # only needed for drm_fourcc.h
drm = drm_full.partial_dependency(compile_args: true, includes: true)

test_inc = include_directories('include')

subdir('protocols')
subdir('src')
