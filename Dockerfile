# Build stage
FROM ubuntu:20.04 as builder

WORKDIR /app

# Install build dependencies
RUN apt-get update && apt-get install -y \
    g++ \
    cmake

# Copy source code
COPY . /app

# Build the application
RUN cmake . && make

# Final stage
FROM ubuntu:20.04

WORKDIR /app

# Copy binaries from the builder stage
COPY --from=builder /app/ser2net2ser /usr/local/bin/ser2net2ser

# Command to run the application
CMD ["ser2net2ser"]
