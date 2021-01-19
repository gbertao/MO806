OBJ := vm guest.o payload.o vm.o rop.o

.PHONY: all run
all: vm

run: vm
	./vm
	./vm -r

vm: vm.o guest.o
	$(CC) $(CFLAGS) $^ -o $@

guest.o: guest.ld payload.o rop.o
	$(LD) -T $< -o $@

.PHONY: clean
clean:
	rm -rf $(OBJ)
