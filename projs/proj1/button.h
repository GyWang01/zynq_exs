#ifndef _BUTTON_H
#define _BUTTON_H

#define GPIO_BTN_LEFT 962  // 左键对应的GPIO编号
#define GPIO_BTN_RIGHT 961 // 右键对应的GPIO编号

int setup_gpio(int gpio);
int read_gpio(int gpio);
void send_mouse_button_event(int fd, int button, int value);
void test_send_button(int uinput_fd);


#endif