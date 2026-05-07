# 🐻‍❄️🏳️‍⚧️ Logrin: Modern, async-aware logging framework for C++20
# Copyright (c) 2026 Noel Towa <cutie@floofy.dev>, et al.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

project(
    'logrin',
    'cpp',

    license: 'MIT',
    meson_version: '>=1.1',
    version: files('.logrin-version'),
    default_options: ['cpp_std=c++23', 'warning_level=3'],
)

cpp = meson.get_compiler('cpp')
compiler_id = cpp.get_id()
copts = cpp.get_supported_arguments(
    '/DNOMINMAX',
    '-fdiagnostics-color=always',
    '-Wmost',
    '-Wpedantic',
    '-Wconversion-null',
    '-Wmissing-declarations',
    '-Wnon-virtual-dtor',
    '-Wundef',
    '-Wunused-local-typedef',
    '-Wunused-result',
    '-Wvarargs',
    '-Wcomma',
    '-Wno-conversion',
    '-Wno-gcc-compat',
    '-Wno-gnu-statement-expression',
    '-Wno-c2y-extensions',
    '-Wno-c++26-extensions',
    '-Wno-pragmas',
    '-Wno-missing-designated-field-initializers',
)

fs = import('fs')
version_raw = fs.read('.logrin-version').strip()

version_parts = version_raw.split('.')
if version_parts.length() == 2
    yy = version_parts[0].to_int()
    mm = version_parts[1].replace('-dev', '').to_int()
    dd = 0
    patch = 0
elif version_parts.length() == 3
    yy = version_parts[0].to_int()
    mm = version_parts[1].to_int()
    dd = version_parts[2].replace('-dev', '').to_int()
    patch = 0
elif version_parts.length() == 4
    yy = version_parts[0].to_int()
    mm = version_parts[1].to_int()
    dd = version_parts[2].replace('-dev', '').to_int()
    patch = version_parts[3].to_int()
else
    error('invalid version format: ' + version_raw)
endif

logrin_version = yy * 1000000 + mm * 10000 + dd * 100 + patch
logrin_devbuild = false

add_global_arguments(
    '-DLOGRIN_VERSION=' + logrin_version.to_string(),
    language: ['c', 'cpp'],
)

if logrin_devbuild
    add_global_arguments('-DLOGRIN_DEVBUILD=1', language: ['c', 'cpp'])
endif

logrin_platform_defines = []

if compiler_id == 'clang'
    logrin_platform_defines += ['-DNOMINMAX']
elif compiler_id == 'clang-cl'
    logrin_platform_defines += ['-DNOMINMAX']
elif compiler_id == 'gcc'
    logrin_platform_defines += ['-DNOMINMAX']
elif compiler_id == 'msvc'
    logrin_platform_defines += [
        '-DNOMINMAX',
        '-DWIN32_LEAN_AND_MEAN',
    ]
endif

add_project_arguments(logrin_platform_defines, language: ['c', 'cpp'])
add_project_arguments(copts, language: ['cpp'])

logrin_include_dir = include_directories('include')

liblogrin_srcs = files(
    'src/LogFactory.cc',
    'src/LogRecord.cc',
    'src/Logger.cc',
    'src/formatters/Json.cc',
    'src/formatters/Pattern.cc',
    'src/formatters/pattern/Parser.cc',
    'src/formatters/pattern/Segment.cc',
    'src/sinks/Console.cc',
)

liblogrin_hdrs = files(
    'include/logrin/AsyncSink.h',
    'include/logrin/Formatter.h',
    'include/logrin/Formatter/Json.h',

    'include/logrin/Formatter/Pattern.h',
    'include/logrin/Formatter/Pattern/Error.h',
    'include/logrin/Formatter/Pattern/Parser.h',
    'include/logrin/Formatter/Pattern/Segment.h',
    'include/logrin/LogFactory.h',
    'include/logrin/LogRecord.h',
    'include/logrin/Logger.h',
    'include/logrin/Sink.h',

    'include/logrin/Sinks/Console.h',
    'include/logrin/Sinks/OpenTelemetry.h',

    'include/logrin/detail/config.h',
)

json_dep = dependency('nlohmann_json')
violet_dep = dependency('violet')
violet_io_dep = dependency('violet_io')

logrin_lib = library(
    'logrin',
    liblogrin_srcs,

    include_directories: logrin_include_dir,
    dependencies: [violet_dep, violet_io_dep, json_dep],
)

logrin_dep = declare_dependency(
    include_directories: logrin_include_dir,
    link_with: logrin_lib,
    dependencies: [json_dep, violet_dep, violet_io_dep],
)

install_headers(liblogrin_hdrs, preserve_path: true)

if get_option('build_examples')
    subdir('examples/console_sink')
endif
