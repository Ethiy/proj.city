FROM ubuntu:16.04
ENV http_proxy http://proxy.ign.fr:3128
ENV https_proxy http://proxy.ign.fr:3128
RUN apt-get -y update
RUN apt-get -y upgrade
RUN apt-get install -y \
            gcc-5-base\
            libgcc-5-dev\
            g++\
            cmake\
            git
RUN apt-get install -y \
            libboost-filesystem-dev\
            libboost-system-dev\
            lib3ds-dev\
            libcgal-dev\
            libcgal-qt5-dev
WORKDIR /home
RUN git clone https://github.com/Ethiy/3DSceneModel.git
WORKDIR 3DSceneModel/
RUN mkdir build && mkdir build/linux
WORKDIR build/linux
RUN cmake -DCGAL_DONT_OVERRIDE_CMAKE_FLAGS=ON ../..
RUN make -j2 all