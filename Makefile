all: ax-vm

ax-vm: ax-vm.o ax-runner.o

ax-vm.o: ax-vm.h

ax-runner.o: ax-vm.h

test: all
	ruby test.rb

clean:
	rm -f ax-vm *.o
