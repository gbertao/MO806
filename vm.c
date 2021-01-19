#include <assert.h>
#include <fcntl.h>
#include <linux/kvm.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


struct kvm {
    int sys_fd;
    int vm_fd;
    int api_ver;
    char *mem;
};

struct vcpu {
    int fd;
    int mmap_size;
    struct kvm_run *kvm_run;
};

extern const unsigned char guest_start[], guest_end[];

void kvm_init(struct kvm *kvm) {
    struct kvm_userspace_memory_region mem_reg;
    int ret;

    kvm->sys_fd = open("/dev/kvm", O_RDWR);
    assert(kvm->sys_fd >= 0);

    kvm->api_ver = ioctl(kvm->sys_fd, KVM_GET_API_VERSION, 0);
    assert(kvm->api_ver == 12);

    kvm->vm_fd = ioctl(kvm->sys_fd, KVM_CREATE_VM, (unsigned long)0);
    assert(kvm->vm_fd >= 0);

    ret = ioctl(kvm->vm_fd, KVM_SET_TSS_ADDR, 0xfffbd000);
    assert(ret >= 0);

    kvm->mem = mmap(NULL, 0x200000, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    assert(kvm->mem != MAP_FAILED);

    madvise(kvm->mem, 0x200000, MADV_MERGEABLE);

    mem_reg.slot = 0;
    mem_reg.flags = 0;
    mem_reg.guest_phys_addr = 0;
    mem_reg.memory_size = 0x200000;
    mem_reg.userspace_addr = (unsigned long)kvm->mem;
    ret = ioctl(kvm->vm_fd, KVM_SET_USER_MEMORY_REGION, &mem_reg);
    assert(ret >= 0);
}

void kvm_del(struct kvm *kvm) {
    int ret;

    ret = munmap(kvm->mem, 0x200000);
    assert(ret >= 0);

    ret = close(kvm->vm_fd);
    assert(ret >= 0);

    ret = close(kvm->sys_fd);
    assert(ret >= 0);
}

void vcpu_init(struct kvm *kvm, struct vcpu *vcpu) {

    vcpu->fd = ioctl(kvm->vm_fd, KVM_CREATE_VCPU, 0);
    assert(vcpu->fd >= 0);

    vcpu->mmap_size = ioctl(kvm->sys_fd, KVM_GET_VCPU_MMAP_SIZE, 0);
    assert(vcpu->mmap_size > 0);

    vcpu->kvm_run = mmap(NULL, vcpu->mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, vcpu->fd, 0);
    assert(vcpu->kvm_run != MAP_FAILED);
}

void vcpu_del(struct vcpu *vcpu) {
    int ret;

    ret = munmap(vcpu->kvm_run, vcpu->mmap_size);
    assert(ret >= 0);

    ret = close(vcpu->fd);
    assert(ret >= 0);
}

int kvm_run(struct kvm *kvm, struct vcpu *vcpu) {
    struct kvm_regs regs;
    struct kvm_sregs sregs;
    int cont = 1, ret;

    // Setup Regs
    ret = ioctl(vcpu->fd, KVM_GET_SREGS, &sregs);
    assert(ret >= 0);

    sregs.cs.base = 0;
    sregs.cs.selector = 0;

    ret = ioctl(vcpu->fd, KVM_SET_SREGS, &sregs);
    assert(ret >= 0);

    memset(&regs, 0, sizeof(regs));
    regs.rip = 0;
    regs.rsp = 0xd0;
    regs.rflags = 0x2;
    ret = ioctl(vcpu->fd, KVM_SET_REGS, &regs);
    assert(ret >= 0);

    // Load Guest
    memcpy(kvm->mem, guest_start, guest_end - guest_start);

    // Run
    int must, tgt;
    while (cont) {
        ret = ioctl(vcpu->fd, KVM_RUN, NULL);
        assert(ret >= 0);
        switch (vcpu->kvm_run->exit_reason) {
            case KVM_EXIT_HLT:
                ret = ioctl(vcpu->fd, KVM_GET_REGS, &regs);
                return regs.rbx;
            case KVM_EXIT_IO:
                ret = ioctl(vcpu->fd, KVM_GET_REGS, &regs);
		        if (vcpu->kvm_run->io.port == 0x50) {
			        must = regs.rip + regs.rsi;
		        } else if (vcpu->kvm_run->io.port == 0x51) {
                    tgt = regs.rsi;
                    if (tgt != must) {
                        printf("Malicius ROP detected\n");
                    }
		        }
                break;
            default:
                cont = 0;
                break;
        }
    }

    return -1;
}

int main(void) {
    struct kvm kvm;
    struct vcpu vcpu;
    int ret = 0;

    // Init
    kvm_init(&kvm);

    vcpu_init(&kvm, &vcpu);

    // Run
    ret = kvm_run(&kvm, &vcpu);

    // Cleanup
    vcpu_del(&vcpu);

    kvm_del(&kvm);

    return ret;
}
