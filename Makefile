OBJ := vm guest.o payload.o vm.o

.PHONY: all
all: vm

vm: vm.o guest.o
	$(CC) $(CFLAGS) $^ -o $@

guest.o: guest.ld payload.o
	$(LD) -T $< -o $@

.PHONY: clean
clean:
	rm -rf $(OBJ)
