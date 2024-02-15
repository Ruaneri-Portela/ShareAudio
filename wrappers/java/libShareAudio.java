public class libShareAudio {
    static {
        String sistemaOperacional = System.getProperty("os.name").toLowerCase();
        boolean isLinux = sistemaOperacional.contains("linux");
        if (isLinux) {
            String actualDirectory = System.getProperty("user.dir");
            System.load(actualDirectory+"/libShareAudioJava.so");
        } else {
            System.loadLibrary("libShareAudioJava");
        }
    }
    public static native void SA_Init(long conn);

    public static native void SA_Server(long conn);

    public static native void SA_Client(long  conn);

    public static native void SA_Close(long  conn);

    public static native void SA_SetVolumeModifier(float vol,long conn);

    public static native float SA_GetVolumeModifier(long conn);

    public static native void SA_ListAllAudioDevices(long conn);

    public static native String SA_ListAllAudioDevicesStr(long conn);

    public static native void SA_Free(long data);

    public static native String SA_Version();

    public static native long SA_Setup(int device, String host, int port, int testMode, int channel, float volMod, int waveSize, double sampleRate);

    public static native String SA_GetStats(long conn);

    public static native  void SA_SetLogNULL();

    public static native void SA_SetLogFILE(String filename, int debug);

    public static native void SA_SetLogCONSOLE(int debug);

    public static native int SA_TestDLL();

    public static native String SA_ReadLastMsg();

    public static native int SA_SendMsg(String msg);

    public static native void SA_InitWavRecord(long conn, String path);

    public static native void SA_CloseWavRecord(long conn);

    public static native long SA_GetWavFilePtr(long conn);

    public static native void SA_SetKey(long conn, String key);

    public static native void SA_SetMode(long conn, int mode);

    public long conn = 0;

}