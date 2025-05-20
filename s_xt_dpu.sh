export PATH=./build/bin:$PATH

ibdev2netdev
ibv_devices

fi_pingpong -e rdm -p "verbs;ofi_rxm" -m tagged -d mlx5_0 -v -I 2 

