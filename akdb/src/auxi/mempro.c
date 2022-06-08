/**
@file mempro.c Implementation of the memory wrappers and debug mode of
Kalashnikov DB.
*/
/*
debmod - "debug mod" za KalashnikovDB
Copyright(c) 2014 Mislav Bozicevic <misbozice@foi.hr> and Marin Rukavina
<marrukavi@foi.hr>

Permission to use, copy, modify, and / or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS.IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include "mempro.h"
/**
* @author Marin Rukavina, Mislav Bozicevic
* @param ds debug mode state
* @param message string to print
* @brief Function prints debug message [private function]
* @return void
*/
void AK_debmod_d(AK_debmod_state* ds, const char *message){
#if AK_DEBMOD_ON & AK_DEBMOD_PRINT
    clock_t t;
    assert(ds != NULL && ds->init == 1);
    if (ds->print == 0){
        return;
    }
    t = clock();
    fprintf(stderr, "[DEBMOD %p] %ld %s", (void*)ds, t, message);
    fflush(stderr);
#endif
}

/**
* @author Marin Rukavina, Mislav Bozicevic
* @param ds debug mode state
* @param format format string like printf
* @brief Function prints debug message [private function]
* @return void
*/
void AK_debmod_dv(AK_debmod_state* ds, const char *format, ...){
#if AK_DEBMOD_ON & AK_DEBMOD_PRINT
    va_list arg;
    clock_t t;
    assert(ds != NULL && ds->init == 1);
    if (ds->print == 0){
        return;
    }
    t = clock();
    fprintf(stderr, "[DEBMOD %p] %ld ", (void*)ds, t);
    va_start(arg, format);
    vfprintf(stderr, format, arg);
    va_end(arg);
    fflush(stderr);
#endif
}

#ifdef __linux__
/**
* @author Marin Rukavina, Mislav Bozicevic
* @brief Signal handler for SIGSEGVs [private function]
* @return void
*/
static void AK_debmod_signal_callback(int sig, siginfo_t* si, void* unused) {
    if (AK_DEBMOD_STATE != NULL && AK_DEBMOD_STATE->init == 1) {
        uint32_t i;
        uint8_t mprotect_sigsegv; /* Indicates Whether this SIGSEGV was caused
                                  * by writing to an mprotected page, in which
                                  * case the program need not fail. Otherwise
                                  * this is a classic segfault and should be
                                  * treated as such.
                                  */
        mprotect_sigsegv = 0;

        AK_debmod_enter_critical_sec(AK_DEBMOD_STATE);
        for (i = 0; i < AK_DEBMOD_PAGES_NUM; ++i) {
            /* find the page that this memory belongs to, if any */
            if (AK_DEBMOD_STATE->page[i] != 0 &&
                (uintptr_t*)AK_DEBMOD_STATE->page[i]
                    <= (uintptr_t*)si->si_addr &&
                (uintptr_t*)AK_DEBMOD_STATE->page[i] + AK_DEBMOD_STATE->real[i]
                    >= (uintptr_t*)si->si_addr) {
                /* if the memory was protected AK_mem_write_protect, this
                should fail  */
                assert(AK_DEBMOD_STATE->protected[i] == 0);
                AK_debmod_dv(
                    AK_DEBMOD_STATE,
                    "%p marked dirty [%d/%d] (%s)\n",
                    AK_DEBMOD_STATE->page[i],
                    i,
                    AK_DEBMOD_PAGES_NUM,
                    AK_debmod_func_get_name(
                        AK_DEBMOD_STATE,
                        AK_DEBMOD_STATE->fstack_items[
                            AK_DEBMOD_STATE->fstack_size - 1]
                    )
                );

                /* unlock the page for RW access */
                assert(mprotect(AK_DEBMOD_STATE->page[i],
                    AK_DEBMOD_STATE->real[i], PROT_READ | PROT_WRITE) == 0);

                AK_DEBMOD_STATE->dirty[i] = 1;
                AK_DEBMOD_STATE->unlocked[i] = 1;
                mprotect_sigsegv = 1;
            }
        }
        AK_debmod_leave_critical_sec(AK_DEBMOD_STATE);

        /* end program execution if the sigsegv was caused by non-protected
        memory access */
	if(mprotect_sigsegv == 0){
		printf("************* Real SIGSEGV occured *************\n");
                printf("This means a memory address which is not under\n");
		printf("our control has been accessed somewhere in your\n");
		printf("code. Please use AK_calloc, AK_free, etc.\n");
		printf("You've got some bugfixing to do :(\n");
                AK_print_active_functions();
		AK_print_function_uses();
		printf("Probable current function: %s\n",
        		AK_debmod_func_get_name(AK_DEBMOD_STATE,
			AK_DEBMOD_STATE->fstack_items[
			AK_DEBMOD_STATE->fstack_size - 1]));
        	assert(mprotect_sigsegv);
        }
    }
}

