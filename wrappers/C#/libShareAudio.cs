using System.Runtime.InteropServices;
namespace libShareAudio
{
    public class SA
    {
        private const string Version = "0.0.1";
        [DllImport("libShareAudio.dll")]
        public static extern int SA_TestDLL();
        [DllImport("libShareAudio.dll")]
        private static extern IntPtr SA_ListAllAudioDevicesStr(IntPtr Context);
        [DllImport("libShareAudio.dll")]
        private static extern IntPtr SA_Free(IntPtr Ptr);
        [DllImport("libShareAudio.dll")]
        private static extern IntPtr SA_Version();
        [DllImport("libShareAudio.dll")]
        private static extern void SA_SetVolumeModifier(float VolumeModifier, IntPtr Context);
        [DllImport("libShareAudio.dll")]
        private static extern IntPtr SA_Setup(int device, IntPtr host, int mode, int port, int testMode, int channel, float volMod, int waveSize, double sampleRate);
        [DllImport("libShareAudio.dll")]
        private static extern IntPtr SA_Init(IntPtr conn);
        [DllImport("libShareAudio.dll")]
        private static extern void SA_Server(IntPtr conn);
        [DllImport("libShareAudio.dll")]
        private static extern void SA_Client(IntPtr conn);
        [DllImport("libShareAudio.dll")]
        private static extern void SA_SetLogNULL();
        [DllImport("libShareAudio.dll")]
        private static extern void SA_SetLogFILE(IntPtr filename, int debug);
        [DllImport("libShareAudio.dll")]
        private static extern void SA_SetLogCONSOLE(int debug);
        [DllImport("libShareAudio.dll")]
        private static extern void SA_Shutdown(IntPtr conn);
        [DllImport("libShareAudio.dll")]
        private static extern void SA_Close(IntPtr conn);
        [DllImport("libShareAudio.dll")]
        private static extern void SA_InitWavRecord(IntPtr conn, IntPtr path);
        [DllImport("libShareAudio.dll")]
        private static extern void SA_CloseWavRecord();
        [DllImport("libShareAudio.dll")]
        private static extern IntPtr SA_GetStats(IntPtr conn);

        public static List<List<string>> ListAllAudioDevices(IntPtr ctx)
        {
            IntPtr ptr = SA_ListAllAudioDevicesStr(ctx);
            if (ptr == IntPtr.Zero)
            {
                return new List<List<string>>();
            }
            string Devices = Marshal.PtrToStringAnsi(ptr);
            if (Devices == "")
            {
                return new List<List<string>>();
            }
            List<List<string>> DeviceList = new List<List<string>>();
            foreach (string DeviceLine in Devices.Split('\n'))
            {
                List<string> DeviceListContent = new List<string>();
                foreach (string DeviceInfo in DeviceLine.Split(','))
                {
                    DeviceListContent.Add(DeviceInfo);
                }
                DeviceList.Add(DeviceListContent);
            }
            SA_Free(ptr);
            return DeviceList;
        }

        public static List<string> GetVersion()
        {
            IntPtr ptr = SA_Version();
            if (ptr == IntPtr.Zero)
            {
                return new List<String>();
            }
            string VersionStr = Marshal.PtrToStringAnsi(ptr);
            SA_Free(ptr);
            if (VersionStr == "")
            {
                return new List<string>();
            }
            List<string> versionData = new List<string>();
            foreach (string versionInfo in VersionStr.Split(','))
            {
                versionData.Add(versionInfo);
            }
            versionData.Add(Version);
            return versionData;
        }

        public static void SetVolumeModifier(float VolumeModifier, IntPtr Context)
        {
            SA_SetVolumeModifier(VolumeModifier / 100, Context);
        }

        public static IntPtr Setup(int device = -1, string host = "localhost", int mode = 0, int port = 9950, int testMode = 0, int channel = 2, float volMod = -1, int waveSize = 2048, double sampleRate = -1)
        {
            return SA_Setup(device, Marshal.StringToHGlobalAnsi(host), mode, port, testMode, channel, volMod, waveSize, sampleRate);
        }

        public static void Init(IntPtr conn)
        {
            SA_Init(conn);
        }

        public static void Server(IntPtr conn)
        {
            SA_Server(conn);
        }

        public static void Client(IntPtr conn)
        {
            SA_Client(conn);
        }

        public static void SetLogNULL()
        {
            SA_SetLogNULL();
        }

        public static void SetLogFile(string filename = "log.txt", int debug = 0)
        {
            SA_SetLogFILE(Marshal.StringToHGlobalAnsi(filename), debug);
        }

        public static void SetLogConsole(int debug = 0)
        {
            SA_SetLogCONSOLE(debug);
        }

        public static void Shutdown(IntPtr conn)
        {
            SA_Shutdown(conn);
        }

        public static void Close(IntPtr conn)
        {
            SA_Close(conn);
        }

        public static void InitWavRecord(IntPtr conn, string path)
        {
            SA_InitWavRecord(conn, Marshal.StringToHGlobalAnsi(path));
        }

        public static void CloseWavRecord()
        {
            SA_CloseWavRecord();
        }

        public static List<string> GetStats(IntPtr conn)
        {
            IntPtr ptr = SA_GetStats(conn);
            if (ptr == IntPtr.Zero)
            {
                return new List<string>();
            }
            string StatsStr = Marshal.PtrToStringAnsi(ptr);
            SA_Free(ptr);
            if (StatsStr == "")
            {
                return new List<string>();
            }
            List<string> StatsData = new List<string>();
            foreach (string StatsInfo in StatsStr.Split(','))
            {
                StatsData.Add(StatsInfo);
            }
            return StatsData;
        }

    }
}
