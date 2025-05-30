###
 # @Author: xb ssbandjl@163.com
 # @Date: 2023-09-18 00:08:08
 # @LastEditors: xb ssbandjl@163.com
 # @LastEditTime: 2023-09-19 17:41:49
 # @FilePath: /libfabric/build.sh
 # @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
### 

clear
./autogen.sh

# s131
# ./configure --prefix=/home/xb/project/libfabric/build/fabtests --enable-debug && make -j 32 && make install


# --with-libfabric: fabtests去哪里找库(lib), prefix表示fabtests的安装目录
# ./configure --with-libfabric=/opt/libfabric --prefix=/opt/fabtests && make -j 32 && sudo make install

# s63
# build_path=/home/xb/project/libfabric/libfabric/build
# build_bin=/home/xb/project/libfabric/libfabric/build/bin
# ./configure --with-libfabric=/home/xb/project/libfabric/libfabric/build/libfabric --prefix=/home/xb/project/libfabric/libfabric/build/ --enable-debug && make -j 32 && make install
# for mercury ./configure --enable-debug && make -j 32 && make install
# ./configure --prefix=/home/xb/project/libfabric/libfabric/build/ --enable-debug && make -j 32 && make install
./configure --prefix=/root/project/net/libfabric/build/ --enable-debug && make -j 32 && make install

# 测试安装成功, 可能缺少原子库:yum install libatomic* -y
export PATH=/root/project/net/libfabric//build/bin:$PATH
echo "fi_info"


# config on hpc_117
./autogen.sh
mkdir build
./configure --enable-debug --with-libfabric=/root/project/net/libfabric/build/lib --prefix=/root/project/net/libfabric/build/
make -j 64
make install


build fabtests:
cd fabtests
./autogen.sh
./configure --enable-debug --with-libfabric=/root/project/net/libfabric/build/ --prefix=/root/project/net/libfabric/build/
make -j64
make install

