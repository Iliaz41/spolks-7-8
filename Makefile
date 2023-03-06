
build: synk asynk advanced generator comparator multiplier
synk:
	mpic++ -o /mnt/nfs_share/sync sync.cpp
asynk:
	mpic++ -o /mnt/nfs_share/async async.cpp
advanced:
	mpic++ -o /mnt/nfs_share/advanced groupmult.cpp
generator:
	g++ -o /mnt/nfs_share/generator randommults.cpp -B include
multiplier:
	g++ -o /mnt/nfs_share/mult multiplication.cpp -B include
comparator:
	g++ -o /mnt/nfs_share/comparator comparator.cpp -B include
resource:
	/mnt/nfs_share/generator /mnt/nfs_share/default 100 100 1
clean:
	rm -f ./mnt/nfs_share/lab78/*