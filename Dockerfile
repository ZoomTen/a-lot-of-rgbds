FROM alpine:latest

RUN apk update && \
    apk add \
        make \
        gcc \
        libc-dev \
        m4 \
        patch \
        pkgconfig \
        bash \
        g++

ENV CFLAGS="-static -D_GNU_SOURCE -D__MUSL__"
ENV CXXFLAGS="-static -D_GNU_SOURCE -D__MUSL__"
ENV CPPFLAGS="-static -D_GNU_SOURCE -D__MUSL__"

RUN mkdir -p /tools

RUN cd /tools && \
    wget http://ftp.gnu.org/gnu/bison/bison-2.6.tar.gz && \
    wget http://ftp.gnu.org/gnu/bison/bison-3.7.6.tar.gz && \
    wget https://www.zlib.net/fossils/zlib-1.2.10.tar.gz && \
    wget https://download.sourceforge.net/libpng/libpng-1.6.28.tar.gz && \
    wget https://github.com/westes/flex/releases/download/v2.6.3/flex-2.6.3.tar.gz

RUN cd /tools && \
    tar xvf bison-2.6.tar.gz && \
    tar xvf bison-3.7.6.tar.gz && \
    tar xvf zlib-1.2.10.tar.gz && \
    tar xvf libpng-1.6.28.tar.gz && \
    tar xvf flex-2.6.3.tar.gz

COPY . /work
WORKDIR /work
CMD ["bash", "./build-linux.sh"]
