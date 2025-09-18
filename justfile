# Justfile for PiRadio

# Build the Docker image
build:
    sudo docker build -t piradio .

# Run PiRadio in background
run:
    sudo docker rm -f piradio || true
    sudo docker run -d --name piradio \
        --device /dev/gpiomem \
        --device /dev/spidev0.0 \
        -v /home/admin/Documents/DockerContainers/PiRadio:/app \
        piradio

# Stop the container
stop:
    sudo docker stop piradio

# Show logs
logs:
    sudo docker logs -f piradio
# Access the container's shell
shell:
    sudo docker exec -it piradio /bin/bash
