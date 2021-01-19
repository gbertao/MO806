SECTIONS
{
  .guest : {
    guest_start = .;
    payload.o(.text)
    guest_end = .;
  }
}
