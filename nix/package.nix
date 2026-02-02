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
{
  stdenv,
  llvmPackages_21,
  lib,
  buildBazelPackage,
  bazel_7,
  nix-gitignore,
  stdenvAdapters,
  git,
}: let
  inherit (import ./lib/common.nix {inherit stdenv llvmPackages_21 stdenvAdapters lib;}) llvm version;

  buildBazelPackage' = buildBazelPackage.override {
    inherit (llvm) stdenv;
  };

  pkg-config =
    builtins.replaceStrings
    ["@version@"]
    [version]
    (builtins.readFile ../hack/pkg-config/liblogrin.pc);
in
  buildBazelPackage' rec {
    inherit version;

    pname = "logrin";
    src = nix-gitignore.gitignoreSource [] ../.;

    bazel = bazel_7;
    bazelTargets = [":logrin"];
    bazelBuildFlags = [
      "--config=opt"

      "--cxxopt=-x"
      "--cxxopt=c++"
      "--host_cxxopt=-x"
      "--host_cxxopt=c++"
    ];

    nativeBuildInputs = [git];

    removeRulesCC = false;
    removeLocalConfigCc = false;

    fetchAttrs.hash = "";
    buildAttrs.installPhase = ''
      mkdir -p $out/{lib,include/logrin}

      cp -r ${src}/include/logrin/. $out/include/logrin

      cp -v bazel-bin/liblogrin.a $out/lib/liblogrin.a
      cp -v bazel-bin/liblogrin.so $out/lib/liblogrin.so

      mkdir -p $out/lib/pkgconfig
      echo "${pkg-config}" > $out/lib/pkgconfig/liblogrin.pc

      substituteInPlace $out/lib/pkgconfig/liblogrin.pc \
        --replace-fail "prefix=@out@" "prefix=$out"
    '';

    meta = {
      description = "🐻‍❄️🏳️‍⚧️ Modern, async-aware logging framework for C++20";
      homepage = "https://docs.floofy.dev/library/cpp/logrin/${version}";
      license = lib.licenses.mit;
      platforms = lib.platforms.linux ++ lib.platforms.darwin;
      maintainers = [lib.maintainers.auguwu];
    };
  }
