FROM ubuntu:24.04
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    curl \
    libssl-dev \
    libboost-system-dev \
    nlohmann-json3-dev \
    libpq-dev \
    libpqxx-dev \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /TBM

COPY . .

RUN mkdir -p build && cd build && cmake .. && make -j$(nproc)

CMD ["./build/TBM"]

ENTRYPOINT ["top", "-b"]