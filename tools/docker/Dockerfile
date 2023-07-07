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

# set default username and uid/gid
ENV USER=gitlab
ARG UID=6000
# local build flag
RUN echo "False" > /tmp/.flag

# config for default software install
ARG DEBIAN_FRONTEND=noninteractive
# install 32 bit lib for toolchain
RUN dpkg --add-architecture i386

# config ubuntu apt source
RUN sed -i 's/archive.ubuntu.com/mirrors.tuna.tsinghua.edu.cn/g' /etc/apt/sources.list \
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
kmod fakeroot pigz doxygen gawk pkg-config libyaml-dev

# config pip source
RUN python3 -m pip config set global.index-url "https://pypi.tuna.tsinghua.edu.cn/simple" \
    && python3 -m pip config set global.extra-index-url "https://mirrors.aliyun.com/pypi/simple/ https://pypi.mirrors.ustc.edu.cn/simple/ https://pypi.douban.com/simple/" \
    && python3 -m pip install -U pip \
    && python3 -m pip install -U pyyaml pycryptodome gmssl \
    && python3 -m pip install -U numpy==1.19.5  protobuf==3.17.3 Pillow \
    onnx==1.9.0 onnx-simplifier==0.3.6 onnxoptimizer==0.2.6 onnxruntime==1.8.0  cmake

# dotnet runtime for nncase v2
RUN wget https://packages.microsoft.com/config/ubuntu/20.04/packages-microsoft-prod.deb -O packages-microsoft-prod.deb \
    && dpkg -i packages-microsoft-prod.deb && rm packages-microsoft-prod.deb \
    && apt-get update \
    && apt-get install -y dotnet-runtime-7.0 libicu-dev

# install local env
RUN wget -P /tmp/ "https://ai.b-bug.org/k230/downloads/aitest/aitest-0.0.2.dev20230625-py3-none-any.whl" || echo "True" > /tmp/.flag \
    && FLAG=$(cat /tmp/.flag) && if [ "$FLAG" = "False" ]; then python3 -m pip install -U /tmp/aitest*.whl; else echo "Skip install"; fi \
    && FLAG=$(cat /tmp/.flag) && if [ "$FLAG" = "False" ]; then python3 -m pip install -U robotframework requests; else echo "Skip install"; fi

# remove software cache
RUN apt-get autoremove && apt-get autoclean && apt-get clean

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

# switch to new user
USER ${USER}

# add ssh key to know_hosts
RUN mkdir -p /home/${USER}/.ssh/ \
    && FLAG=$(cat /tmp/.flag) && if [ "$FLAG" = "False" ]; then ssh-keyscan g.a-bug.org >> /home/${USER}/.ssh/known_hosts; else echo "Skip install"; fi \
    && ssh-keyscan github.com >> /home/${USER}/.ssh/known_hosts \
    && chmod 644 ~/.ssh/known_hosts

# switch work dir
WORKDIR /home/${USER}/