/**
* @author Marin Rukavina, Mislav Bozicevic
* @param ds debug mode state
* @brief To be called in crude intervals for linux WriteWatch [public function]
* @return void
*/
void AK_debmod_sample_state(AK_debmod_state* ds) {
    uint32_t i;
    assert(ds != NULL && ds->init == 1);

    AK_debmod_enter_critical_sec(ds);
    for (i = 0; i < AK_DEBMOD_PAGES_NUM; ++i) {
        if (ds->unlocked[i] == 1) {
            /* protect the memory for writing so that any unsolicited writes
            can be caught and logged */
            assert(mprotect(ds->page[i], AK_DEBMOD_STATE->real[i],
                PROT_READ) == 0);
            ds->unlocked[i] = 0;
        }
    }
    AK_debmod_leave_critical_sec(ds);
}
#endif

/**
* @author Marin Rukavina, Mislav Bozicevic
* @param ds debug mode state
* @brief Reserves ds for use [private function]
* @return void
*/
void AK_debmod_enter_critical_sec(AK_debmod_state* ds){
    assert(ds != NULL && ds->init == 1);
#ifdef _WIN32
    EnterCriticalSection(&ds->critical_section);
#endif
#ifdef __linux__
    pthread_mutex_lock(&AK_debmod_critical_section);
#endif
    while (ds->ready != 1); /* wait loop */
    ds->ready = 0;
#ifdef _WIN32
    LeaveCriticalSection(&ds->critical_section);
#endif
#ifdef __linux__
    pthread_mutex_unlock(&AK_debmod_critical_section);
#endif
}

/**
* @author Marin Rukavina, Mislav Bozicevic
* @param ds debug mode state
* @brief Makes ds available [private function]
* @return void
*/
void AK_debmod_leave_critical_sec(AK_debmod_state* ds){
    ds->ready = 1; /* AK_DEBMOD_STATE can be used again */
}

/**
* @author Marin Rukavina, Mislav Bozicevic
* @brief Initializes debug mode structure [public function]
* @return initialized debug mode state
*/
AK_debmod_state* AK_debmod_init(void){
#ifdef _WIN32
    SYSTEM_INFO system_info;
#endif
#ifdef __linux__
    struct sigaction sa;
#endif
    int32_t i, j;
    AK_debmod_state* ds;
    /*
    ds = (struct AK_debmod_state*)
      calloc(1, sizeof(struct AK_debmod_state));
    */
    ds = NEW(AK_debmod_state, 1);
    if (ds == NULL){
        return NULL;
    }
#ifdef _WIN32
    GetSystemInfo(&system_info);
    ds->page_size = (uint32_t)system_info.dwPageSize;
    if (InitializeCriticalSectionAndSpinCount(
        &ds->critical_section, 0x00000400) == 0){
        free(ds);
        return NULL;
    }
#endif
#ifdef __linux__
    ds->page_size = (uint32_t)getpagesize();

    /* postavi handler za SIGSEGV */
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = AK_debmod_signal_callback;

    if(AK_DEBMOD_ON || 1) {
        if (sigaction(SIGSEGV, &sa, NULL) == -1) {
            return NULL;
        }
    }
#endif
    for (i = 0; i < AK_DEBMOD_PAGES_NUM; ++i){
        ds->page[i] = NULL;
        ds->used[i] = 0;
#ifdef __linux__
        ds->protected[i] = 0;
        ds->unlocked[i] = 0;
#endif
    }
    ds->init = 1;
    ds->last_function_id = -1;
    for (i = 0; i < AK_DEBMOD_MAX_FUNCTIONS; ++i){
        for (j = 0; j < AK_DEBMOD_MAX_FUNCTIONS; ++j){
            ds->func_used_by[i][j] = -1; /* -1 = not used */
        }
    }
    for (i = 0; i < AK_DEBMOD_MAX_FUNCTIONS; ++i){
        ds->alloc_owner[i] = -1;
    }
    for (i = 0; i < AK_DEBMOD_STACKSIZE; ++i){
        ds->fstack_items[i] = -1;
    }
    ds->fstack_size = 1;
    ds->print = AK_DEBMOD_PRINT;
    ds->ready = 1;
    AK_debmod_d(ds, "Initialized.\n");
    return ds;
}

/**
* @author Marin Rukavina, Mislav Bozicevic
* @param ds debug mode state
* @brief Destroy debug mode state (call before main() exit) [public function]
* @return void
*/
void AK_debmod_die(AK_debmod_state* ds){
    int32_t i;
    if (ds == NULL){
        return;
    }
    if (ds->init != 1){
        return;
    }
    AK_debmod_d(ds, "So long, and thanks for all the fish.\n");
    AK_debmod_enter_critical_sec(ds);
    for (i = 0; i < AK_DEBMOD_PAGES_NUM; ++i){
        if (ds->used[i] == 1){
#ifdef _WIN32
            assert(VirtualFree(ds->page[i], 0, MEM_RELEASE) != 0);
#endif
#ifdef __linux__
            free(ds->page[i]);
#endif
            ds->page[i] = NULL;
        }
    }
#ifdef _WIN32
    DeleteCriticalSection(&ds->critical_section);
#endif
    free(ds);
    ds = NULL;
}

