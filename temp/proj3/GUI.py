from __future__ import print_function
import sys
import os
from PyQt5.QtWidgets import QApplication, QWidget, QPushButton, QVBoxLayout, QLabel, QHBoxLayout
from PyQt5.QtCore import QTimer, Qt

class LEDControl(QWidget):
    def __init__(self):
        super(LEDControl, self).__init__()

        self.initUI()
        self.led_status = [0, 0, 0]
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.update_led_status)
        self.timer.start(1000)  # Update status every second for real-time update

    def initUI(self):
        vbox = QVBoxLayout()

        self.status_labels = []
        self.led_buttons = []

        for i in range(3):
            hbox = QHBoxLayout()
            label = QLabel(u"LED %d Status: OFF" % i)
            self.status_labels.append(label)
            button = QPushButton(u"Toggle LED %d" % i)
            button.clicked.connect(self.toggle_led_factory(i))
            self.led_buttons.append(button)
            hbox.addWidget(label)
            hbox.addWidget(button)
            vbox.addLayout(hbox)

        self.flow_button = QPushButton(u"Start Flow LEDs")
        self.flow_button.clicked.connect(self.toggle_flow_leds)
        vbox.addWidget(self.flow_button)

        self.setLayout(vbox)
        self.setWindowTitle(u'LED Control')
        self.show()

    def toggle_led_factory(self, led_index):
        def toggle_led():
            led_path = "/dev/led%d" % led_index
            try:
                with open(led_path, 'r') as f:
                    current_status = f.read().strip()
                new_status = '0' if current_status == '0' else '1'
                with open(led_path, 'w') as f:
                    f.write(new_status)
                print("Toggled LED %d to %s" % (led_index, 'OFF' if new_status == '0' else 'ON'))
            except IOError as e:
                print("Failed to toggle LED %d: %s" % (led_index, e))
            self.update_led_status()
        return toggle_led

    def update_led_status(self):
        for i in range(3):
            led_path = "/dev/led%d" % i
            try:
                with open(led_path, 'r') as f:
                    current_status = f.read().strip()
                self.led_status[i] = current_status
                self.status_labels[i].setText(u"LED %d Status: %s" % (i, 'ON' if current_status == '0' else 'OFF'))
                print("LED %d Status updated to: %s" % (i, 'ON' if current_status == '0' else 'OFF'))
            except IOError as e:
                print("Failed to read status for LED %d: %s" % (i, e))

    def toggle_flow_leds(self):
        if self.timer.isActive():
            self.timer.stop()
            self.flow_button.setText(u"Start Flow LEDs")
            os.system('killall flow_leds.o')
            print("Stopped flow LEDs")
        else:
            self.timer.start(1000)  # Update status every second
            self.flow_button.setText(u"Stop Flow LEDs")
            os.system('/mnt/proj2/led/flow_leds.o &')
            print("Started flow LEDs")

    def mousePressEvent(self, event):
        if event.button() == Qt.LeftButton:
            print("Mouse clicked at ({}, {})".format(event.x(), event.y()))

    def mouseMoveEvent(self, event):
        print("Mouse moved to ({}, {})".format(event.x(), event.y()))

    def mouseReleaseEvent(self, event):
        if event.button() == Qt.LeftButton:
            print("Mouse released at ({}, {})".format(event.x(), event.y()))

if __name__ == '__main__':
    app = QApplication(sys.argv)
    ex = LEDControl()
    sys.exit(app.exec_())
