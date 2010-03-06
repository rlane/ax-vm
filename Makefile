all: ax-vm

test: all
	ruby test.rb

clean:
	rm ax-vm
