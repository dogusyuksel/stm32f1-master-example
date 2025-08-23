FROM ubuntu:22.04
ARG DEBIAN_FRONTEND=noninteractive

USER root

RUN apt-get update && \
    apt-get -y upgrade && \
    apt-get -y --no-install-recommends install \
        build-essential \
        git \
        git-core \
        git-lfs \
        python3-dbg \
        python3-dev \
        python3-pip \
        python3-pexpect \
        python3-git \
        python3-jinja2 \
        python3-subunit \
        vim \
        cmake \
        gcc-multilib \
        g++-multilib \
        software-properties-common \
        language-pack-en-base \
        wget \
        can-utils \
        socat \
        openocd \
        stlink-tools \
        gdb-multiarch \
        valgrind \
        gdb \
        ruby \
        clang-format \
        unzip && \
    apt-get -y clean

RUN gem install ceedling -v 0.31.1
RUN pip install gcovr

RUN git config --global --add safe.directory /workspace

RUN ln -s /usr/bin/python3 /usr/bin/python

RUN wget https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2 && \
    tar -xf gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2
ENV PATH="/gcc-arm-none-eabi-10.3-2021.10/bin:${PATH}"

COPY thirdparty/renode_1.16.0_amd64.deb /tmp/
RUN apt-get update && apt-get install -y ./tmp/renode_1.16.0_amd64.deb \
 && rm /tmp/renode_1.16.0_amd64.deb

CMD ["/bin/bash"]

WORKDIR /workspace/
