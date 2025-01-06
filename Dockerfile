FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y \
    g++ \
    libboost-all-dev \
    nlohmann-json3-dev && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN g++ course.cpp schedule.cpp server.cpp main.cpp -o main -std=c++20

CMD ["./main"]
