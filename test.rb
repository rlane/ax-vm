#!/usr/bin/env ruby
require 'axlib'
require 'test/unit'

class AXTest < Test::Unit::TestCase
	def eval &b
		c = AXCode.new &b
		IO.popen './ax-vm', 'r+' do |io|
			io.write c.bytecode
			io.close_write
			stack = io.readlines.map { |x| x =~ /= (0x[a-fA-F\d]+)/ && $1.to_i(16) }.reverse
			p stack
			stack
		end
	end

	def check expected_stack, &b
		stack = eval &b
		assert_equal expected_stack, stack
	end

	def test_arithmetic
		check [12,9,45,4] do
			const16 5
			const32 7
			add
			dup
			const8 3
			sub
			dup
			const64 5
			mul
			dup
			const64 10
			div_signed
		end
	end

	def test_negative
		check [0xffffffffffffffff] do
			const64 -1
		end
	end
end

