export PATH=./build/fabtests/bin:$PATH
fi_pingpong -e rdm -p "verbs;ofi_rxm" -m tagged -d ib17-0 -v -I 2

