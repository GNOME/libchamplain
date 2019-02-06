libchamplain_demos = [
  ['minimal', 'minimal.c'],
  ['launcher', ['launcher.c', 'markers.c']],
  ['animated-marker', 'animated-marker.c'],
  ['polygons', 'polygons.c'],
  ['url-marker', 'url-marker.c'],
  ['create_destroy_test', 'create-destroy-test.c'],
]

foreach demo: libchamplain_demos
  demo_name = demo.get(0)
  demo_sources = demo.get(1)

  executable(
    demo_name,
    demo_sources,
    install: false,
    dependencies: [
      libchamplain_dep,
    ]
  )
endforeach

subdir('icons')

if build_gtk_widgetry == true
  libchamplain_gtk_demos = [
    ['minimal-gtk', 'minimal-gtk.c'],
    ['launcher-gtk', ['launcher-gtk.c', 'markers.c']],
  ]

  foreach demo: libchamplain_gtk_demos
    demo_name = demo.get(0)
    demo_sources = demo.get(1)

    executable(
      demo_name,
      demo_sources,
      install: false,
      dependencies: [
        libchamplain_dep,
        libchamplain_gtk_dep,
      ]
    )
  endforeach
endif

if generate_vapi == true
  libchamplain_vala_demos = [
    ['launcher-vala', ['launcher-vala.vala', 'markers-vala.vala']],
  ]

  add_languages('vala')

  foreach demo: libchamplain_vala_demos
    demo_name = demo.get(0)
    demo_sources = demo.get(1)

    executable(
      demo_name,
      demo_sources,
      install: false,
      dependencies: [
        libchamplain_dep,
        libchamplain_gtk_dep,
        libchamplain_vapi,
      ]
    )
  endforeach
endif