/**
* @author Marin Rukavina, Mislav Bozicevic
* @param ds debug mode state
* @param size in bytes to allocate
* @brief Allocates memory [private function]
* @return pointer to allocated memory or NULL
*/
void* AK_debmod_calloc(AK_debmod_state* ds, uint32_t size){
    int32_t i, selected_spot;
    uint32_t real_alloc;
#ifdef __linux__
    int ma_ret;
#endif
    assert(ds != NULL && ds->init == 1);
#ifdef __linux__
    AK_debmod_sample_state(ds);
#endif
    if (size == 0){
        return NULL;
    }
    AK_debmod_enter_critical_sec(ds);
    selected_spot = -1;
    for (i = 0; i < AK_DEBMOD_PAGES_NUM; ++i){
        if ((ds->used[i] == 0) && (ds->page[i] == NULL)){
            selected_spot = i; /* unused page found */
            break;
        }
    }
    if ((AK_DEBMOD_PAGES_NUM - selected_spot) < 2){
        AK_debmod_dv(ds, "Low memory warning (%s).\n",
            AK_debmod_func_get_name(ds,
            ds->fstack_items[ds->fstack_size - 1]));
    }
    assert(selected_spot != -1); /* all allocations used */
    real_alloc =  /* size rounded to page size */
                  ds->page_size * ((size / ds->page_size) + 1);
#ifdef _WIN32
    ds->page[selected_spot] = (void *)VirtualAlloc(NULL,
        real_alloc, MEM_RESERVE | MEM_COMMIT | MEM_WRITE_WATCH,
        PAGE_READWRITE);
    if(ds->page[selected_spot] == NULL){
        AK_debmod_dv(ds, "alloc failed on [%d/%d] (%s)\n",
            selected_spot, AK_DEBMOD_PAGES_NUM,
            AK_debmod_func_get_name(ds,
                ds->fstack_items[ds->fstack_size - 1]));
        AK_debmod_leave_critical_sec(ds);
        return NULL;
    }
#endif
#ifdef __linux__
    ma_ret = posix_memalign(&ds->page[selected_spot], ds->page_size,
        real_alloc);
    if(ma_ret != 0){
        AK_debmod_dv(ds, "alloc failed on [%d/%d] (%s)\n",
            selected_spot, AK_DEBMOD_PAGES_NUM,
            AK_debmod_func_get_name(ds,
            ds->fstack_items[ds->fstack_size - 1]));
        AK_debmod_leave_critical_sec(ds);
        return NULL;
    }
#endif
    memset(ds->page[selected_spot], 0, size);
#ifdef __linux__
    assert(mprotect(ds->page[selected_spot], real_alloc, PROT_READ) == 0);
#endif
    ds->nomi[selected_spot] = size;
    ds->real[selected_spot] = real_alloc;
    ds->used[selected_spot] = 1;
    AK_debmod_dv(ds, "%p alloc [%d/%d] (%s)\n", ds->page[selected_spot],
        selected_spot, AK_DEBMOD_PAGES_NUM,
        AK_debmod_func_get_name(ds, ds->fstack_items[ds->fstack_size - 1]));
    ds->alloc_owner[selected_spot] = ds->fstack_items[ds->fstack_size - 1];
    ds->free_owner[selected_spot] = -1;
    AK_debmod_leave_critical_sec(ds);
    return ds->page[selected_spot];
}

/**
* @author Marin Rukavina, Mislav Bozicevic
* @param ds debug mode state
* @param memory
* @brief Frees memory allocated with debmod_alloc [private function]
* @return void
*/
void AK_debmod_free(AK_debmod_state* ds, void* memory){
    int32_t i, selected_spot;
    assert(ds != NULL && ds->init == 1);
#ifdef __linux__
    AK_debmod_sample_state(ds);
#endif
    if (memory == NULL){
        AK_debmod_dv(ds, "tried to free NULL (%s)\n",
            AK_debmod_func_get_name(ds,
            ds->fstack_items[ds->fstack_size - 1]));
        return;
    }
    AK_debmod_enter_critical_sec(ds);
    selected_spot = -1;
    for (i = 0; i < AK_DEBMOD_PAGES_NUM; ++i){
        if (memory == ds->page[i]){
            selected_spot = i;
            break;
        }
    }
    if (selected_spot == -1){
        AK_debmod_dv(ds, "%s tried to free %p which does not belong\n",
            AK_debmod_func_get_name(ds,
            ds->fstack_items[ds->fstack_size - 1]),
            memory);
        AK_debmod_leave_critical_sec(ds);
        return;
    }
#ifdef _WIN32
    assert(VirtualFree(ds->page[selected_spot], 0, MEM_RELEASE) != 0);
#endif
#ifdef __linux__
    assert(mprotect(AK_DEBMOD_STATE->page[i], AK_DEBMOD_STATE->real[i],
        PROT_READ | PROT_WRITE) == 0);
    free(ds->page[selected_spot]);
    ds->protected[selected_spot] = 0;
    ds->unlocked[selected_spot] = 0;
#endif
    ds->page[selected_spot] = NULL;
    ds->used[selected_spot] = 0;
    ds->nomi[selected_spot] = 0;
    ds->real[selected_spot] = 0;
    AK_debmod_dv(ds, "%p free [%d/%d] (%s)\n", memory,
        selected_spot, AK_DEBMOD_PAGES_NUM,
        AK_debmod_func_get_name(ds, ds->fstack_items[ds->fstack_size - 1]));
    ds->free_owner[selected_spot] = ds->fstack_items[ds->fstack_size - 1];
    AK_debmod_leave_critical_sec(ds);
}

