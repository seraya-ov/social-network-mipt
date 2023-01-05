FROM ubuntu

RUN apt-get update && apt-get install -yq \
unzip wget build-essential autoconf libtool pkg-config gcc g++ cmake git libssl-dev zlib1g-dev librdkafka-dev mysql-server mysql-client libmysqlclient-dev libboost-all-dev openjdk-8-jdk openjdk-8-jre default-jdk

RUN apt-get install -yq libgtest-dev && cd /usr/src/gtest/ && cmake -DBUILD_SHARED_LIBS=ON && make && cp lib/*.so /usr/lib

WORKDIR /home
RUN git clone -b master https://github.com/pocoproject/poco.git

WORKDIR /home/poco
RUN mkdir cmake-build && cd cmake-build && cmake .. -DCMAKE_INSTALL_RPATH=/usr/local/lib && cmake --build . --config Release && cmake --build . --target install

WORKDIR /home
RUN git clone -b master https://github.com/no1msd/mstch.git

WORKDIR /home/mstch
RUN mkdir build && cd build && cmake .. && make && make install

# RUN export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/usr/local/lib"
# RUN echo $(ls /usr/local/lib/libPoco*)

WORKDIR /social-network-mipt
COPY ./ .
RUN rm -r build && mkdir build && cd build && cmake .. && make

WORKDIR /social-network-mipt/build
# RUN echo $(ldd ./social_network)
# RUN echo $(ls /usr/local/lib/libPoco*)
# RUN echo $(ls /usr/lib/libPoco*)
CMD ["./social_network", "--host", "172.18.0.1", "--port", "3306", "--login", "root", "--password", "root", "--database", "social_network","--init_db"]