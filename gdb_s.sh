export PATH=./build/fabtests/bin:$PATH
gdb --args ./build/fabtests/bin/fi_pingpong -e rdm -p "verbs;ofi_rxm" -m tagged -d mlx5_0 -v -I 2
#fi_pingpong -e rdm -p "verbs;ofi_rxm" -m tagged -d ib17-0 -v -I 2