/**
* @author Marin Rukavina, Mislav Bozicevic
* @param num number of elements
* @param size of element in bytes
* @brief Allocates memory (see calloc) [public function]
* @return allocated memory or NULL
*/
void* AK_calloc(size_t num, size_t size){
    if (AK_DEBMOD_STATE == NULL || AK_DEBMOD_STATE->init != 1){
        AK_DEBMOD_STATE = AK_debmod_init();
        if (AK_DEBMOD_STATE == NULL || AK_DEBMOD_STATE->init != 1){
            return NULL;
        }
    }
#if !AK_DEBMOD_ON
    return calloc(num, size);
#endif
    return AK_debmod_calloc(AK_DEBMOD_STATE, (uint32_t)(num * size));
}

/**
* @author Marin Rukavina, Mislav Bozicevic
* @param size of memory to allocate in bytes
* @brief Allocate memory (see malloc) [public function]
* @return allocated memory or NULL
*/
void* AK_malloc(size_t size){
#if !AK_DEBMOD_ON
    return malloc(size);
#endif
    return AK_calloc(1, size);
}

/**
* @author Marin Rukavina, Mislav Bozicevic
* @param ptr pointer to memory
* @brief Free memory at ptr (see free) [public function]
* @return void
*/
void AK_free(void* ptr){
#if !AK_DEBMOD_ON
	free(ptr);
	ptr = NULL;
    return;
#endif
    assert(AK_DEBMOD_STATE != NULL && AK_DEBMOD_STATE->init == 1);
    AK_debmod_free(AK_DEBMOD_STATE, ptr);
}

/**
* @author Marin Rukavina, Mislav Bozicevic
* @param ptr old memory
* @param size new size
* @brief Reallocates memory (see realloc) [public function]
* @return reallocated memory or NULL
*/
void* AK_realloc(void* ptr, size_t size){
    void *ret;
    size_t dest_size;
    int32_t i;
    uint32_t old_size;
#ifdef __linux__
    int32_t ret_i;
    int32_t ret_pos;
#endif
#if !AK_DEBMOD_ON
    return realloc(ptr, size);
#endif
    if (ptr == NULL){
        return AK_malloc(size);
    }
    if (size == 0){
        AK_free(ptr);
        return NULL;
    }
    ret = AK_malloc(size);
    if (ret == NULL){
        return NULL;
    }
    assert(AK_DEBMOD_STATE != NULL && AK_DEBMOD_STATE->init == 1);
    AK_debmod_enter_critical_sec(AK_DEBMOD_STATE);
    old_size = -1;
    for (i = 0; i < AK_DEBMOD_PAGES_NUM; ++i){
        if (ptr == AK_DEBMOD_STATE->page[i]){
            old_size = AK_DEBMOD_STATE->nomi[i];
            break;
        }
    }
    if (old_size == -1){
        AK_debmod_dv(AK_DEBMOD_STATE,
            "%s tried to realloc memory which does not belong\n",
            AK_debmod_func_get_name(AK_DEBMOD_STATE,
            AK_DEBMOD_STATE->fstack_items[
                AK_DEBMOD_STATE->fstack_size - 1]));
        AK_debmod_leave_critical_sec(AK_DEBMOD_STATE);
        AK_free(ret);
        return NULL;
    }
    dest_size = (size_t)old_size <= size ? (size_t)old_size : size;
#ifdef __linux__
    ret_pos = -1;
    for (ret_i = 0; ret_i < AK_DEBMOD_PAGES_NUM; ++ret_i) {
        /* get the page pointer for the return memory so it can be unlocked */
        if (ret == AK_DEBMOD_STATE->page[ret_i]) {
            ret_pos = ret_i;
        }
    }
    assert(ret_pos > -1);
    assert(mprotect(AK_DEBMOD_STATE->page[ret_pos],
        AK_DEBMOD_STATE->real[ret_pos], PROT_READ | PROT_WRITE) == 0);
#endif
    ret = memcpy(ret, (const void*)ptr, dest_size);
#ifdef __linux__
    assert(mprotect(AK_DEBMOD_STATE->page[ret_pos],
        AK_DEBMOD_STATE->real[ret_pos], PROT_READ) == 0);
#endif
    AK_debmod_leave_critical_sec(AK_DEBMOD_STATE);
    AK_free(ptr);
    AK_debmod_dv(AK_DEBMOD_STATE,
        "realloc %p (%d) -> %p (%d) (%s)\n",
        ptr, old_size, ret, size,
        AK_debmod_func_get_name(AK_DEBMOD_STATE,
        AK_DEBMOD_STATE->fstack_items[
            AK_DEBMOD_STATE->fstack_size - 1]));
    return ret;
}

