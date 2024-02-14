import javax.swing.*;
import javax.swing.border.EmptyBorder;
import java.awt.*;

public class ShareAudioUI {
    public JFrame frame;
    public JPanel panel;
    public JLabel hostLabel;
    public JTextField hostField;
    public JLabel portLabel;
    public JTextField portField;
    public JPanel radioPanel;
    public JRadioButton serverRadio;
    public JRadioButton clientRadio;
    public ButtonGroup radioGroup;
    public JComboBox<String> comboBox;
    public JButton startButton;
    public JSlider volumSlider;
    public JLabel passwordLabel;
    public JPasswordField passwordField;
    public JButton recordButton;
    public JLabel recordTime;

    public ShareAudioUI() {
        frame = new JFrame("Share Audio");
        frame.setSize(400, 300);
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

        panel = new JPanel();
        panel.setLayout(new GridLayout(6, 2, 10, 10));

        hostLabel = new JLabel("Host:");
        hostField = new JTextField();
        panel.add(hostLabel);
        panel.add(hostField);

        portLabel = new JLabel("Port:");
        portField = new JTextField();
        portField.setText("9950");
        panel.add(portLabel);
        panel.add(portField);

        radioPanel = new JPanel();
        serverRadio = new JRadioButton("Server");
        clientRadio = new JRadioButton("Client");
        radioPanel.add(serverRadio);
        radioPanel.add(clientRadio);
        radioGroup = new ButtonGroup();
        radioGroup.add(serverRadio);
        radioGroup.add(clientRadio);
        panel.add(radioPanel);

        comboBox = new JComboBox<String>();
        panel.add(comboBox);

        startButton = new JButton("Start");
        panel.add(startButton);

        volumSlider = new JSlider(JSlider.HORIZONTAL, 0, 100, 50);
        panel.add(volumSlider);

        passwordLabel = new JLabel("Password:");
        passwordField = new JPasswordField();
        panel.add(passwordLabel);
        panel.add(passwordField);

        recordButton = new JButton("Start Record");
        panel.add(recordButton);

        recordTime = new JLabel("");
        panel.add(recordTime);

        panel.setBorder(new EmptyBorder(10, 10, 10, 10));

        frame.add(panel);

        frame.setVisible(true);
    }
}
