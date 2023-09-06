clear
./autogen.sh

# s131
./configure --prefix=/home/xb/project/libfabric/build/fabtests --enable-debug && make -j 32 && make install

# 测试安装成功, 可能缺少原子库:yum install libatomic* -y
export PATH=/home/xb/project/libfabric/build/fabtests/bin:$PATH
fi_info

