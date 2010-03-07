#!/usr/bin/env ruby
require 'axlib'
require 'test/unit'

class Integer
	def u64
		if self >= 0
			self
		else
			(self + 1) + 0xffffffffffffffff
		end
	end
end

class AXTest < Test::Unit::TestCase
	def eval &b
		c = AXCode.new &b
		IO.popen './ax-vm', 'r+' do |io|
			io.write c.bytecode
			io.close_write
			stack = io.readlines.map { |x| x =~ /= (0x[a-fA-F\d]+)/ && $1.to_i(16) }.reverse
			stack
		end
	end

	def check expected_stack, &b
		stack = eval &b
		assert_equal expected_stack.map(&:u64), stack
	end

	def test_arithmetic
		check [12,9,45,4,3] do
			const16 5
			const32 7
			add

			const16 12
			const8 3
			sub

			const32 9
			const64 5
			mul

			const8 45
			const64 10
			div_signed

			const8 27
			const8 8
			rem_signed
		end
	end

	def test_negative
		check [0xffffffffffffffff, -1, 255] do
			const64 -1
			const64 -1
			const8 -1
		end
	end

	def test_goto
		check [1,2,3] do
			goto 8 #a 8 - 0 = 8      # 0
			const8 2                 # 3=b
			goto 8 #c 13 - 5 = 8     # 5
			const8 1                 # 8=a
			goto -7 #b 3 - 10 = -7   # 10
			const8 3                 # 13=c
		end
	end

	def test_labels
		check [1,2,3] do
			goto :a

			label :b
			const8 2
			goto :c

			label :a
			const8 1
			goto :b

			label :c
			const8 3
		end
	end

	def test_stack
		check [3] do
			const8 3
			const8 1
			pop
		end

		check [3,1] do
			const8 1
			const8 3
			swap
		end

		check [3,3] do
			const8 3
			dup
		end
	end
end