/**
* @author Marin Rukavina, Mislav Bozicevic
* @param memory
* @brief Function write-protects memory [public function]
* @return void
*/
void AK_write_protect(void* memory){
#ifdef _WIN32
    DWORD old; /* dummy variable for VirtualProtect */
#endif
    int32_t i, selected_spot;
    assert(AK_DEBMOD_STATE != NULL && AK_DEBMOD_STATE->init == 1);

    AK_debmod_enter_critical_sec(AK_DEBMOD_STATE);
    selected_spot = -1;
    for (i = 0; i < AK_DEBMOD_PAGES_NUM; ++i){
        if (memory == AK_DEBMOD_STATE->page[i]){
            selected_spot = i;
            break;
        }
    }
    assert(selected_spot != -1);
#ifdef _WIN32
    assert(VirtualProtect(AK_DEBMOD_STATE->page[selected_spot],
        AK_DEBMOD_STATE->real[selected_spot], PAGE_READONLY, &old) != 0);
#endif
#ifdef __linux__
    /* assert(mprotect(memory, AK_DEBMOD_STATE->real[i], PROT_READ) == 0); */
    AK_DEBMOD_STATE->protected[selected_spot] = 1;
#endif

    AK_debmod_leave_critical_sec(AK_DEBMOD_STATE);
}

/**
* @author Marin Rukavina, Mislav Bozicevic
* @param memory
* @brief Function write-unprotects memory [public function]
* @return void
*/
void AK_write_unprotect(void* memory){
#ifdef _WIN32
    DWORD old; /* dummy variable for VirtualProtect */
#endif
    int32_t i, selected_spot;
    assert(AK_DEBMOD_STATE != NULL && AK_DEBMOD_STATE->init == 1);
    AK_debmod_enter_critical_sec(AK_DEBMOD_STATE);
    selected_spot = -1;
    for (i = 0; i < AK_DEBMOD_PAGES_NUM; ++i){
        if (memory == AK_DEBMOD_STATE->page[i]){
            selected_spot = i;
            break;
        }
    }
    assert(selected_spot != -1);
#ifdef _WIN32
    assert(VirtualProtect(AK_DEBMOD_STATE->page[selected_spot],
        AK_DEBMOD_STATE->real[selected_spot], PAGE_READWRITE, &old) != 0);
#endif
#ifdef __linux__
    /*assert(mprotect(memory, AK_DEBMOD_STATE->real[i],
        PROT_READ | PROT_WRITE) == 0);*/
    AK_DEBMOD_STATE->protected[selected_spot] = 0;
#endif
    AK_debmod_leave_critical_sec(AK_DEBMOD_STATE);
}

/**
* @author Marin Rukavina, Mislav Bozicevic
* @brief Marks pages dirty if there were writes between calls to this function
* @return void
*/
void AK_check_for_writes(void){
#ifdef _WIN32
    UINT ret;
    DWORD last_error;
    PVOID changed[AK_DEBMOD_MAX_WRITE_DETECTIONS];
    ULONG_PTR count;
    ULONG t;
    uint32_t j;
#endif
    int32_t i;
    assert(AK_DEBMOD_STATE != NULL && AK_DEBMOD_STATE->init == 1);
#ifdef _WIN32
    count = AK_DEBMOD_PAGES_NUM;
#endif
    AK_debmod_enter_critical_sec(AK_DEBMOD_STATE);
    for (i = 0; i < AK_DEBMOD_PAGES_NUM; ++i){
        if (AK_DEBMOD_STATE->used[i] != 0){
            AK_DEBMOD_STATE->dirty[i] = 0; /* forget previous state */
        }
    }
    for (i = 0; i < AK_DEBMOD_PAGES_NUM; ++i){
        if (AK_DEBMOD_STATE->used[i] != 0){
#ifdef _WIN32
            ret = GetWriteWatch(WRITE_WATCH_FLAG_RESET,
                AK_DEBMOD_STATE->page[i], AK_DEBMOD_STATE->real[i],
                changed, &count, &t);
            if (ret != 0){
                last_error = GetLastError();
                if (last_error == 0x57){ /* ERROR_INVALID_PARAMETER */
                    continue;            /* - ignored */
                }
                else{
                    AK_debmod_dv(AK_DEBMOD_STATE, "Win32 system error %d\n",
                        last_error);
                    break;
                }
            }
            for (j = 0; j < count; ++j){
                if (AK_DEBMOD_STATE->page[i] == changed[j]){
                    AK_DEBMOD_STATE->dirty[i] = 1;
                    AK_debmod_dv(AK_DEBMOD_STATE,
                        "%p marked dirty [%d/%d] (%s)\n",
                        AK_DEBMOD_STATE->page[i],
                        i, AK_DEBMOD_PAGES_NUM,
                        AK_debmod_func_get_name(AK_DEBMOD_STATE,
                        AK_DEBMOD_STATE->fstack_items[
                            AK_DEBMOD_STATE->fstack_size - 1]));
                }
            }
#endif
        }
    }
    AK_debmod_leave_critical_sec(AK_DEBMOD_STATE);
}

