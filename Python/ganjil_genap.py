number = int(input("Masukkan bilangan: ")) #? meminta input user berupa integer
if number % 2 == 0: #? fungsi menentukan bilangan ganjil/genap dengan membagi 2 jika ada sisa bagi maka ganjil, jika 0 maka genap
    print(f"{number} adalah bilangan genap.") #? mencetak hasil, mendefinisikan bilangan genap
else:
    print(f"{number} adalah bilangan ganjil.") #? mencetak hasil, mendefinisikan bilangan ganjil
