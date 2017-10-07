FROM ubuntu:16.04

# Add additional package repositories
RUN apt-get -qq update \
&& apt-get install -y software-properties-common \
&& apt-add-repository "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-5.0 main" \
&& apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 15CF4D18AF4F7421

# Install toolchains and respective deps
RUN apt-get -qq update \
&& apt-get install -y \
    # x86 build requirements
    gcc=4:5* g++=4:5* clang-5.0 \
    # atmega build requirements
    gcc-avr=1:4.* binutils-avr avr-libc \
    # arm build requirements
    gcc-arm-linux-gnueabihf=4:5.* \
    # wasm build requirements
    git cmake=3.5.* wget python=2.7.* openjdk-8-jre-headless nodejs=4.* \
    # js build requirements
    firefox xvfb npm \
    # python build requirements
    python3-setuptools python3-dev \
    # coverage and other CI tooling
    curl graphviz doxygen lcov=1.* \
&& rm -rf /var/lib/apt/lists/* \
\
# Set clang defaults
&& update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-5.0 1000 \
&& update-alternatives --install /usr/bin/clang clang /usr/bin/clang-5.0 1000 \
\
# Fix nodejs symlink
&& ln -s "$(which nodejs)" /usr/local/bin/node

# Build emsdk
RUN wget -q -O- "https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-portable.tar.gz" | tar -xz -C "/opt" \
&& /opt/emsdk-portable/emsdk update \
&& /opt/emsdk-portable/emsdk install sdk-tag-1.37.33-64bit \
&& /opt/emsdk-portable/emsdk activate sdk-tag-1.37.33-64bit

# Manually build binaryen port to warm cache and to prevent nonsense pthreads linking error
RUN mkdir -p /root/.emscripten_ports/binaryen \
&& wget -q -O- "https://github.com/WebAssembly/binaryen/archive/version_42.tar.gz" | tar -xz -C "/root/.emscripten_ports/binaryen/" \
&& cd /root/.emscripten_ports/binaryen/binaryen-version_42 \
&& cmake -DCMAKE_BUILD_TYPE=Release -DTARGET_ARCH:STRING=x86-64 . \
&& cmake --build . \
&& mkdir -p /root/.emscripten_cache/asmjs/ \
&& touch /root/.emscripten_cache/asmjs/binaryen_tag_version_42.txt

# Warm emsdk system lib caches
RUN printf '\n#include <stdlib.h>\n#include <stdio.h>\n#include <string.h>\n#include <stdarg.h>\nint main(void) {}\n' > /tmp/emccwarmup.c \
&& cat /tmp/emccwarmup.c \
&& cd /opt/emsdk-portable/ \
&& /opt/emsdk-portable/emsdk construct_env \
&& . /opt/emsdk-portable/emsdk_set_env.sh \
&& emcc /tmp/emccwarmup.c -o emccwarmup.js \
&& rm -f emccwarmup.js

# Install global npm modules
RUN npm install -g elm@0.18.0 elm-test

WORKDIR /opt/proj

