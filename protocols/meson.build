wl_protocol_dir = wayland_protos.get_variable('pkgdatadir')

wayland_scanner_dep = dependency('wayland-scanner', native: true)

wayland_scanner = find_program('wayland-scanner', native: true)

xml = join_paths([wl_protocol_dir, 'stable/xdg-shell/xdg-shell.xml'])

wl_protos_src = [custom_target(
      xml.underscorify() + '_server_c',
      input: xml,
      output: '@BASENAME@-protocol.c',
      command: [wayland_scanner, 'private-code', '@INPUT@', '@OUTPUT@'],
)]

wl_protos_headers = [custom_target(
      xml.underscorify() + '_server_h',
      input: xml,
      output: '@BASENAME@-protocol.h',
      command: [wayland_scanner, 'server-header', '@INPUT@', '@OUTPUT@'],
)]

lib_server_protos = static_library(
    'server_protos', 
    wl_protos_src + wl_protos_headers,
    dependencies: wayland_server.partial_dependency(compile_args:true),
)

server_protos = declare_dependency(
    link_with: lib_server_protos,
    sources: wl_protos_headers,
)

