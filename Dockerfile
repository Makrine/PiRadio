FROM gcc:12
FROM arm64v8/debian:bookworm

# Install packages needed for building WiringPi
RUN apt-get update && \
    apt-get install -y git build-essential dh-make debhelper fakeroot && \
    rm -rf /var/lib/apt/lists/*

# Fetch and build WiringPi
RUN git clone https://github.com/WiringPi/WiringPi.git /tmp/WiringPi && \
    cd /tmp/WiringPi && \
    ./build debian && \ 
    mv debian-template/wiringpi_*.deb . && \
    apt install ./wiringpi_*.deb


# Set working directory inside the container
WORKDIR /app

# Copy your project
COPY . .

# Build your project using your Makefile
RUN make

# Default command to run your program
CMD ["./main"]
