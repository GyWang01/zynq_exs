CFLAGS = -Wall -g
OBJ = mouse_button.o mouse_location.o uinput_main.o
DEPS = button.h gyro.h
TARGET = uinput

# 规则用来生成目标文件
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# 链接所有目标文件生成最终可执行文件
$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

# 伪目标，用于清理项目
.PHONY: clean

clean:
	rm -f $(OBJ) $(TARGET)
