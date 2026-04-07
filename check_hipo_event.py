import struct, sys

filepath = sys.argv[1]
with open(filepath, 'rb') as f:
    # Read data record at position 15532
    f.seek(15532)
    rec_hdr = f.read(56)
    words = struct.unpack('<14i', rec_hdr)
    events = words[3]
    index_len = words[4]
    data_len = words[8]
    comp_word = words[9]
    comp_len = (comp_word & 0x0FFFFFFF) * 4
    bit_info = words[5]
    pad3 = (bit_info >> 24) & 0x3
    print(f'events={events}, index_len={index_len}, data_len={data_len}, comp_len={comp_len}, pad3={pad3}')
    
    # Read compressed data and try to find event data
    # Since we can't decompress LZ4 without the module, let's check if there's 
    # an uncompressed fallback or read raw
    comp_type = (comp_word >> 28) & 0xF
    print(f'compression type: {comp_type} (1=LZ4, 2=LZ4_best, 0=none)')
    
    if comp_type == 0:
        # Uncompressed - read directly
        raw = f.read(index_len + data_len)
        evt0_size = struct.unpack_from('<i', raw, 0)[0]
        print(f'Event 0 size: {evt0_size}')
        evt0_data = raw[index_len:index_len+32]
        print(f'Event 0 first 32 bytes: {evt0_data.hex()}')
    else:
        print('Data is LZ4 compressed - need lz4 module or pip install lz4')
        print('Try: pip install lz4 --user')
