from libShareAudio import *
import time
SA_SetLogNULL()
host = "hirameki-server.lan"
bhost = host.encode("UTF-8")
ctx = SA_Setup(-1, bytes(bhost), 2, 9950,0, 2, -1, 2048, -1)
SA_Init(ctx)
SA_ClientConnect(ctx)
time.sleep(1)
while True:
    user = input(">:")
    if user == "q":
        SA_Shutdown(ctx)
        break
    elif user == "v":
        vol = float(input("Volume: "))
        SA_SetVolume(ctx, vol)
    elif user == "s":
        data = SA_GetStats(ctx)
        print("Pacotes enviados:\t",data[0])
        print("Pacotes recebidos:\t",data[1])
        print("Pacotes perdidos:\t",data[1] - data[0])
        mb = (data[1] * 4 * data[2] * data[4])/(1024**2)
        print(f"Tranferencia em MB:\t{mb:.2f}")
    elif user == "i":
        data = SA_GetStats(ctx)
        print("Taxa de amostragem:\t",data[3])
        print("Tamanho do pacote:\t",data[4])
        print("Canais:\t\t\t",data[2])
        print("Dispositvo Ativo:\t", data[5])
        print("Volume:\t\t\t", SA_GetVolume(ctx))
        print(f"Connectado em {data[6]} na porta {data[7]}")
    else:
        print("Comando invalido")