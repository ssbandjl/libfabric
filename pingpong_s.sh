# query dev: ibdev2netdev
export PATH=./build/fabtests/bin:$PATH

fi_pingpong -e rdm -p "verbs;ofi_rxm" -m tagged -d mlx5_0 -v -I 2
#fi_pingpong -e rdm -p "verbs;ofi_rxm" -m tagged -d ib17-0 -v -I 2
