
modprobe ib_core
modprobe ib_uverbs
modprobe mlx5_core
ibdev2netdev

s95:
mkdir -p /root/big/backup/rdma/
mv /lib/modules/5.15.0+/updates/dkms/mlx5_core.ko /root/big/backup/rdma/
rmmod mlx5_core
insmod /lib/modules/5.15.0+/kernel/drivers/net/ethernet/mellanox/mlx5/core/mlx5_core.ko
modinfo mlx5_core
ls -alh /lib/modules/5.15.0+/kernel/drivers/net/ethernet/mellanox/mlx5/core/mlx5_core.ko
depmod -a