/**
* @author Marin Rukavina, Mislav Bozicevic
* @param ds debug mode state
* @param func_name function name [private function]
* @brief Returns function id for given func_name
* @return function id
*/
int32_t AK_debmod_func_id(AK_debmod_state* ds, const char *func_name){
    int32_t i;
    assert(ds != NULL && ds->init == 1);
    assert(ds->last_function_id < AK_DEBMOD_MAX_FUNCTIONS);
    assert(func_name != NULL);
    if (ds->last_function_id == -1){
        return -1;
    }
    for (i = 0; i <= ds->last_function_id; ++i){
        if (strncmp(func_name, ds->function[i], AK_DEBMOD_MAX_FUNC_NAME) == 0){
            return i;
        }
    }
    return -1;
}

/**
* @author Marin Rukavina, Mislav Bozicevic
* @param ds debug mode state
* @param function_id
* @brief Lookup function name [private function]
* @return function name for given function_id
*/
const char * AK_debmod_func_get_name(AK_debmod_state* ds,
    int32_t function_id){
    assert(ds != NULL && ds->init == 1);
    if (function_id < 0){
        return "unknown_function";
    }
    assert(function_id < AK_DEBMOD_MAX_FUNCTIONS);
    return ds->function[function_id];
}

/**
* @author Marin Rukavina, Mislav Bozicevic
* @param ds debug mode state
* @param func_name
* @brief Adds function name to list [private function]
* @return id for added function name
*/
int32_t AK_debmod_func_add(AK_debmod_state* ds, const char *func_name){
    assert(ds != NULL && ds->init == 1);
    assert(ds->last_function_id < AK_DEBMOD_MAX_FUNCTIONS);
    assert(func_name != NULL);
    ds->last_function_id += 1;
    strncpy(ds->function[ds->last_function_id], func_name,
        AK_DEBMOD_MAX_FUNC_NAME);
    return ds->last_function_id;
}

/**
* @author Marin Rukavina, Mislav Bozicevic
* @param ds debug mode state
* @param func_id function id
* @brief Push function id on stack [private function]
* @return void
*/
void AK_debmod_fstack_push(AK_debmod_state* ds, int32_t func_id){
    assert(ds != NULL && ds->init == 1);
    assert(ds->fstack_size < AK_DEBMOD_STACKSIZE); /* stack overflow */
    ds->fstack_items[ds->fstack_size++] = func_id;
}

/**
* @author Marin Rukavina, Mislav Bozicevic
* @param ds debug mode state
* @brief Pops function id from stack [private function]
* @return function id popped
*/
int32_t AK_debmod_fstack_pop(AK_debmod_state* ds){
    assert(ds != NULL && ds->init == 1);
    assert(ds->fstack_size > 0); /* stack underflow */
    return ds->fstack_items[--ds->fstack_size];
}

/**
* @author Marin Rukavina, Mislav Bozicevic
* @param ds debug mode state
* @param new_function_id
* @brief Sets current function [private function]
* @return void
*/
void AK_debmod_function_current(AK_debmod_state* ds, int32_t new_function_id){
    int32_t old;
    assert(ds != NULL && ds->init == 1);
    assert(new_function_id > -1);
    assert(new_function_id < AK_DEBMOD_MAX_FUNCTIONS);
    old = ds->fstack_items[ds->fstack_size - 1];
    if (old != -1){
        ds->func_used_by[old][new_function_id] = 1;
    }
    AK_debmod_fstack_push(ds, new_function_id);
}

/**
* @author Marin Rukavina, Mislav Bozicevic
* @param func_name function name as in source
* @param source_file file name where function is defined
* @param source_line line from which this function is called
* @brief Not for direct use (only with macro AK_PRO). Marks function prologue
* @return void
*/
void AK_debmod_function_prologue(const char *func_name,
    const char *source_file, int source_line){
    int32_t id;
    if (AK_DEBMOD_STATE == NULL || AK_DEBMOD_STATE->init != 1){
        AK_DEBMOD_STATE = AK_debmod_init();
        if (AK_DEBMOD_STATE == NULL || AK_DEBMOD_STATE->init != 1){
            return;
        }
    }
    AK_debmod_enter_critical_sec(AK_DEBMOD_STATE);
    id = AK_debmod_func_id(AK_DEBMOD_STATE, func_name);
    if (id == -1){
        id = AK_debmod_func_add(AK_DEBMOD_STATE, func_name);
    }

    AK_debmod_function_current(AK_DEBMOD_STATE, id);

    AK_debmod_leave_critical_sec(AK_DEBMOD_STATE);
}

