SECTIONS
{
  .guest 0 : {
    guest_start = .;
    payload.o(.text)
    guest_end = .;
  }
  .rop 0 : AT(LOADADDR(.guest)+SIZEOF(.guest)) {
    rop_start = .;
    rop.o(.text)
    rop_end = .;
  }
}
