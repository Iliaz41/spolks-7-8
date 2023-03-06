set -e

make resource f=squ+1 h=2000 w=800
make resource f=squ+2 h=2000 w=800
mpirun -np 2 /mnt/nfs_share/async /mnt/nfs_share/squ+1.mtrx /mnt/nfs_share/squ+2.mtrx /mnt/nfs_share/lresult_async
mpirun -np 2 /mnt/nfs_share/advanced /mnt/nfs_share/squ+1.mtrx /mnt/nfs_share/squ+2.mtrx /mnt/nfs_share/lresult_advanced 2
mpirun -np 2 /mnt/nfs_share/sync /mnt/nfs_share/squ+1.mtrx /mnt/nfs_share/squ+2.mtrx /mnt/nfs_share/lresult_sync
/mnt/nfs_share/mult /mnt/nfs_share/squ+1.mtrx /mnt/nfs_share/squ+2.mtrx /mnt/nfs_share/lresult_mult
/mnt/nfs_share/comparator /mnt/nfs_share/lresult_async /mnt/nfs_share/lresult_mult
/mnt/nfs_share/comparator /mnt/nfs_share/lresult_advanced0 /mnt/nfs_share/lresult_mult
/mnt/nfs_share/comparator /mnt/nfs_share/lresult_sync /mnt/nfs_share/lresult_mult