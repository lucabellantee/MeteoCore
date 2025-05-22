FROM ubuntu:22.04

# Avoid interactive prompts during installation
ENV DEBIAN_FRONTEND=noninteractive

# Update the system and install the required dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    wget \
    bison \
    flex \
    gcc \
    make \
    graphviz \ 
    && rm -rf /var/lib/apt/lists/*

# load and compile Spin
WORKDIR /tmp
RUN wget https://github.com/nimble-code/Spin/archive/refs/tags/version-6.5.2.tar.gz \
    && tar -xzf version-6.5.2.tar.gz \
    && cd Spin-version-6.5.2/Src \
    && make \
    && cp spin /usr/local/bin/ \
    && cd / \
    && rm -rf /tmp/version-6.5.2.tar.gz /tmp/Spin-version-6.5.2

# Set the working directory
WORKDIR /work

CMD ["/bin/bash"]