# Build stage
FROM ubuntu:20.04 as builder

WORKDIR /app

# Install build dependencies
RUN apt-get update && apt-get install -y \
    g++ \
    cmake \
    libboost-system-dev \
    libboost-program-options-dev \
    libboost-log-dev \
    libboost-date-time-dev

# Copy source code
COPY . /app

# Build the application
RUN cmake . && make

# Final stage
FROM ubuntu:20.04

WORKDIR /app

# Install runtime dependencies only
RUN apt-get update && apt-get install -y \
    libboost-system1.71.0 \
    libboost-program-options1.71.0 \
    libboost-log1.71.0 \
    libboost-date-time1.71.0 && \
    rm -rf /var/lib/apt/lists/*

# Copy binaries from the builder stage
COPY --from=builder /app/ser2net2ser /usr/local/bin/ser2net2ser

# Command to run the application
CMD ["ser2net2ser"]