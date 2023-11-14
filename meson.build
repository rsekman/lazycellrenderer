project('lazyrenderer', 'cpp',
  default_options : ['warning_level=everything', 'cpp_std=c++17'],
  version : '0.1.0'
)

fs = import('fs')

gtk_dep = dependency('gtkmm-3.0')
fmt_dep = dependency('fmt')
ui_file = fs.copyfile('ui.glade')

executable('demo', 'main.cpp',
  dependencies : [gtk_dep, fmt_dep],
)
