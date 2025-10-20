__attribute__((section(".user_func")))
void main_app_a() {
    *((volatile int *) 0x80200000) = 0x1234; // new!
    for (;;);    
}

__attribute__((section(".user_func")))
void main_app_b(){
    for (;;);    
}