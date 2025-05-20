XT_SERVER:
export LD_LIBRARY_PATH=/root/project/rdma/dpu_user_rdma/build/lib
export HUGE_PAGE_NUM=20
export XT_CQ_INLINE_CQE=0
export FI_UNIVERSE_SIZE=16

cd build/bin/

# pass
./build/bin/fi_pingpong -e rdm -d xtrdma_0 -I 2 -p "verbs;ofi_rxm" -m "tagged"

# failed
./fi_dgram_pingpong -d xtrdma_0 -S all -p verbs

./fi_rma_bw -v -d xtrdma_0 -S all

MLX_CLIENT:
export LD_LIBRARY_PATH=/root/project/rdma/rdma-core/build/lib
export FI_UNIVERSE_SIZE=16
./build/bin/fi_pingpong -e rdm -d mlx5_0 -I 2 -p "verbs;ofi_rxm" -m "tagged" 192.168.2.117


./fi_dgram_pingpong -S all -p verbs 192.168.2.117

