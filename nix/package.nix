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
{}
# this is roughly what it'll be but it doesn't work :(
# TODO(@auguwu): use bazel_8 instead of bazel 7
#
# {
#   buildBazelPackage,
#   lib,
#   bazel_7,
#   git,
# }: let
#   version = builtins.readFile ../.logrin-version;
# in
#   buildBazelPackage rec {
#     inherit version;
#     pname = "logrin";
#     src = ../.;
#     nativeBuildInputs = [git];
#     removeRulesCC = false;
#     bazel = bazel_7;
#     bazelTestTargets = ["//..."];
#     bazelTargets = ["//:logrin"];
#     buildFlags = [
#       "--config=opt"
#       "--cxxopt=-x"
#       "--cxxopt=c++"
#       "--host_cxxopt=-x"
#       "--host_cxxopt=c++"
#     ];
#     fetchAttrs.hash = "";
#     buildAttrs.installPhase = ''
#             mkdir -p $out/include/logrin
#             cp -r ${src}/include/logrin/**/*.h $out/include/logrin/
#             mkdir -p $out/lib
#             # Add the `.so` and `.a` files into lib
#             cp -v bazel-bin/liblogrin.a $out/lib
#             cp -v bazel-bin/liblogrin.so $out/lib
#             # Generate a pkg-config file
#             mkdir -p $out/lib/pkgconfig
#             cat > $out/lib/pkgconfig/liblogrin.pc <<EOF
#       # 🐻‍❄️🏳️‍⚧️ Logrin: Modern, async-aware logging framework for C++20
#       # Copyright (c) 2026 Noel Towa <cutie@floofy.dev>, et al.
#       #
#       # Permission is hereby granted, free of charge, to any person obtaining a copy
#       # of this software and associated documentation files (the "Software"), to deal
#       # in the Software without restriction, including without limitation the rights
#       # to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#       # copies of the Software, and to permit persons to whom the Software is
#       # furnished to do so, subject to the following conditions:
#       #
#       # The above copyright notice and this permission notice shall be included in all
#       # copies or substantial portions of the Software.
#       #
#       # THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#       # IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#       # FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#       # AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#       # LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#       # OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#       # SOFTWARE.
#       prefix=$out
#       exec_prefix=''${prefix}
#       includedir=''${prefix}/include
#       libdir=''${prefix}/lib
#       Name: logrin
#       description: Modern, async-aware logging framework for C++20
#       Version: ${version}
#       Cflags: -I''${includedir}
#       Libs: -L''${libdir}
#       EOF
#     '';
#     meta = with lib; {
#       description = "🐻‍❄️🏳️‍⚧️ Modern, async-aware logging framework for C++20";
#       homepage = "https://docs.floofy.dev/library/cpp/logrin/${version}";
#       license = licenses.mit;
#       platforms = platforms.linux ++ platforms.darwin;
#       maintainers = [maintainers.auguwu];
#     };
#   }
