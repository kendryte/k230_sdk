#  1. build image:
#     docker build -t k230_sdk:latest .
#     docker build --progress=plain -t k230_sdk:latest .
#
#  2.  run docker image:
#
#     2.1 run with default user
#     docker run -it -v $(pwd):$(pwd) -v $(pwd)/toolchain:/opt/toolchain k230_sdk:latest /bin/bash
#
#     2.2 run with root user
#     docker run -u 0 -it -v $(pwd):$(pwd) -v $(pwd)/toolchain:/opt/toolchain k230_sdk:latest /bin/bash
#

FROM ubuntu:20.04
LABEL verdor=Canaan
LABEL name=k230_sdk
LABEL version=0.1
LABEL date='2023-01-10'
LABEL auther='wuwentao@canaan-creative.com'

ENV USER=gitlab
ARG UID=6000

# config for default software install
ARG DEBIAN_FRONTEND=noninteractive
# install 32 bit lib for toolchain
RUN dpkg --add-architecture i386

# config ubuntu local apt source
RUN cp /etc/apt/sources.list /etc/apt/sources_bak.list \
    && sed -i 's/archive.ubuntu.com/mirrors.tuna.tsinghua.edu.cn/g' /etc/apt/sources.list \
    && sed -i 's/security.ubuntu.com/mirrors.tuna.tsinghua.edu.cn/g' /etc/apt/sources.list

# install software
RUN apt-get clean all && \
    apt-get update && \
    apt-get install -y --fix-broken --fix-missing --no-install-recommends \
    sudo vim wget curl git git-lfs openssh-client net-tools sed tzdata expect mtd-utils inetutils-ping locales \
    sed make binutils build-essential diffutils gcc g++ bash patch gzip bzip2 perl tar cpio unzip rsync file bc findutils \
    dosfstools mtools bison flex autoconf automake \
    libc6-dev-i386 libncurses5:i386 libssl-dev \
    python3 python3-pip python-is-python3 \
    lib32z1 scons libncurses5-dev \
    kmod fakeroot pigz tree doxygen gawk pkg-config libyaml-dev libconfuse2 libconfuse-dev

# config pip local source
RUN echo \
    '[global]\n\
    timeout = 60\n\
    index-url = https://pypi.tuna.tsinghua.edu.cn/simple\n\
    extra-index-url = https://mirrors.aliyun.com/pypi/simple/ https://mirrors.cloud.tencent.com/pypi/simple\n\
    '  > /etc/pip.conf; cat /etc/pip.conf

# install pip software
RUN python3 -m pip install -U pyyaml pycryptodome gmssl \
    numpy==1.19.5  protobuf==3.17.3 Pillow onnx==1.9.0 onnx-simplifier==0.3.6 onnxoptimizer==0.2.6 onnxruntime==1.8.0  cmake

# dotnet runtime for nncase v2
RUN wget https://packages.microsoft.com/config/ubuntu/20.04/packages-microsoft-prod.deb -O packages-microsoft-prod.deb \
    && dpkg -i packages-microsoft-prod.deb && rm packages-microsoft-prod.deb \
    && apt-get update \
    && apt-get install -y dotnet-runtime-7.0 libicu-dev

# k230 canmv, required libconfuse-dev
RUN wget https://github.com/pengutronix/genimage/releases/download/v16/genimage-16.tar.xz -O /tmp/genimage-16.tar.xz \
    && cd /tmp/; tar -xf genimage-16.tar.xz; cd genimage-16 \
    && ./configure \
    && make -j \
    && make install \
    && cd .. ; rm -rf genimage*

# default encode
RUN rm -rf /var/lib/apt/lists/* \
    && localedef -i en_US -c -f UTF-8 -A /usr/share/locale/locale.alias en_US.UTF-8
ENV LANG en_US.utf8

# default timezone
ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone \
    && dpkg-reconfigure --frontend noninteractive tzdata

# add docker user and disable default root user
RUN groupadd --gid ${UID} ${USER} \
    && adduser --home /home/${USER} --uid ${UID} --gid ${UID} --shell /bin/bash --disabled-password --gecos '' ${USER} \
    && adduser ${USER} sudo \
    && echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

# toolchain and work dir
RUN mkdir -p /opt/toolchain/

# default local build flag, default is True
RUN echo "False" > /tmp/.flag

# local build, download error will set flag to True
RUN wget -P /tmp/ "https://ai.b-bug.org/k230/downloads/aitest/aitest-0.0.6.dev20240829-py3-none-any.whl" || echo "True" > /tmp/.flag

# check and install test env with local build flag
RUN FLAG=$(cat /tmp/.flag) && if [ "$FLAG" = "False" ]; then python3 -m pip install -U /tmp/aitest*.whl; else echo "Skip install"; fi \
    && FLAG=$(cat /tmp/.flag) && if [ "$FLAG" = "False" ]; then python3 -m pip install -U robotframework requests; else echo "Skip install"; fi

# switch to new user
USER ${USER}

# add ssh key to know_hosts
RUN mkdir -p /home/${USER}/.ssh/ \
    && FLAG=$(cat /tmp/.flag) && if [ "$FLAG" = "False" ]; then ssh-keyscan g.a-bug.org >> /home/${USER}/.ssh/known_hosts; else echo "Skip install"; fi \
    && ssh-keyscan github.com >> /home/${USER}/.ssh/known_hosts \
    && chmod 644 /home/${USER}/.ssh/known_hosts

# switch work dir
WORKDIR /home/${USER}/