/**
* @author Marin Rukavina, Mislav Bozicevic
* @param func_id calling function id
* @brief print debmod information on function [private function]
* @return void
*/
void AK_debmod_log_memory_alloc(int32_t func_id){
#if AK_DEBMOD_ON & AK_DEBMOD_PRINT
    uint8_t has_allocs;
    int32_t i;
    assert(AK_DEBMOD_STATE != NULL && AK_DEBMOD_STATE->init == 1);
    assert(func_id > -1);
    assert(func_id < AK_DEBMOD_MAX_FUNCTIONS);
    has_allocs = 0;
    for (i = 0; i < AK_DEBMOD_PAGES_NUM; ++i){
        if (AK_DEBMOD_STATE->used[i] != 0){
            if (AK_DEBMOD_STATE->alloc_owner[i] ==
                func_id){
                has_allocs = 1;
                printf("%s allocated %d bytes in %p [%d/%d] ",
                    AK_debmod_func_get_name(AK_DEBMOD_STATE,
                        func_id),
                        AK_DEBMOD_STATE->nomi[i],
                        AK_DEBMOD_STATE->page[i],
                        i, AK_DEBMOD_PAGES_NUM
                    );
                if (AK_DEBMOD_STATE->free_owner[i] ==
                    func_id){
                    printf("(did not forget to free)\n");
                }
                else{
                    printf("(did forget to free)\n");
                }
            }
        }
    }
    if (has_allocs == 0){
        printf("Function %s did not allocate memory.\n",
            AK_debmod_func_get_name(AK_DEBMOD_STATE, func_id));
    }
#endif
}

/**
* @author Marin Rukavina, Mislav Bozicevic
* @param func_name function name as in source
* @param source_file file name where function is defined
* @param source_line line from which this function is called
* @brief Not for direct use (only with macro AK_EPI). Marks function epilogue
* @return void
*/
void AK_debmod_function_epilogue(const char *func_name,
    const char *source_file, int source_line){
    int32_t id;
    if (AK_DEBMOD_STATE == NULL || AK_DEBMOD_STATE->init != 1){
        AK_DEBMOD_STATE = AK_debmod_init();
        if (AK_DEBMOD_STATE == NULL || AK_DEBMOD_STATE->init != 1){
            return;
        }
    }
    AK_debmod_enter_critical_sec(AK_DEBMOD_STATE);
    id = AK_debmod_func_id(AK_DEBMOD_STATE, func_name);
    assert(id != -1);

    AK_debmod_fstack_pop(AK_DEBMOD_STATE);

    AK_debmod_log_memory_alloc(id);
    AK_debmod_leave_critical_sec(AK_DEBMOD_STATE);
    AK_check_for_writes();
}

/**
* @author Marin Rukavina, Mislav Bozicevic
* @param func_name function name
* @param in_recur called in recursion
* @brief Print function dependency [private function]
* @return void
*/
void AK_debmod_print_function_use(const char *func_name, uint8_t in_recur){
#if AK_DEBMOD_ON & AK_DEBMOD_PRINT
    int32_t i, func_id;
    assert(AK_DEBMOD_STATE != NULL && AK_DEBMOD_STATE->init == 1);
    if (in_recur == 0){
        printf("Function dependency for %s:\n", func_name);
    }
    func_id = AK_debmod_func_id(AK_DEBMOD_STATE, func_name);
    if (func_id == -1){
        AK_debmod_dv(AK_DEBMOD_STATE, "Function %s does not exist.\n",
            func_name);
        return;
    }
    for (i = 0; i < AK_DEBMOD_MAX_FUNCTIONS; ++i){
        if (AK_DEBMOD_STATE->func_used_by[func_id][i] != -1){
            AK_debmod_print_function_use(
                AK_debmod_func_get_name(AK_DEBMOD_STATE, i), 1);
            printf("%s -> %s\n", func_name,
                AK_debmod_func_get_name(AK_DEBMOD_STATE, i));
        }
    }
#endif
}

/**
* @author Marin Rukavina, Mislav Bozicevic
* @param func_name function name
* @brief Print function dependency [public function]
* @return void
*/
void AK_print_function_use(const char *func_name){
    AK_debmod_print_function_use(func_name, 0);
}

/**
* @author Marin Rukavina, Mislav Bozicevic
* @brief Print function dependency for all functions [public function]
* @return void
*/
void AK_print_function_uses(){
#if AK_DEBMOD_ON & AK_DEBMOD_PRINT
    int32_t i, j;
    assert(AK_DEBMOD_STATE != NULL && AK_DEBMOD_STATE->init == 1);
    printf("Function dependency:\n");
    for (i = 0; i < AK_DEBMOD_MAX_FUNCTIONS; ++i){
        for (j = 0; j < AK_DEBMOD_MAX_FUNCTIONS; ++j){
            if (AK_DEBMOD_STATE->func_used_by[i][j] != -1){
                printf("%d/%d.\t%s -> %s\n",
                    (i + 1)*(j + 1),
                    AK_DEBMOD_MAX_FUNCTIONS * AK_DEBMOD_MAX_FUNCTIONS,
                    AK_debmod_func_get_name(AK_DEBMOD_STATE, i),
                    AK_debmod_func_get_name(AK_DEBMOD_STATE, j));
            }
        }
    }
#endif
}

