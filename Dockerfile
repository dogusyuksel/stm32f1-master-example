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

RUN gem install ceedling
RUN pip install gcovr

RUN git config --global --add safe.directory /workspace

RUN ln -s /usr/bin/python3 /usr/bin/python

RUN wget https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2 && \
    tar -xf gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2
ENV PATH="/gcc-arm-none-eabi-10.3-2021.10/bin:${PATH}"

ADD /patches/ /thirdparty

RUN git clone https://github.com/modm-io/cmsis-header-stm32.git /thirdparty/cmsis-header-stm32 && \
    git clone https://github.com/ARM-software/CMSIS_5.git /thirdparty/CMSIS_5 && \
    git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git /thirdparty/FreeRTOS-Kernel && \
    git clone https://github.com/STMicroelectronics/stm32f1xx_hal_driver.git /thirdparty/STM32F103X_HAL && \
    git clone https://github.com/dogusyuksel/embedded_linting.git /thirdparty/linting && \
    git clone https://github.com/mpaland/printf.git /thirdparty/custom_printf && \
    git clone https://github.com/STMicroelectronics/OpenOCD.git /thirdparty/openocd && \
    cat /thirdparty/openocd/tcl/board/stm32f103c8_blue_pill.cfg | sed -e "s/set FLASH_SIZE 0x20000/set FLASH_SIZE 0x10000/" > /thirdparty/openocd/tcl/board/stm32f103c8_custom.cfg && \
    git clone https://github.com/OpenCyphal/libcanard.git /thirdparty/libcanard && \
    cd /thirdparty/libcanard && git checkout 43de1c4966b8d1e5d57978949d63e697f045b358 && \
    git submodule update --init --recursive && \
    cd / && patch -d . -p1 < /thirdparty/transport.py.patch && patch -d . -p1 < /thirdparty/canard_stm32.patch

CMD ["/bin/bash"]

WORKDIR /workspace/
