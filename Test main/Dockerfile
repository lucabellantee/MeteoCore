FROM ubuntu:22.04

# Evita prompt interattivi durante l'installazione
ENV DEBIAN_FRONTEND=noninteractive

# Aggiorna il sistema e installa le dipendenze necessarie
RUN apt-get update && apt-get install -y \
    build-essential \
    wget \
    bison \
    flex \
    gcc \
    make \
    graphviz \ 
    && rm -rf /var/lib/apt/lists/*

# Scarica e compila Spin
WORKDIR /tmp
RUN wget https://github.com/nimble-code/Spin/archive/refs/tags/version-6.5.2.tar.gz \
    && tar -xzf version-6.5.2.tar.gz \
    && cd Spin-version-6.5.2/Src \
    && make \
    && cp spin /usr/local/bin/ \
    && cd / \
    && rm -rf /tmp/version-6.5.2.tar.gz /tmp/Spin-version-6.5.2

# Imposta la directory di lavoro
WORKDIR /work

CMD ["/bin/bash"]