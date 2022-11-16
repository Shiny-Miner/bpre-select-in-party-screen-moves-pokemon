.gba
.thumb
.open "BPRE.gba","build/rom.gba", 0x08000000
//---------------------------------------------------

.org 0x0811fb28
.area 0x7C, 0xFF
    push {lr}
    ldr r3, =Task_HandleChooseMonInput|1  
    bl bx_r3_p
    pop {pc}

bx_r3_p:
    bx r3
.pool
.endarea

.org 0x0811fefc
.area 0x96, 0xFF
    push {lr}
    ldr r3, =PartyMenuButtonHandler|1  
    bl bx_r3_pp
    pop {pc}

bx_r3_pp:
    bx r3
.pool
.endarea

.align 4
.org insertinto
.importobj "build/linked.o"
.close
