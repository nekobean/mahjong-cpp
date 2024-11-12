FROM ubuntu:22.04
LABEL maintainer "pystyle"

ENV LC_ALL C.UTF-8
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential \
    ca-certificates \
    cmake \
    git \
    libboost-all-dev

# Install SSH server
RUN apt-get install -y --no-install-recommends openssh-server && \
    echo "root:root" | chpasswd && \
    sed -i "s/#PermitRootLogin prohibit-password/PermitRootLogin yes/" /etc/ssh/sshd_config

RUN rm -rf /var/lib/apt/lists/*

EXPOSE 22
CMD service ssh start && /bin/bash
