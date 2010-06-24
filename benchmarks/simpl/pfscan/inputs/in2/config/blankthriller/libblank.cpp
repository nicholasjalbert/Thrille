PY**INCLUDE

void BlankHandler::AfterInitialize() {

}

void BlankHandler::ThreadStart(void * (* start_routine) (void *),
        void * arg) {

} 

void BlankHandler::ThreadFinish(void * (* start_routine) (void *),
        void * status) {

} 

bool BlankHandler::BeforeCreate(void * ret_addr,
        pthread_t * param0, 
        const pthread_attr_t * param1,
        void *(* param2)(void *),
        void * param3,
        ThreadInfo * & param4) {
    param4 = new TLSBlankData();
    return true;

}

int BlankHandler::SimulateCreate(void * ret_addr,
        pthread_t * param0,
        const pthread_attr_t * param1,
        void *(* param2)(void *),
        void * param3,
        ThreadInfo * & param4) {

    return 0;

}

void BlankHandler::AfterCreate(void * ret_addr,
        int ret_val,
        pthread_t * param0,
        const pthread_attr_t * param1,
        void *(* param2)(void *),
        void * param3) { }





