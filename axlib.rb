class AXCode
	OPCODES = {
		:float => 0x01,
		:add => 0x02,
		:sub => 0x03,
		:mul => 0x04,
		:div_signed => 0x05,
		:div_unsigned => 0x06,
		:rem_signed => 0x07,
		:rem_unsigned => 0x08,
		:lsh => 0x09,
		:rsh_signed => 0x0a,
		:rsh_unsigned => 0x0b,
		:trace => 0x0c,
		:trace_quick => 0x0d,
		:log_not => 0x0e,
		:bit_and => 0x0f,
		:bit_or => 0x10,
		:bit_xor => 0x11,
		:bit_not => 0x12,
		:equal => 0x13,
		:less_signed => 0x14,
		:less_unsigned => 0x15,
		:ext => 0x16,
		:ref8 => 0x17,
		:ref16 => 0x18,
		:ref32 => 0x19,
		:ref64 => 0x1a,
		:ref_float => 0x1b,
		:ref_double => 0x1c,
		:ref_long_double => 0x1d,
		:l_to_d => 0x1e,
		:d_to_l => 0x1f,
		:if_goto => 0x20,
		:goto => 0x21,
		:const8 => 0x22,
		:const16 => 0x23,
		:const32 => 0x24,
		:const64 => 0x25,
		:reg => 0x26,
		:end => 0x27,
		:dup => 0x28,
		:pop => 0x29,
		:zero_ext => 0x2a,
		:swap => 0x2b,
		:trace16 => 0x30,
	}

	IMMS = {
		:ext => 1,
		:zero_ext => 1,
		:if_goto => 2,
		:goto => 2,
		:const8 => 1,
		:const16 => 2,
		:const32 => 4,
		:const64 => 8,
		:reg => 2,
		:trace_quick => 1,
		:trace16 => 2,
	}

	def initialize &b
		@a = []
		@relocations = []
		@labels = {}
		instance_eval &b if block_given?
	end

	def method_missing op, imm=nil
		fail 'unknown op' unless OPCODES[op]
		fail 'missing immediate' if IMMS[op] and not imm
		fail 'unexpected immediate' if !IMMS[op] and imm
		@a << OPCODES[op]
		if imm.is_a? Integer
			@a.concat [imm].pack('Q').bytes.to_a[0...IMMS[op]].reverse if imm
		elsif imm.is_a? Symbol
			@relocations << [@a.size, @a.size-1, imm]
			@a.concat [nil,nil]
		elsif imm
			fail
		end
		nil
	end

	def label sym
		fail "duplicate label #{sym}" if @labels[sym]
		@labels[sym] = @a.size
	end

	def relocated
		a = @a.dup
		@relocations.each do |pos, start, sym|
			target = @labels[sym] or fail "undefined symbol #{sym}"
			offset = target - start
			a[pos..(pos+1)] = [offset].pack('n').bytes.to_a
		end
		a
	end

	def bytecode
		relocated.map(&:chr).join
	end

	undef :sub, :dup
end
