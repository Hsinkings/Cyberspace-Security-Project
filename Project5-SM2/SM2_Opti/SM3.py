def sm3_hash(message: bytes) -> bytes:
    #SM3哈希函数实现（遵循GB/T 32905-2016）
    #消息填充
    msg_len = len(message) * 8
    message += b'\x80'
    while (len(message) * 8) % 512 != 448:
        message += b'\x00'
    message += msg_len.to_bytes(8, byteorder='big')
    
    #初始向量
    V = [0x7380166f, 0x4914b2b9, 0x172442d7, 0xda8a0600,
         0xa96f30bc, 0x163138aa, 0xe38dee4d, 0xb0fb0e4e]
    T = [0x79cc4519, 0x7a879d8a]  #常量
    
    #按512位分组处理
    for i in range(0, len(message), 64):
        B = message[i:i+64]
        W = [0] * 68  #消息扩展字
        
        #前16个字
        for j in range(16):
            W[j] = int.from_bytes(B[j*4:(j+1)*4], 'big') & 0xFFFFFFFF
        
        #后52个字
        for j in range(16, 68):
            W[j] = (W[j-16] ^ W[j-9] ^ 
                   ((W[j-3] << 15) | (W[j-3] >> 17)) ^ 
                   ((W[j-13] << 7) | (W[j-13] >> 25))) & 0xFFFFFFFF
        
        W_ = [W[j] ^ W[j+4] for j in range(64)]  #辅助扩展
        
        #压缩函数
        A, B_, C, D, E, F, G_, H = V
        for j in range(64):
            #计算SS1和SS2
            if j < 16:
                SS1 = ((A << 12) | (A >> 20)) + E + (T[0] << j)
            else:
                SS1 = ((A << 12) | (A >> 20)) + E + (T[1] << (j % 32))
            SS1 = SS1 & 0xFFFFFFFF
            SS2 = SS1 ^ ((A << 12) | (A >> 20))
            
            #计算TT1和TT2
            TT1 = (A ^ B_ ^ C) + D + SS2 + W_[j]
            TT2 = (E ^ F ^ G_) + H + SS1 + W[j]
            TT1, TT2 = TT1 & 0xFFFFFFFF, TT2 & 0xFFFFFFFF
            
            #更新状态
            A, B_, C, D = TT1, A, (B_ << 9) | (B_ >> 23), C
            E = (TT2 ^ ((TT2 << 9) | (TT2 >> 23)) ^ ((TT2 << 17) | (TT2 >> 15))) & 0xFFFFFFFF
            F, G_, H = E, F, G_
        
        #更新向量
        V = [(V[k] ^ [A, B_, C, D, E, F, G_, H][k]) & 0xFFFFFFFF for k in range(8)]
    
    #转换为字节
    hex_digest = ''.join(f'{x:08x}' for x in V)
    return bytes.fromhex(hex_digest)
    