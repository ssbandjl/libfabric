clear
./autogen.sh
./configure --prefix=/root/project/net/libfabric/build/ --enable-debug && make -j 32 && make install

# export PATH=/root/project/net/libfabric//build/bin:$PATH
# echo "fi_info"

# build fabtests:
cd fabtests
./autogen.sh
./configure --enable-debug --with-libfabric=/root/project/net/libfabric/build/ --prefix=/root/project/net/libfabric/build/
make -j64
make install
cd -
