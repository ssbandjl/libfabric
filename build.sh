###
 # @Author: xb ssbandjl@163.com
 # @Date: 2023-09-18 00:08:08
 # @LastEditors: xb ssbandjl@163.com
 # @LastEditTime: 2023-09-18 11:04:49
 # @FilePath: /libfabric/build.sh
 # @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
### 

clear
./autogen.sh

# s131
# ./configure --prefix=/home/xb/project/libfabric/build/fabtests --enable-debug && make -j 32 && make install

# s63
build_path=/home/xb/project/libfabric/libfabric/build
build_bin=/home/xb/project/libfabric/libfabric/build/bin
./configure --prefix=$build_path --enable-debug && make -j 32 && make install

# 测试安装成功, 可能缺少原子库:yum install libatomic* -y
export PATH=$build_bin:$PATH
fi_info

