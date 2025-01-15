for i in range(1, 11):  #? ini perulangan untuk membuat baris
    for j in range(1, 11):  #? ini perulangan untuk membuat kolom
        print(f"{i}×{j} = {i * j}".ljust(10), end="")  #? ini untuk membuat format, nantinya hasil print bakal 1×1 = 1 sampai 10×10 = 100
    print()  #? ini untuk pindah ke baris baru setelah satu baris selesai