/**
* @author Marin Rukavina, Mislav Bozicevic
* @brief Print all detected functions
* @return void
*/
void AK_print_active_functions(){
#if AK_DEBMOD_ON & AK_DEBMOD_PRINT
    int32_t i;
    assert(AK_DEBMOD_STATE != NULL && AK_DEBMOD_STATE->init == 1);
    printf("Used functions:\n");
    for (i = 0; i <= AK_DEBMOD_STATE->last_function_id &&
        i < AK_DEBMOD_MAX_FUNCTIONS; ++i){
        printf("%d.\t%s\n", i + 1, AK_DEBMOD_STATE->function[i]);
    }
#endif
}

/**
* @author Marin Rukavina, Mislav Bozicevic
* @brief Write to a file from a buffer (see fwrite) [public function]
* @return number of items written
*/
size_t AK_fwrite(const void *buf, size_t size, size_t count, FILE *fp) {
    size_t ret;
#if !AK_DEBMOD_ON
    return fwrite(buf, size, count, fp);
#endif
#ifdef __linux__
    int32_t pos, i;
    pos = -1;
    for (i = 0; i < AK_DEBMOD_PAGES_NUM; ++i) {
        if (buf == AK_DEBMOD_STATE->page[i]) {
            pos = i;
        }
    }
    assert(mprotect(AK_DEBMOD_STATE->page[pos],
        AK_DEBMOD_STATE->real[pos], PROT_READ | PROT_WRITE) == 0);
#endif
    ret = fwrite(buf, size, count, fp);
#ifdef __linux__
    assert(mprotect(AK_DEBMOD_STATE->page[pos],
        AK_DEBMOD_STATE->real[pos], PROT_READ) == 0);
#endif
    return ret;
}

/**
* @author Marin Rukavina, Mislav Bozicevic
* @brief Read from a file (see fread) [public function]
* @return number of items read
*/
size_t AK_fread(void *buf, size_t size, size_t count, FILE *fp) {
    size_t ret;
#if !AK_DEBMOD_ON
    return fread(buf, size, count, fp);
#endif
#ifdef __linux__
    int32_t pos, i;
    pos = -1;
    for (i = 0; i < AK_DEBMOD_PAGES_NUM; ++i) {
        if (buf == AK_DEBMOD_STATE->page[i]) {
            pos = i;
        }
    }
    assert(mprotect(AK_DEBMOD_STATE->page[pos],
        AK_DEBMOD_STATE->real[pos], PROT_READ | PROT_WRITE) == 0);
#endif
    ret = fread(buf, size, count, fp);
#ifdef __linux__
    assert(mprotect(AK_DEBMOD_STATE->page[pos],
        AK_DEBMOD_STATE->real[pos], PROT_READ) == 0);
#endif
    return ret;
}

#if 0
/* Dummy versions of wrapper functions */
void* AK_calloc(size_t num, size_t size) { return calloc(num, size); }
void* AK_malloc(size_t size) { return malloc(size); }
void AK_free(void* ptr) { return free(ptr); }
void* AK_realloc(void* ptr, size_t size) { return realloc(ptr, size); }
size_t AK_fwrite(const void *buf, size_t size, size_t count, FILE *fp) {
    return fwrite(buf, size, count, fp);
}
size_t AK_fread(void *buf, size_t size, size_t count, FILE *fp) {
    return fread(buf, size, count, fp);
}
void AK_debmod_function_epilogue(const char *func_name,
    const char *source_file, int source_line) { }
void AK_debmod_function_prologue(const char *func_name,
    const char *source_file, int source_line){ }
#endif


/**
* @author Ivan Kristo
* @brief Test function
*/
void AK_mempro_test()
{
    AK_PRO;
    int passed_test = 0;
    int failed_test = 0;

    struct test_struct {
        int id;
        char *name;
        char *val;
    };

    typedef struct test_struct test;

    printf("\nAllocate memory test\nTry to allocate 35 bytes * size of structure test_struct(id,name,val) into t_1\n");
    test *t_1 = AK_malloc(sizeof(test)*(35));
    if(t_1 != NULL) {
        printf("SUCCESS\n");
        passed_test++;
    } else {
        printf("FAIL\n");
        failed_test++;
    }

    printf("\nInsert data into t_1\n");
    t_1[0].id = 1;
    t_1[0].name = "stuktura1";
    t_1[0].val = "101";
    printf("Data in t_1: %i, %s, %s \n",t_1[0].id,t_1[0].name,t_1[0].val);

    printf("\nReallocate memory test\nTry to reallocate t_1 into t_2\n");
    test *t_2 = AK_realloc(t_1,sizeof(test)*(35));

    if(t_2 != NULL) {
        printf("Data in t_2: %i, %s, %s \n",t_2[0].id,t_2[0].name,t_2[0].val);
        printf("SUCCESS\n");
        passed_test++;
    } else {
        printf("FAIL\n");
        failed_test++;
    }   

    printf("\nDeallocate memory test\nTry to deallocate created stuctures\n");
    AK_free(t_2);

    if(t_2[0].id != 1) {
        printf("SUCCESS\n");
        passed_test++;
    } else {
        printf("FAIL\n");
        failed_test++;
    }

    printf("\nSUMMARY:\n");
    printf("Number of test that pass: %i\n", passed_test);
    printf("Number of test that fail: %i\n", failed_test);

    AK_EPI;
    return TEST_result(passed_test,failed_test);
}
