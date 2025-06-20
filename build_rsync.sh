# ./configure --with-libfabric=/home/xb/project/net/libfabric/libfabric/build/libfabric --prefix=/home/xb/project/net/libfabric/libfabric/build/fabtests --enable-debug && make -j 32 && sudo make install
make -j 32 && sudo make install

# ssh root@c72 -C "mkdir -p /home/xb/project/net/libfabric/libfabric/"
rsync -rpvalu build root@c72:/home/xb/project/net/libfabric/libfabric/
# rsync -rpvalu s.sh c.sh root@c72:/home/xb/project/net/libfabric/libfabric/
