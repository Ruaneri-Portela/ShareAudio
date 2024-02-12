import java.util.Arrays;
import java.util.List;
import java.util.Scanner;
public class libShareAudio {

    static {
        System.loadLibrary("libShareAudioJava");
    }
    private static native void SA_Init(long conn);

    private static native void SA_Server(long conn);

    private static native void SA_Client(long  conn);

    private static native void SA_Close(long  conn);

    private static native void SA_SetVolumeModifier(float vol,long conn);

    private static native float SA_GetVolumeModifier(long conn);

    private static native void SA_ListAllAudioDevices(long conn);

    private static native String SA_ListAllAudioDevicesStr(long conn);

    private static native void SA_Free(long data);

    private static native String SA_Version();

    private static native long SA_Setup(int device, String host, int mode, int port, int testMode, int channel, float volMod, int waveSize, double sampleRate);

    private static native String SA_GetStats(long conn);

    private static native  void SA_SetLogNULL();

    private static native void SA_SetLogFILE(String filename, int debug);

    private static native void SA_SetLogCONSOLE(int debug);

    private static native int SA_TestDLL();

    private static native String SA_ReadLastMsg();

    private static native int SA_SendMsg(String msg);

    private static native void SA_InitWavRecord(long conn, String path);

    private static native void SA_CloseWavRecord();

    //private static native void* SA_GetWavFileP();

    //private static native void SA_SetWavFileP(FILE* file);
    public static void main(String[] args) {
       System.out.println(SA_TestDLL()); 
       SA_SetLogCONSOLE(1);
       System.out.println(SA_Version());
       SA_ListAllAudioDevices(0);
       long ptr = 0;
       ptr = SA_Setup(-1, "nero-pc.lan", 2, 9950, 0, 2, -1, 2048, -1);
       SA_Init(ptr);
       SA_Client(ptr);
       Scanner scanner = new Scanner(System.in);
       Boolean exit = false;
       Boolean isRecording = false;
       while(!exit){
            String userInput = scanner.nextLine();
            switch (userInput) {
                case "v":
                    System.out.print("Volume:");
                    userInput = scanner.nextLine();
                    float volMod = Float.parseFloat(userInput);
                    SA_SetVolumeModifier(volMod,ptr);
                    break;
                case "s":
                    String stats = SA_GetStats(ptr);
                    List<String> statsList = Arrays.asList(stats.split(","));
                    if(statsList.size() == 9){
                        switch (statsList.get(8)) 
                        {
                            case "2":
                                System.out.println("Connected");
                                System.out.println("Packets Sender by Server: "+ statsList.get(0));
                                System.out.println("Packets Recived by Client: "+ statsList.get(1));
                                int revc = Integer.parseInt(statsList.get(0));
                                int sends = Integer.parseInt(statsList.get(1));
                                System.out.println("Packets Lost: " + (revc - sends));
                                System.out.println("Channels: "+ statsList.get(2));
                                System.out.println("Sample Rate: "+ statsList.get(3));
                                System.out.print("Volume: " + SA_GetVolumeModifier(ptr));
                                break;
                            case "1":
                                System.out.print("Connecting");
                                break;
                            default:
                                break;
                        }
                    }
                    System.out.println();
                    break;
                case "r":
                    if(isRecording){
                        SA_CloseWavRecord();
                        System.out.println("Record Endded");
                    }else{
                        SA_InitWavRecord(ptr, "MyRecord.wav");
                        System.out.println("Record Started");
                    }
                    isRecording = !isRecording;
                    break;
                case "e":
                    exit = true;
                    break;
            }
       }
       scanner.close();
       SA_Close(ptr);
    }
}