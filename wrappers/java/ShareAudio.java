import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.time.Instant;
import java.time.Duration;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JOptionPane;

public class ShareAudio {

    static private Instant initRecord;

    static private boolean recordAsStarted = false;

    static private void fillComboBox(JComboBox<String> cBox, Boolean isServer) {
        cBox.removeAllItems();
        String devices = libShareAudio.SA_ListAllAudioDevicesStr(0);
        String[] deviceList = devices.split("\n");
        for (String device : deviceList) {
            String[] deviceInfo = device.split(",");
            if (deviceInfo.length < 4) {
                break;
            }
            String deviceName = deviceInfo[0] + " @" + deviceInfo[1] + " Hz";
            if (isServer) {
                if (Integer.parseInt(deviceInfo[2]) > 0) {
                    cBox.addItem(deviceName);
                }
            } else {
                if (Integer.parseInt(deviceInfo[3]) > 0) {
                    cBox.addItem(deviceName);
                }
            }
        }
    }

    private static int getIndexDevice(String deviceName) {
        int removeIndex = deviceName.indexOf("@");
        String name = deviceName.substring(0, removeIndex - 1);
        String devices = libShareAudio.SA_ListAllAudioDevicesStr(0);
        String[] deviceList = devices.split("\n");
        for (int i = 0; i < deviceList.length; i++) {
            String[] deviceInfo = deviceList[i].split(",");
            if (deviceInfo[0].equals(name)) {
                return Integer.parseInt(deviceInfo[4]);
            }
        }
        return -1;
    }

    private static boolean checkIsNotEmpty(ShareAudioUI ui) {
        if (ui.hostField.getText().equals("") || ui.portField.getText().equals("")) {
            JFrame frame = new JFrame();
            JOptionPane.showMessageDialog(frame, "Host and Port can't be empty", "Error", JOptionPane.ERROR_MESSAGE);
            return false;
        }
        return true;
    }

    public static void listenButton(libShareAudio lShareAudio, ShareAudioUI ui) {
        ui.startButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                switch (ui.startButton.getText()) {
                    case "Start Server":
                    case "Connect to Server":
                        if (checkIsNotEmpty(ui)) {
                            Integer device = getIndexDevice(ui.comboBox.getSelectedItem().toString());
                            String host = ui.hostField.getText();
                            lShareAudio.conn = libShareAudio.SA_Setup(device, host, 9950, 0, 2, -1, 2048, -1);
                        } else {
                            break;
                        }
                        if(ui.clientRadio.isSelected()) {
                            libShareAudio.SA_SetMode(lShareAudio.conn, 1);
                        }
                        else{
                            libShareAudio.SA_SetMode(lShareAudio.conn, 0);
                        }
                        libShareAudio.SA_Init(lShareAudio.conn);
                        char[] passwordChars = ui.passwordField.getPassword();
                        if (passwordChars.length > 0) {
                            String password = new String(passwordChars);
                            libShareAudio.SA_SetKey(lShareAudio.conn, password);
                        }
                        if (ui.clientRadio.isSelected()) {
                            libShareAudio.SA_Client(lShareAudio.conn);
                        } else {
                            libShareAudio.SA_Server(lShareAudio.conn);
                        }
                        break;
                    case "Stop":
                        if (libShareAudio.SA_GetWavFilePtr(lShareAudio.conn) != 0) {
                            libShareAudio.SA_CloseWavRecord(lShareAudio.conn);
                        }
                        libShareAudio.SA_Close(lShareAudio.conn);
                        ui.recordButton.setText("Start Record");
                        lShareAudio.conn = 0;
                        break;
                    default:
                        break;
                }
            }
        });
    }

    public static void listenSlider(libShareAudio lShareAudio, ShareAudioUI ui) {
        ui.volumSlider.addChangeListener(new ChangeListener() {
            @Override
            public void stateChanged(ChangeEvent e) {
                if (lShareAudio.conn != 0) {
                    float vol = (float) ui.volumSlider.getValue() / 100;
                    libShareAudio.SA_SetVolumeModifier(vol, lShareAudio.conn);
                }
            }
        });
    }

    public static void listenButtonRecord(libShareAudio lShareAudio, ShareAudioUI ui) {
        ui.recordButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                if (lShareAudio.conn != 0) {
                    if (libShareAudio.SA_GetWavFilePtr(lShareAudio.conn) != 0) {
                        libShareAudio.SA_CloseWavRecord(lShareAudio.conn);
                        ui.recordButton.setText("Start Record");
                        recordAsStarted = false;
                        ui.recordTime.setText("");
                    } else {
                        libShareAudio.SA_InitWavRecord(lShareAudio.conn, "output.wav");
                        ui.recordButton.setText("Stop Record");
                        initRecord = Instant.now();
                        recordAsStarted = true;
                    }
                }
            }
        });
    }

    public static void main(String[] args) {
        ShareAudioUI ui = new ShareAudioUI();
        ui.recordButton.setEnabled(false);
        ui.clientRadio.setSelected(true);
        libShareAudio.SA_TestDLL();
        libShareAudio.SA_SetLogCONSOLE(1);
        Boolean contextChangedA = true;
        Boolean contextChangedB = true;
        Boolean contextChangedC = true;
        Boolean contextChangedD = true;
        libShareAudio lShareAudio = new libShareAudio();
        listenButton(lShareAudio, ui);
        listenSlider(lShareAudio, ui);
        listenButtonRecord(lShareAudio, ui);
        while (ui.frame.isVisible()) {
            if (lShareAudio.conn == 0) {
                if (contextChangedD) {
                    contextChangedC = true;
                    contextChangedD = true;
                    ui.comboBox.setEnabled(true);
                    ui.serverRadio.setEnabled(true);
                    ui.clientRadio.setEnabled(true);
                    ui.hostField.setEnabled(true);
                    ui.portField.setEnabled(true);
                    ui.recordButton.setEnabled(false);
                    ui.passwordField.setEnabled(true);
                }
                if (ui.serverRadio.isSelected()) {
                    contextChangedB = true;
                    ui.startButton.setText("Start Server");
                    if (contextChangedA) {
                        fillComboBox(ui.comboBox, true);
                        contextChangedA = false;
                    }
                } else {
                    contextChangedA = true;
                    ui.startButton.setText("Connect to Server");
                    if (contextChangedB) {
                        fillComboBox(ui.comboBox, false);
                        contextChangedB = false;
                    }
                }
            } else {
                if (contextChangedC) {
                    contextChangedD = true;
                    ui.startButton.setText("Stop");
                    ui.comboBox.setEnabled(false);
                    ui.serverRadio.setEnabled(false);
                    ui.clientRadio.setEnabled(false);
                    ui.hostField.setEnabled(false);
                    ui.portField.setEnabled(false);
                    ui.recordButton.setEnabled(true);
                    ui.passwordField.setEnabled(false);
                }
                contextChangedC = false;
            }
            if (recordAsStarted) {
                Instant now = Instant.now();
                Duration duration = Duration.between(initRecord, now);
                long secondsT = duration.getSeconds();
                long hours = secondsT / 3600;
                secondsT = secondsT - (hours * 3600);
                long minutes = secondsT / 60;
                secondsT = secondsT - (minutes * 60);
                long seconds = secondsT;
                ui.recordTime.setText(String.format("%02d:%02d:%02d", hours, minutes, seconds));
            }
            try {
                Thread.sleep(50);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }
}
