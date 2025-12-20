FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential cmake git libboost-all-dev

WORKDIR /app
COPY . .

RUN mkdir build && cd build && \
    cmake .. && make && make install

FROM ubuntu:22.04
WORKDIR /app
COPY --from=builder /app/build/install ./install

EXPOSE 50000
CMD ["./install/bin/nanikiru"]
