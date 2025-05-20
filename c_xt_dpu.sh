export LD_LIBRARY_PATH=/root/project/rdma/dpu_user_rdma/build/lib
export HUGE_PAGE_NUM=20
export XT_CQ_INLINE_CQE=0
export PATH=./build/bin:$PATH
ibdev2netdev
ibv_devices

# fi_pingpong -e rdm -p "verbs;ofi_rxm" -m tagged -d ib17-0 -v -I 2
fi_pingpong -e rdm -p "verbs;ofi_rxm" -m tagged -d xtrdma_0 -v -I 2 192.168.2.118
export LD_LIBRARY_PATH=/root/project/rdma/dpu_user_rdma/build/lib;gdb --args fi_pingpong -e rdm -p "verbs;ofi_rxm" -m tagged -d xtrdma_0 -v -I 2 192.168.2.118
