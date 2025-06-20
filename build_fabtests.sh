export PATH=/home/xb/project/libfabric/libfabric/build/bin:$PATH
export PATH=/home/xb/project/libfabric/libfabric/build/fabtests/bin:$PATH
cd fabtests/
./configure --with-libfabric=/home/xb/project/libfabric/libfabric/build --prefix=/home/xb/project/libfabric/libfabric/build/fabtests
make && make install
echo -e "cd ../build/fabtests/bin/"

