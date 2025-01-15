total_pertemuan = int(input("Masukkan total pertemuan: ")) #? meminta input user berupa integer
kehadiran = int(input("Masukkan jumlah kehadiran mahasiswa: ")) #? meminta input user berupa integer
persentase = (kehadiran / total_pertemuan) * 100 #? menghitung persentase kehadiran mahasiswa kehadiran dibagi pertemuan kemudian dikali 100
if persentase > 75: #? jika persentase kehadiran mahasiswa lebih dari 75 maka true
    print(f"Mahasiswa lulus dengan kehadiran {persentase:.2f}%.") #? menampilkan hasil persentase kehadiran mahasiswa dengan dua angka di belakang
else:
    print(f"Mahasiswa tidak lulus dengan kehadiran {persentase:.2f}%.") #? menampilkan hasil persentase kehadiran mahasiswa dengan dua angka di belakang
