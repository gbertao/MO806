# UNICAMP - MO806 - Tópicos em Sistemas Operacionais

Esse repositório contém o meu projeto de MO806. O projeto trata de uma forma
de detecção de ROP assistida por hardware utilizando a KVM.

## Resumo
Ao implementar um hypervisor é possível fazer hypercalls antes da execução das
instruções de `call` e de `ret`.  Antes do `call`, o `rip` é salvo em uma
variável do hypervisor.  Antes do `ret`, o endereço no topo da pilha é
comparado com a variável. Se forem diferentes, então stack-overflow foi aplicado
com o intuito de disparar uma cadeia de ROP.


## Build
É necessário ter um processador compatível com VT-x ou AMD-V.
A compatibilidade pode ser checada usando:
```
egrep "(vmx|svm)" /proc/cpuinfo
```
Se a maquina for compatível. A opção `vmx` ou `svm` deve aparecer.

A KVM é necessária para executar o programa. Instale usando:
```
sudo apt-get install qemu-kvm
```

Para compilar o projeto utilize o `Makefile`:
```
make vm
```

## Execução
É possível executar normalmente pelo console usando:
```
./vm
```
ou pelo Makefile usando:
```
make run
```

### Argumento (-r)
O argumento `-r` é opcional, quando presente ele carrega o código ROP no
hypervisor.  Por padrão, o código sem ROP é carregado.
```
./vm -r
```

## Saída
A execução padrão seta o registrador `rbx` para `10`. Enquanto isso, a execução
pelo ROP seta o `rbx` para `0`
