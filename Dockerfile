FROM ubuntu:22.04

RUN apt-get update && \
    apt-get install -y g++ make build-essential && \
    apt-get clean

WORKDIR /app

COPY ./ ./app

RUN g++ -o server-synchronization ./app/codes/server-synchronization.cpp -pthread -lrt
RUN g++ -o server-no-synchronization ./app/codes/server-no-synchronization.cpp -pthread -lrt

RUN g++ -o client ./app/codes/client.cpp -pthread -lrt
RUN g++ -o client-spam ./app/codes/client-spam-chat.cpp -pthread -lrt

CMD ["/bin/bash"]
