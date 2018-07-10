executable('minimal',
  'minimal.c',
  dependencies: libchamplain_dep
)

executable('launcher',
  'launcher.c',
  'markers.c',
  dependencies: libchamplain_dep
)

executable('animated-marker',
  'animated-marker.c',
  dependencies: libchamplain_dep
)

executable('polygons',
  'polygons.c',
  dependencies: libchamplain_dep
)

executable('url-marker',
  'url-marker.c',
  dependencies: libchamplain_dep
)

executable('create-destroy-test',
  'create-destroy-test.c',
  dependencies: libchamplain_dep
)

if get_option('gtk')
  executable('minimal-gtk',
    'minimal-gtk.c',
    dependencies: libchamplain_gtk_dep
  )

  executable('launcher-gtk',
    'launcher-gtk.c',
    'markers.c',
    dependencies: libchamplain_gtk_dep
  )

  if get_option('memphis')
    executable('local-rendering',
      'local-rendering.c',
      dependencies: libchamplain_gtk_dep
    )
  endif

  if get_option('vala-support')
    add_languages('vala')
    executable('launcher-vala',
      'launcher-vala.vala',
      'markers-vala.vala',
      dependencies: [ libchamplain_gtk_dep, libchamplain_vapi ]
    )
  endif
endif