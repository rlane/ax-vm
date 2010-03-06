all: ax-vm

ax-vm: ax-vm.o ax-runner.o

test: all
	ruby test.rb

clean:
	rm ax-vm
